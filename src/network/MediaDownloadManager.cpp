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

#include "MediaDownloadManager.h"
#include "video/MediaDownload.h"
#include <QNetworkCookieJar>

MediaDownloadManager::MediaDownloadManager(QObject *parent)
    : QObject(parent), m_cookieJar(0)
{
}

MediaDownloadManager::~MediaDownloadManager()
{
}

void MediaDownloadManager::setCookieJar(QNetworkCookieJar *cookieJar)
{
    m_cookieJar = cookieJar;
}

MediaDownload * MediaDownloadManager::getOrCreate(const QUrl &url)
{
    Q_ASSERT(url.isValid());

    if (m_mediaDownloads.contains(url))
        return m_mediaDownloads.value(url);

    QList<QNetworkCookie> cookies = m_cookieJar ? m_cookieJar->cookiesForUrl(url) : QList<QNetworkCookie>();
    MediaDownload *mediaDownload = new MediaDownload(url, cookies);
    m_mediaDownloads.insert(url, mediaDownload);
    return mediaDownload;
}

MediaDownload * MediaDownloadManager::acquireMediaDownload(const QUrl &url)
{
    if (!url.isValid())
        return 0;

    QMutexLocker mapMutexLocker(&m_mapMutex);
    Q_UNUSED(mapMutexLocker);

    MediaDownload *mediaDownload = getOrCreate(url);
    mediaDownload->ref();

    return mediaDownload;
}

void MediaDownloadManager::releaseMediaDownload(const QUrl &url)
{
    if (!url.isValid())
        return;

    QMutexLocker mapMutexLocker(&m_mapMutex);
    Q_UNUSED(mapMutexLocker);

    if (m_mediaDownloads.contains(url))
    {
        MediaDownload *mediaDownload = m_mediaDownloads.value(url);

        if (mediaDownload->deref())
            m_mediaDownloads.remove(url);
    }
}
