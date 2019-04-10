/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#include "EventVideoDownload.h"
#include "core/BluecherryApp.h"
#include "server/DVRServer.h"
#include "video/MediaDownload.h"
#include "network/MediaDownloadManager.h"
#include <QtConcurrentRun>
#include <QFutureWatcher>

QString EventVideoDownload::statusToString(DownloadStatus status)
{
    switch (status)
    {
    case NotStarted: return tr("Preparing to download");
    case InProgress: return tr("Downloading video");
    case CopyingFile: return tr("Copying file");
    case Finished: return tr("Video downloaded");
    case Failed: return tr("Video download failed");
    }

    return QString();
}

EventVideoDownload::EventVideoDownload(const EventData &event, const QString &toFilePath, QObject *parent)
    : QObject(parent), m_event(event), m_mediaDownload(0), m_status(NotStarted), m_finalPath(toFilePath), m_futureWatch(0)
{
    Q_ASSERT(!m_finalPath.isEmpty());
}

EventVideoDownload::~EventVideoDownload()
{
    if (m_mediaDownload && m_event.server())
    {
        QUrl eventDownloadUrl = m_event.server()->url().resolved(QUrl(QLatin1String("/media/request.php")));
        eventDownloadUrl.addQueryItem(QLatin1String("id"), QString::number(m_event.mediaId()));

        bcApp->mediaDownloadManager()->releaseMediaDownload(eventDownloadUrl);
        m_mediaDownload = 0;
    }
}

void EventVideoDownload::start()
{
    if (!m_event.server())
        return;

    QUrl eventDownloadUrl = m_event.server()->url().resolved(QUrl(QLatin1String("/media/request.php")));
    eventDownloadUrl.addQueryItem(QLatin1String("id"), QString::number(m_event.mediaId()));

    m_mediaDownload = bcApp->mediaDownloadManager()->acquireMediaDownload(eventDownloadUrl);
    changeStatus(InProgress);

    if (!m_mediaDownload->isFinished())
    {
        connect(m_mediaDownload, SIGNAL(finished()), this, SLOT(startCopy()));
        m_mediaDownload->start();
    }
    else
        startCopy();
}

void EventVideoDownload::changeStatus(DownloadStatus status)
{
    if (status == m_status)
        return;

    m_status = status;
    emit statusChanged(m_status);

    if (Finished == m_status)
        emit finished(this);
}

void EventVideoDownload::startCopy()
{
    if (!m_mediaDownload || m_finalPath.isEmpty())
    {
        qWarning() << "EventVideoDownload::startCopy: Invalid parameters";
        changeStatus(Failed);
        return;
    }

    m_tempFilePath = m_mediaDownload->bufferFilePath();
    if (m_tempFilePath.isEmpty())
    {
        qWarning() << "EventVideoDownload::startCopy: No buffer file to copy from";
        changeStatus(Failed);
        return;
    }

    if (QFile::exists(m_finalPath))
    {
        if (!QFile::remove(m_finalPath))
        {
            qDebug() << "EventVideoDownload: Failed to replace video file";
            changeStatus(Failed);
            /* TODO: proper error */
            return;
        }
    }

    QFuture<bool> re = QtConcurrent::run(&QFile::copy, m_tempFilePath, m_finalPath);
    m_futureWatch = new QFutureWatcher<bool>(this);
    m_futureWatch->setFuture(re);

    connect(m_futureWatch, SIGNAL(finished()), SLOT(copyFinished()));

    changeStatus(CopyingFile);
}

void EventVideoDownload::copyFinished()
{
    Q_ASSERT(m_futureWatch);

    m_futureWatch->deleteLater();
    m_futureWatch = 0;

    changeStatus(Finished);
}
