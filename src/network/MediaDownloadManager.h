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
