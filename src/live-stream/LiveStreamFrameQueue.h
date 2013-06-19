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

#ifndef LIVE_STREAM_FRAME_QUEUE_H
#define LIVE_STREAM_FRAME_QUEUE_H

#include "core/ThreadPause.h"
#include <QDateTime>
#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QQueue>

class LiveStreamFrame;

class LiveStreamFrameQueue
{
    Q_DISABLE_COPY(LiveStreamFrameQueue);

public:
    LiveStreamFrameQueue(quint16 sizeLimit);
    ~LiveStreamFrameQueue();

    LiveStreamFrame * dequeue();
    void enqueue(LiveStreamFrame *frame);
    void clear();

private:
    QMutex m_frameQueueLock;
    QQueue<LiveStreamFrame *> m_frameQueue;
    quint16 m_sizeLimit;
    qint64 m_ptsBase;
    QElapsedTimer m_ptsTimer;

    void dropOldFrames();

};

#endif // LIVE_STREAM_FRAME_QUEUE_H
