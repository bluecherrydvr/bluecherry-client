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

#include "RtspStreamFrameQueue.h"
#include "RtspStreamFrame.h"

extern "C" {
#   include "libavcodec/avcodec.h"
#   include "libavutil/avutil.h"
#   include "libavutil/mathematics.h"
}

#define RENDER_TIMER_FPS 30

RtspStreamFrameQueue::RtspStreamFrameQueue(quint16 sizeLimit) :
        m_frameQueueLock(QMutex::NonRecursive), m_sizeLimit(sizeLimit), m_ptsBase(AV_NOPTS_VALUE)
{
}

RtspStreamFrameQueue::~RtspStreamFrameQueue()
{
    clear();
}

RtspStreamFrame * RtspStreamFrameQueue::dequeue()
{
    QMutexLocker locker(&m_frameQueueLock);
    if (m_frameQueue.isEmpty())
        return 0;

    if (m_ptsBase == (int64_t)AV_NOPTS_VALUE)
    {
        m_ptsBase = m_frameQueue.head()->avFrame()->pts;
        m_ptsTimer.start();
    }

    //qint64 now = m_ptsTimer.elapsed() * 1000;

    // TODO: needs checking
    // something is wrong with this code as after few minutes all frames are considered outdated - some calculation is off here
    RtspStreamFrame *frame = m_frameQueue.dequeue();
    /*while (!m_frameQueue.isEmpty() && frame)
    {
        qint64 scaledFrameDisplayTime = av_rescale_rnd(frame->avFrame()->pts - m_ptsBase, AV_TIME_BASE, 90000, AV_ROUND_NEAR_INF);
        if (abs(scaledFrameDisplayTime - now) >= AV_TIME_BASE/2)
        {
            m_ptsBase = frame->avFrame()->pts;
            m_ptsTimer.start();
            now = scaledFrameDisplayTime = 0;
        }

        if (now >= scaledFrameDisplayTime || (scaledFrameDisplayTime - now) <= AV_TIME_BASE/(RENDER_TIMER_FPS*2))
        {
            delete frame;
            frame = m_frameQueue.dequeue();
        }
        else
            break;
    }*/

    return frame;
}

void RtspStreamFrameQueue::enqueue(RtspStreamFrame *frame)
{
    if (!frame)
        return;

    QMutexLocker locker(&m_frameQueueLock);
    m_frameQueue.enqueue(frame);

    dropOldFrames();
}

void RtspStreamFrameQueue::clear()
{
    QMutexLocker locker(&m_frameQueueLock);
    qDeleteAll(m_frameQueue);
    m_frameQueue.clear();
}

// Calling this method should be proteced by m_frameQueueLock
void RtspStreamFrameQueue::dropOldFrames()
{
    while (m_frameQueue.size() >= 6)
    {
        RtspStreamFrame *frame = m_frameQueue.dequeue();
        delete frame;
    }
}
