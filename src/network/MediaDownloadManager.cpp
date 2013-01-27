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
