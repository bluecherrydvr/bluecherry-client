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

#ifndef MEDIADOWNLOADMANAGER_H
#define MEDIADOWNLOADMANAGER_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QUrl>

class MediaDownload;
class QNetworkCookieJar;

class MediaDownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit MediaDownloadManager(QObject *parent = 0);
    virtual ~MediaDownloadManager();

    QNetworkCookieJar *cookieJar() const { return m_cookieJar; }
    void setCookieJar(QNetworkCookieJar *cookieJar);

    MediaDownload * acquireMediaDownload(const QUrl &url);
    void releaseMediaDownload(const QUrl &url);

private:
    QMap<QUrl, MediaDownload *> m_mediaDownloads;
    QMutex m_mapMutex;
    QNetworkCookieJar *m_cookieJar;

    MediaDownload * getOrCreate(const QUrl &url);
};

#endif // MEDIADOWNLOADMANAGER_H
