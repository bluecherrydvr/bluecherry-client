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
#include "core/BluecherryApp.h"
#include "core/LoggableUrl.h"
#include <QDebug>
#include <QThread>
#include <QUrl>

LiveStreamThread::LiveStreamThread(const QUrl &url, QObject *parent) :
        QObject(parent)
{
    qDebug() << Q_FUNC_INFO << LoggableUrl(url);

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
    connect(m_worker.data(), SIGNAL(fatalError(QString)), this, SIGNAL(fatalError(QString)));

    connect(m_worker.data(), SIGNAL(bytesDownloaded(uint)), bcApp->globalRate, SLOT(addSampleValue(uint)));

    m_thread.data()->start();
}

LiveStreamThread::~LiveStreamThread()
{
    if (m_worker.data())
        m_worker.data()->stop();

    m_worker.clear();
    m_thread.clear();
}

void LiveStreamThread::setPaused(bool paused)
{
    m_worker.data()->setPaused(paused);
}

LiveStreamWorker * LiveStreamThread::worker() const
{
    return m_worker.data();
}
