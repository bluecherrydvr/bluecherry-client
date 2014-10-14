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

#include "RtspStreamWorker.h"
#include "RtspStreamFrame.h"
#include "RtspStreamFrameFormatter.h"
#include "RtspStreamFrameQueue.h"
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

int rtspStreamInterruptCallback(void *opaque)
{
    RtspStreamWorker *worker = (RtspStreamWorker *)opaque;
    return worker->shouldInterrupt();
}

RtspStreamWorker::RtspStreamWorker(QSharedPointer<RtspStreamFrameQueue> &shared_queue, QObject *parent)
    : QObject(parent), m_ctx(0),
      m_cancelFlag(false), m_autoDeinterlacing(true),
      m_frameQueue(new RtspStreamFrameQueue(6))
{
    shared_queue = m_frameQueue;
}

RtspStreamWorker::~RtspStreamWorker()
{
    if (!m_ctx)
        return;

    for (unsigned int i = 0; i < m_ctx->nb_streams; ++i)
    {
        avcodec_close(m_ctx->streams[i]->codec);
        av_freep(m_ctx->streams[i]);
    }

    startInterruptableOperation(5);
    avformat_close_input(&m_ctx);
}

void RtspStreamWorker::setUrl(const QUrl &url)
{
    m_url = url;
}

void RtspStreamWorker::setAutoDeinterlacing(bool autoDeinterlacing)
{
    m_autoDeinterlacing = autoDeinterlacing;
    if (m_frameFormatter)
        m_frameFormatter->setAutoDeinterlacing(autoDeinterlacing);
}

bool RtspStreamWorker::shouldInterrupt() const
{
    if (m_cancelFlag)
        return true;

    if (m_timeout < QDateTime::currentDateTime())
        return true;

    return false;
}

void RtspStreamWorker::run()
{
    ASSERT_WORKER_THREAD();

    // Prevent concurrent invocations
    if (m_ctx)
        return;

    if (setup())
        processStreamLoop();

    deleteLater();
}

void RtspStreamWorker::processStreamLoop()
{
    bool abortFlag = false;
    while (!m_cancelFlag && !abortFlag)
    {
        if (m_threadPause.shouldPause())
            pause();
        abortFlag = !processStream();
    }
}

bool RtspStreamWorker::processStream()
{
    bool ok;
    AVPacket packet = readPacket(&ok);
    if (!ok)
        return false;

    bool result = processPacket(packet);
    av_free_packet(&packet);
    return result;
}

AVPacket RtspStreamWorker::readPacket(bool *ok)
{
    if (ok)
        *ok = true;

    AVPacket packet;
    startInterruptableOperation(30);
    int re = av_read_frame(m_ctx, &packet);
    if (0 == re)
        return packet;

    emit fatalError(QString::fromLatin1("Reading error: %1").arg(errorMessageFromCode(re)));
    av_free_packet(&packet);

    if (ok)
        *ok = false;
    return packet;
}

bool RtspStreamWorker::processPacket(struct AVPacket packet)
{
    emit bytesDownloaded(packet.size);

    while (packet.size > 0)
    {
        AVFrame *frame = extractFrame(packet);
        if (frame)
        {
            processFrame(frame);
            av_free(frame);
        }
    }

    return true;
}

AVFrame * RtspStreamWorker::extractFrame(AVPacket &packet)
{
    AVFrame *frame = avcodec_alloc_frame();
    startInterruptableOperation(5);

    int pictureAvailable;
    int re = avcodec_decode_video2(m_ctx->streams[0]->codec, frame, &pictureAvailable, &packet);
    if (re == 0) {
        return 0;
    }

    if (re < 0)
    {
        emit fatalError(QString::fromLatin1("Decoding error: %1").arg(errorMessageFromCode(re)));
        av_free(frame);
        return 0;
    }

    packet.size -= re;
    packet.data += re;

    if (!pictureAvailable)
    {
        av_free(frame);
        return 0;
    }

    return frame;
}

void RtspStreamWorker::processFrame(struct AVFrame *rawFrame)
{
    Q_ASSERT(m_frameFormatter);
    startInterruptableOperation(5);
    m_frameQueue->enqueue(m_frameFormatter->formatFrame(rawFrame));
}

QString RtspStreamWorker::errorMessageFromCode(int errorCode)
{
    char error[512];
    av_strerror(errorCode, error, sizeof(error));
    return QString::fromLatin1(error);
}

bool RtspStreamWorker::setup()
{
    ASSERT_WORKER_THREAD();

    m_ctx = avformat_alloc_context();
    m_ctx->interrupt_callback.callback = rtspStreamInterruptCallback;
    m_ctx->interrupt_callback.opaque = this;

    AVDictionary *options = createOptions();
    bool prepared = prepareStream(&m_ctx, options);
    av_dict_free(&options);

    if (prepared)
    {
        m_frameFormatter.reset(new RtspStreamFrameFormatter(m_ctx->streams[0]));
        m_frameFormatter->setAutoDeinterlacing(m_autoDeinterlacing);
    }
    else if (m_ctx)
    {
        avformat_close_input(&m_ctx);
        m_ctx = 0;
    }

    return prepared;
}

