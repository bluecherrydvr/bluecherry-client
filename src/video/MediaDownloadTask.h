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

#ifndef MEDIADOWNLOAD_TASK_H
#define MEDIADOWNLOAD_TASK_H

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkCookie;

/* Represents an actual download task on behalf of MediaDownload.
 * There is one instance of this object per HTTP request, living
 * within a worker thread. It includes a position, and provides
 * data at the given position via a signal (direct) to MediaDownload,
 * which handles buffering that to file and other concerns.
 *
 * There will generally only be one of these active per MediaDownload
 * at a time, but there may be slight overlap when seeking. This is
 * handled properly by MediaDownload.
 */
class MediaDownloadTask : public QObject
{
    Q_OBJECT

public:
    explicit MediaDownloadTask(const QUrl &url, const QList<QNetworkCookie> &cookies, unsigned position, unsigned size, QObject *parent = 0);
    virtual ~MediaDownloadTask();

public slots:
    void startDownload();
    void abort();
    void abortLater();

signals:
    void requestReady(unsigned fileSize);
    void dataRead(const QByteArray &data, unsigned position);
    void error(const QString &errorMessage);
    void finished();

private slots:
    void metaDataReady();
    void read();
    void requestFinished();

private:
    friend class MediaDownload;

    QUrl m_url;
    QList<QNetworkCookie> m_cookies;
    unsigned m_position;
    unsigned m_size;
    QNetworkReply *m_reply;

};

#endif // MEDIADOWNLOAD_TASK_H
