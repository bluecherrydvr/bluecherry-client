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

#ifndef RTSP_STREAM_THREAD_H
#define RTSP_STREAM_THREAD_H

#include <QMutex>
#include <QObject>
#include <QWeakPointer>
#include <QSharedPointer>

class RtspStreamFrame;
class RtspStreamWorker;
class RtspStreamFrameQueue;
class QThread;
class QUrl;

class RtspStreamThread : public QObject
{
    Q_OBJECT

public:
    explicit RtspStreamThread(QObject *parent = 0);
    virtual ~RtspStreamThread();

    void start(const QUrl &url);
    void stop();
    void setPaused(bool paused);

    bool isRunning() const;
    bool hasWorker();

    void setAutoDeinterlacing(bool autoDeinterlacing);
    RtspStreamFrame * frameToDisplay();

signals:
    void fatalError(const QString &error);
    void finished();
    void foundAudioStream();

private:
    QWeakPointer<QThread> m_thread;
    QWeakPointer<RtspStreamWorker> m_worker;
    QSharedPointer<RtspStreamFrameQueue> m_frameQueue;
    QMutex m_workerMutex;
    bool m_isRunning;

private slots:
    void clearWorker();
};

#endif // RTSP_STREAM_THREAD_H
