#ifndef MEDIADOWNLOAD_H
#define MEDIADOWNLOAD_H

#include <QObject>
#include <QUrl>
#include <QTemporaryFile>
#include <QWaitCondition>
#include <QMutex>
#include "utils/RangeMap.h"

class QNetworkReply;
class MediaDownloadTask;
class QNetworkCookie;

class MediaDownload : public QObject
{
    Q_OBJECT

public:
    explicit MediaDownload(QObject *parent = 0);
    virtual ~MediaDownload();

    unsigned fileSize() const { return m_fileSize; }
    unsigned readPosition() const { return m_readPos; }
    bool isFinished() const { return m_isFinished; }

    /* Read (up to) 'size' bytes at 'position' into 'buffer'
     *
     * This may block for a long time, because some reads may spawn
     * a new range request and wait for the requested amount of data to
     * be available.
     *
     * Pending calls to read will be aborted (returning 0) if seek()
     * is called. Returning -1 is a fatal error (and will be accompanied
     * by emitting the error(QString) signal). Otherwise, the return
     * value is the number of bytes read. */
    int read(unsigned position, char *buffer, int size);

    void start(const QUrl &url, const QList<QNetworkCookie> &cookies);

public slots:
    void cancel();
    /* Seek the stream to a specific byte offset
     *
     * For large jumps, or backwards jumps, at which the data is not available,
     * this may cause a new request to be launched at the specified position.
     *
     * Calling seek() will abort any read() which was blocked while waiting for
     * data, under the assumption that there is only one reader at a time, and
     * that reader will no longer be interested in previous requests in any
     * situation where a seek would be appropriate. */
    bool seek(unsigned offset);

signals:
    void started();
    void fileSizeChanged(unsigned fileSize);
    void finished();
    void stopped();
    void error(const QString &errorMessage);

private slots:
    /* Called on the network thread, beware! */
    void requestReady(unsigned fileSize);
    void incomingData(const QByteArray &data, unsigned position);

    void taskError(const QString &message);
    void taskFinished();

private:
    QUrl m_url;
    QThread *m_thread;
    MediaDownloadTask *m_task;
    QMutex m_bufferLock;
    QWaitCondition m_bufferWait;
    QTemporaryFile m_bufferFile;
    QFile m_readFile;
    QList<QNetworkCookie> m_cookies;
    unsigned m_fileSize, m_readPos, m_writePos;
    RangeMap m_bufferRanges;
    bool m_isFinished;

    /* size may be 0, which will continue to the end of the file */
    Q_INVOKABLE void startRequest(unsigned position, unsigned size);
    bool openFiles();
};

#endif // MEDIADOWNLOAD_H
