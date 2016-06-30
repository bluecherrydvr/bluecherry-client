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

#ifndef RTSPSTREAMWORKER_H
#define RTSPSTREAMWORKER_H

#include "core/ThreadPause.h"
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <QSharedPointer>

struct AVDictionary;
struct AVFrame;
struct AVStream;

class RtspStreamFrame;
class RtspStreamFrameFormatter;
class RtspStreamFrameQueue;

class RtspStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit RtspStreamWorker(QSharedPointer<RtspStreamFrameQueue> &shared_queue, QObject *parent = 0);
    virtual ~RtspStreamWorker();

    void setUrl(const QUrl &url);

    void stop();
    void setPaused(bool paused);
    void setAutoDeinterlacing(bool autoDeinterlacing);

    bool shouldInterrupt() const;
    RtspStreamFrame * frameToDisplay();

public slots:
    void run();

signals:
    void fatalError(const QString &message);
    void finished();
    void bytesDownloaded(unsigned int bytes);
    void foundAudioStream();

private:
    struct AVFormatContext *m_ctx;
    QDateTime m_timeout;
    QUrl m_url;
    bool m_cancelFlag;
    bool m_autoDeinterlacing;
    mutable bool m_lastCancel;
    mutable int m_lastSeconds;
    int m_decodeErrorsCnt;
    int m_videoStreamIndex;
    int m_audioStreamIndex;

    ThreadPause m_threadPause;
    QScopedPointer<RtspStreamFrameFormatter> m_frameFormatter;
    QSharedPointer<RtspStreamFrameQueue> m_frameQueue;


    bool setup();
    bool prepareStream(AVFormatContext **context, AVDictionary *options);
    AVDictionary * createOptions() const;
    bool openInput(AVFormatContext **context, AVDictionary *options);
    bool findStreamInfo(AVFormatContext *context, AVDictionary *options);
    AVDictionary ** createStreamsOptions(AVFormatContext *context, AVDictionary *options) const;
    void destroyStreamOptions(AVFormatContext *context, AVDictionary **streamOptions);
    void openCodecs(AVFormatContext *context, AVDictionary *options);
    bool openCodec(AVStream *stream, AVDictionary *options);

    void pause();

    void processStreamLoop();
    bool processStream();
    struct AVPacket readPacket(bool *ok = 0);
    bool processPacket(struct AVPacket packet);
    AVFrame * extractVideoFrame(struct AVPacket &packet);
    void processFrame(struct AVFrame *frame);

    QString errorMessageFromCode(int errorCode);
    void startInterruptableOperation(int timeoutInSeconds);

};

#endif // RTSPSTREAMWORKER_H
