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
#include "LiveStreamFrameFormatter.h"
#include "LiveStreamFrameQueue.h"
#include "core/BluecherryApp.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

extern "C" {
#   include "libavcodec/avcodec.h"
#   include "libavformat/avformat.h"
#   include "libswscale/swscale.h"
#   include "libavutil/mathematics.h"
}

#define ASSERT_WORKER_THREAD() Q_ASSERT(QThread::currentThread() == thread())

int liveStreamInterruptCallback(void *opaque)
{
    LiveStreamWorker *worker = (LiveStreamWorker *)opaque;
    return worker->lastInterruptableOperationStarted().secsTo(QDateTime::currentDateTime()) > 10;
}

LiveStreamWorker::LiveStreamWorker(QObject *parent)
    : QObject(parent), m_ctx(0),
      m_lastInterruptableOperationStarted(QDateTime::currentDateTime()),
      m_cancelFlag(false), m_autoDeinterlacing(true),
      m_frameQueue(new LiveStreamFrameQueue(6))
{
}

LiveStreamWorker::~LiveStreamWorker()
{
    if (!m_ctx)
        return;


    for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
    {
        avcodec_close(m_ctx->streams[i]->codec);
        av_freep(m_ctx->streams[i]);
    }

    startInterruptableOperation();
    avformat_close_input(&m_ctx);
}

void LiveStreamWorker::setUrl(const QByteArray &url)
{
    m_url = url;
}

void LiveStreamWorker::setAutoDeinterlacing(bool autoDeinterlacing)
{
    m_autoDeinterlacing = autoDeinterlacing;
    if (m_frameFormatter)
        m_frameFormatter->setAutoDeinterlacing(autoDeinterlacing);
}

void LiveStreamWorker::run()
{
    qDebug() << Q_FUNC_INFO;

    ASSERT_WORKER_THREAD();

    // Prevent concurrent invocations
    if (m_ctx)
        return;

    if (setup())
        processStreamLoop();

    emit finished();
}

void LiveStreamWorker::processStreamLoop()
{
    bool abortFlag = false;
    while (!m_cancelFlag && !abortFlag)
    {
        if (m_threadPause.shouldPause())
            pause();
        abortFlag = !processStream();
    }
}

bool LiveStreamWorker::processStream()
{
    startInterruptableOperation();

    bool ok;
    AVPacket packet = readPacket(&ok);
    if (!ok)
        return false;

    bool result = processPacket(packet);
    av_free_packet(&packet);
    return result;
}

AVPacket LiveStreamWorker::readPacket(bool *ok)
{
    if (ok)
        *ok = true;

    AVPacket packet;
    int re = av_read_frame(m_ctx, &packet);
    if (0 == re)
        return packet;

    emit fatalError(QString::fromLatin1("Reading error: %1").arg(errorMessageFromCode(re)));
    av_free_packet(&packet);

    if (ok)
        *ok = false;
    return packet;
}

bool LiveStreamWorker::processPacket(struct AVPacket packet)
{
    bcApp->globalRate->addSampleValue(packet.size);

    startInterruptableOperation();
    int got_picture = 0;

    AVPacket datapacket;
    while (packet.size > 0)
    {
        AVFrame *frame = avcodec_alloc_frame();
        int re = avcodec_decode_video2(m_ctx->streams[0]->codec, frame, &got_picture, &packet);
        if (re == 0)
            break;

        if (re < 0)
        {
            emit fatalError(QString::fromLatin1("Decoding error: %1").arg(errorMessageFromCode(re)));
            av_free(frame);
            return false;
        }

        if (got_picture)
            processFrame(frame);

        av_free(frame);
        
        packet.data += re;
        packet.size -= re;
    }

    return true;
}

QString LiveStreamWorker::errorMessageFromCode(int errorCode)
{
    char error[512];
    av_strerror(errorCode, error, sizeof(error));
    return QString::fromLatin1(error);
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
        emit fatalError(QString::fromLatin1("Open error: %1").arg(errorMessageFromCode(re)));
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
        emit fatalError(QString::fromLatin1("Find stream error: %1").arg(errorMessageFromCode(re)));
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

    if (ok)
    {
        m_frameFormatter.reset(new LiveStreamFrameFormatter(m_ctx->streams[0]));
        m_frameFormatter->setAutoDeinterlacing(m_autoDeinterlacing);
    }

    return ok;
}

void LiveStreamWorker::startInterruptableOperation()
{
    m_lastInterruptableOperationStarted = QDateTime::currentDateTime();
}

QDateTime LiveStreamWorker::lastInterruptableOperationStarted() const
{
    return m_lastInterruptableOperationStarted;
}

LiveStreamFrame * LiveStreamWorker::frameToDisplay()
{
    return m_frameQueue.data()->dequeue();
}

void LiveStreamWorker::processFrame(struct AVFrame *rawFrame)
{
    Q_ASSERT(m_frameFormatter);
    m_frameQueue->enqueue(m_frameFormatter->formatFrame(rawFrame));
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
