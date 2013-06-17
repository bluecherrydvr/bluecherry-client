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

#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include "core/ThreadPause.h"
#include <QDateTime>
#include <QObject>

struct AVDictionary;
struct AVStream;

class LiveStreamFrame;
class LiveStreamFrameFormatter;
class LiveStreamFrameQueue;

class LiveStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamWorker(QObject *parent = 0);
    virtual ~LiveStreamWorker();

    void setUrl(const QByteArray &url);

    void stop();
    void setPaused(bool paused);

    bool shouldInterrupt() const;
    LiveStreamFrame * frameToDisplay();

public slots:
    void run();

    void setAutoDeinterlacing(bool autoDeinterlacing);

signals:
    void fatalError(const QString &message);
    void finished();

private:
    struct AVFormatContext *m_ctx;
    QDateTime m_lastInterruptableOperationStarted;
    QByteArray m_url;
    bool m_cancelFlag;
    bool m_autoDeinterlacing;

    ThreadPause m_threadPause;
    QScopedPointer<LiveStreamFrameFormatter> m_frameFormatter;
    QScopedPointer<LiveStreamFrameQueue> m_frameQueue;

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
    struct AVFrame * extractFrame(struct AVPacket &packet);
    void processFrame(struct AVFrame *frame);

    QString errorMessageFromCode(int errorCode);
    void startInterruptableOperation();

};

#endif // LIVESTREAMWORKER_H
