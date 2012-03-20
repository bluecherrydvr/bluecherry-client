#include "LiveStreamWorker.h"
#include "BluecherryApp.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#define ASSERT_WORKER_THREAD() Q_ASSERT(QThread::currentThread() == thread())

LiveStreamWorker::LiveStreamWorker(QObject *parent)
    : QObject(parent), ctx(0), sws(0), cancelFlag(false), paused(false), autoDeinterlacing(true),
      blockingLoop(0), frameHead(0), frameTail(0)
{
}

LiveStreamWorker::~LiveStreamWorker()
{
    Q_ASSERT(!ctx);

    /* This is a little quirky; The frame in frameHead can be used by the LiveStream
     * at ANY point, including during or after destroy(), so we can't delete it, but
     * we can't guarantee that LiveStream ever saw it, or even still exists. However,
     * we can guarantee that, once this object destructs, LiveStream no longer has
     * any interest in this object or the frame, so this is the only time when it's
     * finally safe to free that frame that cannot leak. */
    for (StreamFrame *f = frameHead, *n; f; f = n)
    {
        n = f->next;
        delete f;
    }
}

void LiveStreamWorker::setUrl(const QByteArray &url)
{
    this->url = url;
}

void LiveStreamWorker::setAutoDeinterlacing(bool enabled)
{
    autoDeinterlacing = enabled;
}

