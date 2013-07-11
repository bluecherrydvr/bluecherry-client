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
    return worker->shouldInterrupt();
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

void LiveStreamWorker::setUrl(const QUrl &url)
{
    m_url = url;
}

void LiveStreamWorker::setAutoDeinterlacing(bool autoDeinterlacing)
{
    m_autoDeinterlacing = autoDeinterlacing;
    if (m_frameFormatter)
        m_frameFormatter->setAutoDeinterlacing(autoDeinterlacing);
}

bool LiveStreamWorker::shouldInterrupt() const
{
    if (m_cancelFlag)
        return true;

    if (m_lastInterruptableOperationStarted.secsTo(QDateTime::currentDateTime()) > 5)
        return true;

    return false;
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
    startInterruptableOperation();
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

AVFrame * LiveStreamWorker::extractFrame(AVPacket &packet)
{
    AVFrame *frame = avcodec_alloc_frame();
    startInterruptableOperation();

    int pictureAvailable;
    int re = avcodec_decode_video2(m_ctx->streams[0]->codec, frame, &pictureAvailable, &packet);
    if (re == 0)
        return 0;

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

void LiveStreamWorker::processFrame(struct AVFrame *rawFrame)
{
    Q_ASSERT(m_frameFormatter);
    startInterruptableOperation();
    m_frameQueue->enqueue(m_frameFormatter->formatFrame(rawFrame));
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

    m_ctx = avformat_alloc_context();
    m_ctx->interrupt_callback.callback = liveStreamInterruptCallback;
    m_ctx->interrupt_callback.opaque = this;

    AVDictionary *options = createOptions();
    bool prepared = prepareStream(&m_ctx, options);
    av_dict_free(&options);

    if (prepared)
    {
        m_frameFormatter.reset(new LiveStreamFrameFormatter(m_ctx->streams[0]));
        m_frameFormatter->setAutoDeinterlacing(m_autoDeinterlacing);
    }
    else if (m_ctx)
    {
        avformat_close_input(&m_ctx);
        m_ctx = 0;
    }

    return prepared;
}

bool LiveStreamWorker::prepareStream(AVFormatContext **context, AVDictionary *options)
{
    if (!openInput(context, options))
        return false;

    if (!findStreamInfo(*context, options))
        return false;

    openCodecs(*context, options);
    return true;
}

AVDictionary * LiveStreamWorker::createOptions() const
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

bool LiveStreamWorker::openInput(AVFormatContext **context, AVDictionary *options)
{
    AVDictionary *optionsCopy = 0;
    av_dict_copy(&optionsCopy, options, 0);
    startInterruptableOperation();
    int errorCode = avformat_open_input(context, qPrintable(m_url.toString()), NULL, &optionsCopy);
    av_dict_free(&optionsCopy);

    if (errorCode < 0)
    {
        emit fatalError(QString::fromLatin1("Open error: %1").arg(errorMessageFromCode(errorCode)));
        return false;
    }

    return true;
}

bool LiveStreamWorker::findStreamInfo(AVFormatContext* context, AVDictionary* options)
{
    AVDictionary **streamOptions = createStreamsOptions(context, options);
    startInterruptableOperation();
    int errorCode = avformat_find_stream_info(context, streamOptions);
    destroyStreamOptions(context, streamOptions);

    if (errorCode < 0)
    {
        emit fatalError(QString::fromLatin1("Find stream error: %1").arg(errorMessageFromCode(errorCode)));
        return false;
    }

    return true;
}

AVDictionary ** LiveStreamWorker::createStreamsOptions(AVFormatContext *context, AVDictionary *options) const
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

void LiveStreamWorker::destroyStreamOptions(AVFormatContext *context, AVDictionary** streamOptions)
{
    if (!streamOptions)
        return;

    for (unsigned int i = 0; i < context->nb_streams; ++i)
        av_dict_free(&streamOptions[i]);
    delete[] streamOptions;
}

void LiveStreamWorker::openCodecs(AVFormatContext *context, AVDictionary *options)
{
    for (unsigned int i = 0; i < context->nb_streams; i++)
    {
        AVStream *stream = context->streams[i];
        bool codecOpened = openCodec(stream, options);
        if (!codecOpened)
        {
            qDebug() << "LiveStream: cannot find decoder for stream" << i << "codec" <<
                        stream->codec->codec_id;
            continue;
        }

        char info[512];
        avcodec_string(info, sizeof(info), stream->codec, 0);
        qDebug() << "LiveStream: stream #" << i << ":" << info;
    }
}

bool LiveStreamWorker::openCodec(AVStream *stream, AVDictionary *options)
{
    startInterruptableOperation();
    AVCodec *codec = avcodec_find_decoder(stream->codec->codec_id);

    AVDictionary *optionsCopy = 0;
    av_dict_copy(&optionsCopy, options, 0);
    startInterruptableOperation();
    int errorCode = avcodec_open2(stream->codec, codec, &optionsCopy);
    av_dict_free(&optionsCopy);

    return 0 == errorCode;
}

void LiveStreamWorker::startInterruptableOperation()
{
    m_lastInterruptableOperationStarted = QDateTime::currentDateTime();
}

LiveStreamFrame * LiveStreamWorker::frameToDisplay()
{
    return m_frameQueue.data()->dequeue();
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
