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

#ifndef EVENTVIDEODOWNLOAD_H
#define EVENTVIDEODOWNLOAD_H

#include "core/EventData.h"
#include <QFutureWatcher>
#include <QTimer>
#include <QUrl>

class QProgressDialog;
class MediaDownload;

class EventVideoDownload : public QObject
{
    Q_OBJECT

public:
    enum DownloadStatus
    {
        NotStarted,
        InProgress,
        CopyingFile,
        Finished,
        Failed
    };

    static QString statusToString(DownloadStatus status);

    explicit EventVideoDownload(const EventData &event, const QString &toFilePath, QObject *parent = 0);
    ~EventVideoDownload();

    void start();

    EventData eventData() const { return m_event; }
    MediaDownload * media() const { return m_mediaDownload; }
    DownloadStatus status() const { return m_status; }

signals:
    void statusChanged(EventVideoDownload::DownloadStatus status);
    void finished(EventVideoDownload *eventVideoDownload);

private slots:
    void startCopy();
    void copyFinished();

private:
    EventData m_event;
    MediaDownload *m_mediaDownload;
    DownloadStatus m_status;
    QString m_finalPath;
    QFutureWatcher<bool> *m_futureWatch;
    QString m_tempFilePath;

    void changeStatus(DownloadStatus status);
};

#endif // EVENTVIDEODOWNLOAD_H
