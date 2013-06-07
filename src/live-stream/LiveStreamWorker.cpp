/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LiveStreamWorker.h"
#include "LiveStreamFrame.h"
#include "core/BluecherryApp.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#define ASSERT_WORKER_THREAD() Q_ASSERT(QThread::currentThread() == thread())

int liveStreamInterruptCallback(void *opaque)
{
    LiveStreamWorker *worker = (LiveStreamWorker *)opaque;
    return worker->lastInterruptableOperationStarted().secsTo(QDateTime::currentDateTime()) > 10;
}

LiveStreamWorker::LiveStreamWorker(QObject *parent)
    : QObject(parent), m_ctx(0), m_sws(0),
      m_lastInterruptableOperationStarted(QDateTime::currentDateTime()),
      m_cancelFlag(false), m_autoDeinterlacing(true),
      m_frameHead(0), m_frameTail(0)
{
}

LiveStreamWorker::~LiveStreamWorker()
{
    Q_ASSERT(!m_ctx);

    /* This is a little quirky; The frame in frameHead can be used by the LiveStream
     * at ANY point, including during or after destroy(), so we can't delete it, but
     * we can't guarantee that LiveStream ever saw it, or even still exists. However,
     * we can guarantee that, once this object destructs, LiveStream no longer has
     * any interest in this object or the frame, so this is the only time when it's
     * finally safe to free that frame that cannot leak. */
    LiveStreamFrame::deleteFrom(m_frameHead);
}

void LiveStreamWorker::setUrl(const QByteArray &url)
{
    m_url = url;
}

void LiveStreamWorker::setAutoDeinterlacing(bool enabled)
{
    m_autoDeinterlacing = enabled;
}

