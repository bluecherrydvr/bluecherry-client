#include "LiveStreamWorker.h"
#include <QDebug>
#include <QCoreApplication>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

LiveStreamWorker::LiveStreamWorker(QObject *parent)
    : QObject(parent), ctx(0), sws(0), cancelFlag(false)
{
}

void LiveStreamWorker::run()
{
    if (!setup()) {
        destroy();
        return;
    }

    AVPacket packet;
    AVFrame *frame = avcodec_alloc_frame();

    while (!cancelFlag)
    {
        int re = av_read_frame(ctx, &packet);
        if (re < 0)
        {
            char error[512];
            av_strerror(re, error, sizeof(error));
            qDebug() << "LiveStream: read_frame error:" << error;
            break;
        }

        while (packet.size > 0)
        {
            int got_picture = 0;
            re = avcodec_decode_video2(ctx->streams[0]->codec, frame, &got_picture, &packet);
            if (re < 0)
            {
                qDebug() << "LiveStream: decoding error";
                break;
            }

            if (got_picture)
            {
                processVideo(ctx->streams[0], frame);
            }

            /*packet.size -= re;
            packet.data += re;*/
            break;
        }

        av_free_packet(&packet);

        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    qDebug() << "LiveStream: Ending thread";

    av_free(frame);
    destroy();
}

bool LiveStreamWorker::setup()
{
    int re;
    if ((re = avformat_open_input(&ctx, "rtsp://192.168.0.15/live_41", NULL, NULL)) != 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        qDebug() << "LiveStream: open_input error:" << error;
        return false;
    }

    if ((re = avformat_find_stream_info(ctx, NULL)) < 0)
    {
        char error[512];
        av_strerror(re, error, sizeof(error));
        qDebug() << "LiveStream: find_stream_info error:" << error;
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
    if (!ctx)
        return;

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

    QImage image = QImage(stream->codec->width, stream->codec->height, QImage::Format_RGB32);

    AVFrame frame;
    avcodec_get_frame_defaults(&frame);
    avpicture_fill((AVPicture*)&frame, (uint8_t*)image.bits(), fmt, stream->codec->width, stream->codec->height);
    sws_scale(sws, (const uint8_t**)rawFrame->data, rawFrame->linesize, 0, stream->codec->height,
              frame.data, frame.linesize);

    emit this->frame(image);
}

void LiveStreamWorker::stop()
{
    cancelFlag = true;
    deleteLater();
}
