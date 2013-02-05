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

#ifndef EVENTDOWNLOADMANAGER_H
#define EVENTDOWNLOADMANAGER_H

#include <QObject>
#include <QQueue>

class QTimer;
class EventData;
class EventVideoDownload;

class EventDownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit EventDownloadManager(QObject *parent = 0);
    virtual ~EventDownloadManager();

    QString defaultFileName(const EventData &event) const;
    QString absoluteFileName(const QString &fileName) const;
    void updateLastSaveDirectory(const QString &fileName);

    void startEventDownload(const EventData &event, const QString &fileName);
    void startEventDownload(const EventData &event);
    void startMultipleEventDownloads(const QList<EventData> &events);

    QList<EventVideoDownload *> list() const { return m_eventVideoDownloadList; }

signals:
    void eventVideoDownloadAdded(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadRemoved(EventVideoDownload *eventVideoDownload);

private:
    QTimer *m_checkQueueTimer;
    QString m_lastSaveDirectory;
    QList<EventVideoDownload *> m_eventVideoDownloadList;
    QList<EventVideoDownload *> m_activeEventVideoDownloadList;
    QQueue<EventVideoDownload *> m_eventVideoDownloadQueue;

private slots:
    void checkQueue();
    void eventVideoDownloadFinished(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadDestroyed(QObject *destroyedObject);
};

#endif // EVENTDOWNLOADMANAGER_H
