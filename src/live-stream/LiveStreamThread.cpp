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

#include "LiveStreamThread.h"
#include "LiveStreamWorker.h"
#include "core/LoggableUrl.h"
#include <QDebug>
#include <QThread>
#include <QUrl>

LiveStreamThread::LiveStreamThread(QObject *parent) : QObject(parent), m_isRunning(false)
{
}

LiveStreamThread::~LiveStreamThread()
{
    stop();
}

void LiveStreamThread::start(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO << LoggableUrl(url);

    if (!m_worker)
    {
        Q_ASSERT(!m_thread);
        m_thread = new QThread();

        LiveStreamWorker *worker = new LiveStreamWorker();
        worker->moveToThread(m_thread.data());

        m_worker = worker;
        m_worker.data()->setUrl(url);

        connect(m_thread.data(), SIGNAL(started()), m_worker.data(), SLOT(run()));
        connect(m_worker.data(), SIGNAL(destroyed()), m_thread.data(), SLOT(quit()));
        connect(m_worker.data(), SIGNAL(finished()), m_worker.data(), SLOT(deleteLater()));
        connect(m_thread.data(), SIGNAL(finished()), m_thread.data(), SLOT(deleteLater()));

        connect(m_worker.data(), SIGNAL(finished()), this, SIGNAL(finished()));
        connect(m_worker.data(), SIGNAL(finished()), this, SLOT(threadFinished()));
        connect(m_worker.data(), SIGNAL(fatalError(QString)), this, SIGNAL(fatalError(QString)));

        m_thread.data()->start();
    }
    else
        m_worker.data()->metaObject()->invokeMethod(m_worker.data(), "run");

    m_isRunning = true;
}

void LiveStreamThread::stop()
{
    m_isRunning = false;

    if (m_worker)
    {
        /* Worker will delete itself, which will then destroy the thread */
        m_worker.data()->stop();
        m_worker.clear();
        m_thread.clear();
    }

    Q_ASSERT(!m_thread);
}

void LiveStreamThread::setPaused(bool paused)
{
    m_worker.data()->setPaused(paused);
}

bool LiveStreamThread::hasWorker() const
{
    return !m_worker.isNull();
}

void LiveStreamThread::threadFinished()
{
    m_isRunning = false;
}

bool LiveStreamThread::isRunning() const
{
    return m_isRunning;
}

void LiveStreamThread::setAutoDeinterlacing(bool autoDeinterlacing)
{
    if (hasWorker())
        m_worker.data()->setAutoDeinterlacing(autoDeinterlacing);
}

LiveStreamFrame * LiveStreamThread::frameToDisplay() const
{
    if (hasWorker())
        return m_worker.data()->frameToDisplay();
    else
        return 0;
}