void LiveStreamWorker::run()
{
    ASSERT_WORKER_THREAD();

    // Prevent concurrent invocations
    if (ctx)
        return;

    AVPacket packet;
    AVFrame *frame = avcodec_alloc_frame();
    bool abortFlag = false;

    if (!setup())
        abortFlag = true;

    while (!cancelFlag && !abortFlag)
    {
        int re = av_read_frame(ctx, &packet);
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
            re = avcodec_decode_video2(ctx->streams[0]->codec, frame, &got_picture, &packet);
            if (re < 0)
            {
                emit fatalError(QLatin1String("Decoding error"));
                abortFlag = true;
                break;
            }

            if (got_picture)
            {
                processVideo(ctx->streams[0], frame);
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

    AVDictionary **opt_si = 0;
    AVDictionary *opt_cpy = 0;
    av_dict_copy(&opt_cpy, opt, 0);

    int re;
    if ((re = avformat_open_input(&ctx, url.constData(), NULL, &opt_cpy)) != 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        goto end;
    }

    av_dict_free(&opt_cpy);

    /* avformat_find_stream_info takes an array of AVDictionary ptrs for each stream */
    opt_si = new AVDictionary*[ctx->nb_streams];
    for (int i = 0; i < ctx->nb_streams; ++i)
    {
        opt_si[i] = 0;
        av_dict_copy(&opt_si[i], opt, 0);
    }

    if ((re = avformat_find_stream_info(ctx, opt_si)) < 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        goto end;
    }

    for (int i = 0; i < ctx->nb_streams; ++i)
    {
        char info[512];
        AVCodec *codec = avcodec_find_decoder(ctx->streams[i]->codec->codec_id);
        av_dict_copy(&opt_cpy, opt, 0);
        if (!ctx->streams[i]->codec->codec &&
            (re = avcodec_open2(ctx->streams[i]->codec, codec, &opt_cpy)) < 0)
        {
            qDebug() << "LiveStream: cannot find decoder for stream" << i << "codec" <<
                        ctx->streams[i]->codec->codec_id;
            av_dict_free(&opt_cpy);
            continue;
        }
        av_dict_free(&opt_cpy);
        avcodec_string(info, sizeof(info), ctx->streams[i]->codec, 0);
        qDebug() << "LiveStream: stream #" << i << ":" << info;
    }

    ok = true;
end:
    av_dict_free(&opt);
    if (opt_si)
    {
        for (int i = 0; i < ctx->nb_streams; ++i)
            av_dict_free(&opt_si[i]);
        delete[] opt_si;
    }
    if (!ok && ctx)
    {
        av_close_input_file(ctx);
        ctx = 0;
    }

    return ok;
}

void LiveStreamWorker::destroy()
{
    ASSERT_WORKER_THREAD();

    if (!ctx)
        return;

    if (sws)
    {
        sws_freeContext(sws);
        sws = 0;
    }

    frameLock.lock();
    if (frameHead)
    {
        /* Even now, we cannot touch frameHead. It might be used by the other thread. */
        for (StreamFrame *f = frameHead->next, *n; f; f = n)
        {
            n = f->next;
            delete f;
        }
        frameHead->next = 0;
    }
    frameTail = frameHead;
    frameLock.unlock();

    for (int i = 0; i < ctx->nb_streams; ++i)
    {
        avcodec_close(ctx->streams[i]->codec);
        av_freep(ctx->streams[i]);
    }

    av_close_input_file(ctx);
    ctx = 0;
}

void LiveStreamWorker::processVideo(struct AVStream *stream, struct AVFrame *rawFrame)
{
    const PixelFormat fmt = PIX_FMT_BGRA;

    /* Assume that H.264 D1-resolution video is interlaced, to work around a solo(?) bug
     * that results in interlaced_frame not being set for videos from solo6110. */
    if (autoDeinterlacing && (rawFrame->interlaced_frame ||
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

    sws = sws_getCachedContext(sws, stream->codec->width, stream->codec->height, stream->codec->pix_fmt,
                               stream->codec->width, stream->codec->height, fmt, SWS_BICUBIC,
                               NULL, NULL, NULL);

    int bufSize  = avpicture_get_size(fmt, stream->codec->width, stream->codec->height);
    uint8_t *buf = (uint8_t*) av_malloc(bufSize);

    AVFrame *frame = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)frame, buf, fmt, stream->codec->width, stream->codec->height);
    sws_scale(sws, (const uint8_t**)rawFrame->data, rawFrame->linesize, 0, stream->codec->height,
              frame->data, frame->linesize);

    frame->width  = stream->codec->width;
    frame->height = stream->codec->height;
    frame->pts    = rawFrame->pkt_pts;

    StreamFrame *sf = new StreamFrame;
    sf->d    = frame;

    frameLock.lock();
    if (!frameTail) {
        frameHead = sf;
        frameTail = sf;
    } else {
        sf->d->display_picture_number = frameTail->d->display_picture_number+1;
        frameTail->next = sf;
        frameTail = sf;

        /* If necessary, drop frames to avoid exploding memory. This will only happen if
         * the UI thread cannot keep up enough to do is own PTS-based framedropping.
         * It is NEVER safe to drop frameHead; only the UI thread may do that. */
        if (frameTail->d->display_picture_number - frameHead->next->d->display_picture_number >= 30)
        {
            for (StreamFrame *f = frameHead->next, *n = f->next; f && f != frameTail; f = n, n = f->next)
                delete f;
            frameHead->next = frameTail;
        }
    }
    frameLock.unlock();
}

void LiveStreamWorker::stop()
{
    ASSERT_WORKER_THREAD();
    cancelFlag = true;
    if (blockingLoop)
        blockingLoop->exit();
    deleteLater();
}

void LiveStreamWorker::setPaused(bool v)
{
    ASSERT_WORKER_THREAD();
    if (!ctx || v == paused)
        return;

    paused = v;
    if (paused)
    {
        av_read_pause(ctx);
        QEventLoop loop;
        blockingLoop = &loop;
        loop.exec();
        blockingLoop = 0;
    }
    else
    {
        av_read_play(ctx);
        Q_ASSERT(blockingLoop);
        if (blockingLoop)
            blockingLoop->exit();
    }
}

StreamFrame::~StreamFrame()
{
    if (d)
    {
        av_free(d->data[0]);
        av_free(d);
    }
}