void LiveStreamWorker::run()
{
    qDebug() << Q_FUNC_INFO;

    ASSERT_WORKER_THREAD();

    // Prevent concurrent invocations
    if (m_ctx)
        return;

    AVPacket packet;
    AVFrame *frame = avcodec_alloc_frame();
    bool abortFlag = false;

    if (!setup())
        abortFlag = true;

    while (!m_cancelFlag && !abortFlag)
    {
        if (m_threadPause.shouldPause())
            pause();

        startInterruptableOperation();
        int re = av_read_frame(m_ctx, &packet);
        if (re < 0)
        {
            char error[512];
            av_strerror(re, error, sizeof(error));
            emit fatalError(QString::fromLatin1("%1 (in read_frame)").arg(QLatin1String(error)));
            break;
        }

        uint8_t *data = packet.data;
        bcApp->globalRate->addSampleValue(packet.size);

        int in_packet = 0;
        while (packet.size > 0)
        {
            int got_picture = 0;
            startInterruptableOperation();
            re = avcodec_decode_video2(m_ctx->streams[0]->codec, frame, &got_picture, &packet);
            if (re < 0)
            {
                emit fatalError(QLatin1String("Decoding error"));
                abortFlag = true;
                break;
            }

            if (got_picture)
            {
                processVideo(m_ctx->streams[0], frame);
                in_packet++;
            }

            packet.size -= re;
            packet.data += re;
            break;
        }

        packet.data = data;
        av_free_packet(&packet);

        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    av_free(frame);
    destroy();

    emit finished();
}

bool LiveStreamWorker::setup()
{
    ASSERT_WORKER_THREAD();

    bool ok = false;

    AVDictionary *opt = 0;
    av_dict_set(&opt, "threads", "1", 0);
    av_dict_set(&opt, "allowed_media_types", "-audio-data", 0);
    av_dict_set(&opt, "max_delay", QByteArray::number(qint64(0.3*AV_TIME_BASE)).constData(), 0);
    /* Because the server always starts streams on a keyframe, we don't need any time here.
     * If the first frame is not a keyframe, this could result in failures or corruption. */
    av_dict_set(&opt, "analyzeduration", "0", 0);

    /* Only TCP is supported currently; speed up connection by trying that first */
    av_dict_set(&opt, "rtsp_transport", "tcp", 0);

    AVDictionary **opt_si = 0;
    AVDictionary *opt_cpy = 0;
    av_dict_copy(&opt_cpy, opt, 0);

    m_ctx = avformat_alloc_context();
    m_ctx->interrupt_callback.callback = liveStreamInterruptCallback;
    m_ctx->interrupt_callback.opaque = this;

    int re;
    startInterruptableOperation();
    if ((re = avformat_open_input(&m_ctx, m_url.constData(), NULL, &opt_cpy)) != 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        goto end;
    }

    av_dict_free(&opt_cpy);

    /* avformat_find_stream_info takes an array of AVDictionary ptrs for each stream */
    opt_si = new AVDictionary*[m_ctx->nb_streams];
    for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
    {
        opt_si[i] = 0;
        av_dict_copy(&opt_si[i], opt, 0);
    }

    startInterruptableOperation();
    if ((re = avformat_find_stream_info(m_ctx, opt_si)) < 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        goto end;
    }

    for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
    {
        char info[512];
        startInterruptableOperation();
        AVCodec *codec = avcodec_find_decoder(m_ctx->streams[i]->codec->codec_id);
        av_dict_copy(&opt_cpy, opt, 0);
        startInterruptableOperation();
        if (!m_ctx->streams[i]->codec->codec &&
            (re = avcodec_open2(m_ctx->streams[i]->codec, codec, &opt_cpy)) < 0)
        {
            qDebug() << "LiveStream: cannot find decoder for stream" << i << "codec" <<
                        m_ctx->streams[i]->codec->codec_id;
            av_dict_free(&opt_cpy);
            continue;
        }
        av_dict_free(&opt_cpy);
        avcodec_string(info, sizeof(info), m_ctx->streams[i]->codec, 0);
        qDebug() << "LiveStream: stream #" << i << ":" << info;
    }

    ok = true;
end:
    av_dict_free(&opt);
    if (opt_si)
    {
        for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
            av_dict_free(&opt_si[i]);
        delete[] opt_si;
    }
    if (!ok && m_ctx)
    {
        avformat_close_input(&m_ctx);
        m_ctx = 0;
    }

    return ok;
}

void LiveStreamWorker::destroy()
{
    ASSERT_WORKER_THREAD();

    if (!m_ctx)
        return;

    if (m_sws)
    {
        sws_freeContext(m_sws);
        m_sws = 0;
    }

    m_frameLock.lock();
    if (m_frameHead)
    {
        /* Even now, we cannot touch frameHead. It might be used by the other thread. */
        LiveStreamFrame::deleteFrom(m_frameHead->next);
        m_frameHead->next = 0;
    }
    m_frameTail = m_frameHead;
    m_frameLock.unlock();

    for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
    {
        avcodec_close(m_ctx->streams[i]->codec);
        av_freep(m_ctx->streams[i]);
    }

    startInterruptableOperation();
    avformat_close_input(&m_ctx);
    m_ctx = 0;
}

void LiveStreamWorker::startInterruptableOperation()
{
    m_lastInterruptableOperationStarted = QDateTime::currentDateTime();
}

QDateTime LiveStreamWorker::lastInterruptableOperationStarted() const
{
    return m_lastInterruptableOperationStarted;
}

void LiveStreamWorker::processVideo(struct AVStream *stream, struct AVFrame *rawFrame)
{
    const PixelFormat fmt = PIX_FMT_BGRA;

    /* Assume that H.264 D1-resolution video is interlaced, to work around a solo(?) bug
     * that results in interlaced_frame not being set for videos from solo6110. */
    if (m_autoDeinterlacing && (rawFrame->interlaced_frame ||
                              (stream->codec->codec_id == CODEC_ID_H264 &&
                               ((stream->codec->width == 704 && stream->codec->height == 480) ||
                                (stream->codec->width == 720 && stream->codec->height == 576)))))
    {
        if (avpicture_deinterlace((AVPicture*)rawFrame, (AVPicture*)rawFrame, stream->codec->pix_fmt,
                                  stream->codec->width, stream->codec->height) < 0)
        {
            qDebug("deinterlacing failed");
        }
    }

    m_sws = sws_getCachedContext(m_sws, stream->codec->width, stream->codec->height, stream->codec->pix_fmt,
                               stream->codec->width, stream->codec->height, fmt, SWS_BICUBIC,
                               NULL, NULL, NULL);

    int bufSize  = avpicture_get_size(fmt, stream->codec->width, stream->codec->height);
    uint8_t *buf = (uint8_t*) av_malloc(bufSize);

    AVFrame *frame = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)frame, buf, fmt, stream->codec->width, stream->codec->height);
    sws_scale(m_sws, (const uint8_t**)rawFrame->data, rawFrame->linesize, 0, stream->codec->height,
              frame->data, frame->linesize);

    frame->width  = stream->codec->width;
    frame->height = stream->codec->height;
    frame->pts    = rawFrame->pkt_pts;

    LiveStreamFrame *sf = new LiveStreamFrame;
    sf->d    = frame;

    m_frameLock.lock();
    if (!m_frameTail) {
        m_frameHead = sf;
        m_frameTail = sf;
    } else {
        sf->d->display_picture_number = m_frameTail->d->display_picture_number+1;
        m_frameTail->next = sf;
        m_frameTail = sf;

        /* If necessary, drop frames to avoid exploding memory. This will only happen if
         * the UI thread cannot keep up enough to do is own PTS-based framedropping.
         * It is NEVER safe to drop frameHead; only the UI thread may do that. */
        if (m_frameTail->d->display_picture_number - m_frameHead->next->d->display_picture_number >= 6)
        {
            for (LiveStreamFrame *f = m_frameHead->next, *n = f->next; f && f != m_frameTail; f = n, n = f->next)
                delete f;
            m_frameHead->next = m_frameTail;
        }
    }
    m_frameLock.unlock();
}

void LiveStreamWorker::stop()
{
    m_cancelFlag = true;
    m_threadPause.setPaused(false);
}

void LiveStreamWorker::setPaused(bool paused)
{
    if (!m_ctx)
        return;

    m_threadPause.setPaused(paused);
}

void LiveStreamWorker::pause()
{
    av_read_pause(m_ctx);
    m_threadPause.pause();
    av_read_play(m_ctx);
}