bool RtspStreamWorker::prepareStream(AVFormatContext **context, AVDictionary *options)
{
    if (!openInput(context, options))
        return false;

    if (!findStreamInfo(*context, options))
        return false;

    openCodecs(*context, options);
    return true;
}

AVDictionary * RtspStreamWorker::createOptions() const
{
    AVDictionary *options = 0;

    av_dict_set(&options, "threads", "1", 0);
    av_dict_set(&options, "allowed_media_types", "-audio-data", 0);
    av_dict_set(&options, "max_delay", QByteArray::number(qint64(0.3*AV_TIME_BASE)).constData(), 0);
    /* Because the server always starts streams on a keyframe, we don't need any time here.
     * If the first frame is not a keyframe, this could result in failures or corruption. */
    av_dict_set(&options, "analyzeduration", "0", 0);
    /* Only TCP is supported currently; speed up connection by trying that first */
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    return options;
}

bool RtspStreamWorker::openInput(AVFormatContext **context, AVDictionary *options)
{
    AVDictionary *optionsCopy = 0;
    av_dict_copy(&optionsCopy, options, 0);
    startInterruptableOperation(20);
    int errorCode = avformat_open_input(context, qPrintable(m_url.toString()), NULL, &optionsCopy);
    av_dict_free(&optionsCopy);

    if (errorCode < 0)
    {
        emit fatalError(QString::fromLatin1("Open error: %1").arg(errorMessageFromCode(errorCode)));
        return false;
    }

    return true;
}

bool RtspStreamWorker::findStreamInfo(AVFormatContext* context, AVDictionary* options)
{
    AVDictionary **streamOptions = createStreamsOptions(context, options);
    startInterruptableOperation(20);
    int errorCode = avformat_find_stream_info(context, streamOptions);
    destroyStreamOptions(context, streamOptions);

    if (errorCode < 0)
    {
        emit fatalError(QString::fromLatin1("Find stream error: %1").arg(errorMessageFromCode(errorCode)));
        return false;
    }

    return true;
}

AVDictionary ** RtspStreamWorker::createStreamsOptions(AVFormatContext *context, AVDictionary *options) const
{
    AVDictionary **streamOptions = 0;
    streamOptions = new AVDictionary*[context->nb_streams];
    for (unsigned int i = 0; i < context->nb_streams; ++i)
    {
        streamOptions[i] = 0;
        av_dict_copy(&streamOptions[i], options, 0);
    }

    return streamOptions;
}

void RtspStreamWorker::destroyStreamOptions(AVFormatContext *context, AVDictionary** streamOptions)
{
    if (!streamOptions)
        return;

    for (unsigned int i = 0; i < context->nb_streams; ++i)
        av_dict_free(&streamOptions[i]);
    delete[] streamOptions;
}

void RtspStreamWorker::openCodecs(AVFormatContext *context, AVDictionary *options)
{
    for (unsigned int i = 0; i < context->nb_streams; i++)
    {
        AVStream *stream = context->streams[i];
        bool codecOpened = openCodec(stream, options);
        if (!codecOpened)
        {
            qDebug() << "RtspStream: cannot find decoder for stream" << i << "codec" <<
                        stream->codec->codec_id;
            continue;
        }

        char info[512];
        avcodec_string(info, sizeof(info), stream->codec, 0);
        qDebug() << "RtspStream: stream #" << i << ":" << info;
    }
}

bool RtspStreamWorker::openCodec(AVStream *stream, AVDictionary *options)
{
    startInterruptableOperation(5);
    AVCodec *codec = avcodec_find_decoder(stream->codec->codec_id);

    AVDictionary *optionsCopy = 0;
    av_dict_copy(&optionsCopy, options, 0);
    startInterruptableOperation(5);
    int errorCode = avcodec_open2(stream->codec, codec, &optionsCopy);
    av_dict_free(&optionsCopy);

    return 0 == errorCode;
}

void RtspStreamWorker::startInterruptableOperation(int timeoutInSeconds)
{
    m_timeout = QDateTime::currentDateTime().addSecs(timeoutInSeconds);
}

RtspStreamFrame * RtspStreamWorker::frameToDisplay()
{
    if (m_cancelFlag || !m_frameQueue)
        return 0;

    return m_frameQueue.data()->dequeue();
}

void RtspStreamWorker::stop()
{
    m_cancelFlag = true;
    m_threadPause.setPaused(false);
}

void RtspStreamWorker::setPaused(bool paused)
{
    if (!m_ctx)
        return;

    m_threadPause.setPaused(paused);
}

void RtspStreamWorker::pause()
{
    av_read_pause(m_ctx);
    m_threadPause.pause();
    av_read_play(m_ctx);
}
