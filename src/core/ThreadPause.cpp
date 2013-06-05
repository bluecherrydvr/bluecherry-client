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

#include "ThreadPause.h"
#include <QThread>

ThreadPause::ThreadPause() : m_paused(false)
{
}

ThreadPause::~ThreadPause()
{
    setPaused(false);
}

void ThreadPause::setPaused(bool paused)
{
    QMutexLocker mutexLocked(&m_pauseMutex);

    if (paused == m_paused)
        return;

    m_paused = paused;
    if (!m_paused)
        m_pauseWaitCondition.wakeOne();
}

bool ThreadPause::shouldPause()
{
    QMutexLocker mutexLocked(&m_pauseMutex);

    return m_paused;
}

void ThreadPause::pause()
{
    m_pauseWaitConditionMutex.lock();
    m_pauseWaitCondition.wait(&m_pauseWaitConditionMutex);
    m_pauseWaitConditionMutex.unlock();
}
