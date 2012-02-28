#include "LiveStreamWorker.h"
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
    : QObject(parent), ctx(0), sws(0), cancelFlag(false)
{
}

void LiveStreamWorker::setUrl(const QByteArray &url)
{
    this->url = url;
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

    int re;
    if ((re = avformat_open_input(&ctx, url.constData(), NULL, NULL)) != 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        return false;
    }

    if ((re = avformat_find_stream_info(ctx, NULL)) < 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        emit fatalError(QString::fromLatin1(error));
        av_close_input_file(ctx);
        return false;
    }

    for (int i = 0; i < ctx->nb_streams; ++i)
    {
        char info[512];
        AVCodec *codec = avcodec_find_decoder(ctx->streams[i]->codec->codec_id);
        if ((re = avcodec_open2(ctx->streams[i]->codec, codec, NULL)) < 0)
        {
            qDebug() << "LiveStream: cannot find decoder for stream" << i << "codec" <<
                        ctx->streams[i]->codec->codec_id;
            continue;
        }

        avcodec_string(info, sizeof(info), ctx->streams[i]->codec, 0);
        qDebug() << "LiveStream: stream #" << i << ":" << info;
    }

    return true;
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

    AVFrame *frame = takeFrame();
    if (frame)
    {
        av_free(frame->data[0]);
        av_free(frame);
    }

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

    sws = sws_getCachedContext(sws, stream->codec->width, stream->codec->height, stream->codec->pix_fmt,
                               stream->codec->width, stream->codec->height, fmt, SWS_BICUBIC,
                               NULL, NULL, NULL);

    int bufSize = avpicture_get_size(fmt, stream->codec->width, stream->codec->height);
    uint8_t *buf = (uint8_t*) av_malloc(bufSize);

    AVFrame *frame = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)frame, buf, fmt, stream->codec->width, stream->codec->height);
    sws_scale(sws, (const uint8_t**)rawFrame->data, rawFrame->linesize, 0, stream->codec->height,
              frame->data, frame->linesize);

    frame->width = stream->codec->width;
    frame->height = stream->codec->height;

    AVFrame *oldFrame = videoFrame.fetchAndStoreOrdered(frame);
    if (oldFrame)
    {
        qDebug() << "LiveStream: discarded frame";
        av_free(oldFrame->data[0]);
        av_free(oldFrame);
    }
}

AVFrame *LiveStreamWorker::takeFrame()
{
    return videoFrame.fetchAndStoreOrdered(0);
}

void LiveStreamWorker::stop()
{
    cancelFlag = true;
    deleteLater();
}
