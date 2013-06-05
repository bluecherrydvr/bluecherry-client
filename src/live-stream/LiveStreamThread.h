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

#ifndef LIVE_STREAM_THREAD_H
#define LIVE_STREAM_THREAD_H

#include <QObject>
#include <QWeakPointer>

class LiveStreamWorker;
class QThread;

class LiveStreamThread : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamThread(QObject *parent = 0);
    virtual ~LiveStreamThread();

    void start(const QByteArray &url);
    void stop();
    void setPaused(bool paused);

    LiveStreamWorker * worker() const;

signals:
    void fatalError(const QString &error);

private:
    QWeakPointer<QThread> m_thread;
    QWeakPointer<LiveStreamWorker> m_worker;

};

#endif // LIVE_STREAM_THREAD_H
