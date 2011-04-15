#ifndef MEDIADOWNLOAD_P_H
#define MEDIADOWNLOAD_P_H

#include <QObject>
#include <QUrl>
#include <QThreadStorage>

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
    MediaDownloadTask(QObject *parent = 0);
    virtual ~MediaDownloadTask();

public slots:
    void start(const QUrl &url, const QList<QNetworkCookie> &cookies, unsigned postion);
    void abort();

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
    static QThreadStorage<QNetworkAccessManager*> threadNAM;
    QNetworkReply *m_reply;
    unsigned m_writePos;
};

#endif // MEDIADOWNLOAD_P_H
