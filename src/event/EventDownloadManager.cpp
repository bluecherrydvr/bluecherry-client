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

#include "EventDownloadManager.h"
#include "core/BluecherryApp.h"
#include "core/EventData.h"
#include "server/DVRServer.h"
#include "event/EventVideoDownload.h"
#include "ui/MainWindow.h"
#include "utils/StringUtils.h"
#include <QFileDialog>
#include <QSettings>
#include <QTimer>
#include <QUrl>

#define MAX_CONCURRENT_DOWNLOADS 30

EventDownloadManager::EventDownloadManager(QObject *parent)
    : QObject(parent)
{
     QSettings settings;
     m_lastSaveDirectory = settings.value(QLatin1String("download/lastSaveDirectory")).toString();

     m_checkQueueTimer = new QTimer(this);
     connect(m_checkQueueTimer, SIGNAL(timeout()), this, SLOT(checkQueue()));
     m_checkQueueTimer->start(1000);
}

EventDownloadManager::~EventDownloadManager()
{
}

QString EventDownloadManager::defaultFileName(const EventData &event) const
{
    return withSuffix(event.baseFileName(), QLatin1String(".mkv"));
}

QString EventDownloadManager::absoluteFileName(const QString &fileName) const
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.isAbsolute())
        return fileInfo.absoluteFilePath();

    fileInfo.setFile(QString::fromLatin1("%1/%2").arg(m_lastSaveDirectory).arg(fileName));
    return fileInfo.absoluteFilePath();
}

void EventDownloadManager::updateLastSaveDirectory(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QString newSaveDirectory = fileInfo.absoluteDir().absolutePath();

    if (m_lastSaveDirectory != newSaveDirectory)
    {
        m_lastSaveDirectory = newSaveDirectory;

        QSettings settings;
        settings.setValue(QLatin1String("download/lastSaveDirectory"), m_lastSaveDirectory);
    }
}

void EventDownloadManager::startEventDownload(const EventData &event, const QString &fileName)
{
    if (fileName.isEmpty())
        return;

    QString saveFileName = absoluteFileName(withSuffix(fileName, QLatin1String(".mkv")));
    updateLastSaveDirectory(saveFileName);

    EventVideoDownload *dl = new EventVideoDownload(event, saveFileName, bcApp->mainWindow);
    m_eventVideoDownloadQueue.append(dl);

    m_eventVideoDownloadList.append(dl);
    connect(dl, SIGNAL(destroyed(QObject*)), this, SLOT(eventVideoDownloadDestroyed(QObject*)));
    emit eventVideoDownloadAdded(dl);
}

void EventDownloadManager::startEventDownload(const EventData &event)
{
    QString fileName = absoluteFileName(defaultFileName(event));
    QString saveFileName = QFileDialog::getSaveFileName(bcApp->mainWindow, tr("Save event video"),
                                                        fileName, tr("Matroska Video (*.mkv)"));

    startEventDownload(event, saveFileName);
}

void EventDownloadManager::startMultipleEventDownloads(const QList<EventData> &events)
{
    QString dirName = QFileDialog::getExistingDirectory(bcApp->mainWindow, tr("Save event videos"),
                                                        m_lastSaveDirectory,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirName.isEmpty())
        return;

    foreach (const EventData &event, events)
        startEventDownload(event, QString::fromLatin1("%1/%2").arg(dirName, defaultFileName(event)));
}

void EventDownloadManager::checkQueue()
{
    while (m_activeEventVideoDownloadList.size() < MAX_CONCURRENT_DOWNLOADS && !m_eventVideoDownloadQueue.isEmpty())
    {
        EventVideoDownload *dl = m_eventVideoDownloadQueue.dequeue();
        connect(dl, SIGNAL(finished(EventVideoDownload*)), this, SLOT(eventVideoDownloadFinished(EventVideoDownload*)));
        dl->start();
        m_activeEventVideoDownloadList.append(dl);
    }
}

void EventDownloadManager::eventVideoDownloadFinished(EventVideoDownload *eventVideoDownload)
{
    m_activeEventVideoDownloadList.removeAll(eventVideoDownload);
}

void EventDownloadManager::eventVideoDownloadDestroyed(QObject *destroyedObject)
{
    EventVideoDownload *dl = static_cast<EventVideoDownload *>(destroyedObject);
    m_eventVideoDownloadList.removeAll(dl);
    m_activeEventVideoDownloadList.removeAll(dl);
    m_eventVideoDownloadQueue.removeAll(dl);
    emit eventVideoDownloadRemoved(dl);
}
