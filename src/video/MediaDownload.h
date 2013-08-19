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

#ifndef MEDIADOWNLOAD_H
#define MEDIADOWNLOAD_H

#include <QObject>
#include <QUrl>
#include <QTemporaryFile>
#include <QWaitCondition>
#include <QMutex>
#include <QAtomicInt>
#include "utils/RangeMap.h"

class QNetworkReply;
class MediaDownloadTask;
class QNetworkCookie;

class MediaDownload : public QObject
{
    Q_OBJECT

public:
    explicit MediaDownload(const QUrl &url, const QList<QNetworkCookie> &cookies, QObject *parent = 0);
    virtual ~MediaDownload();

    /* Reference counting used by VideoHttpBuffer and
     * EventVideoDownload to track if we need to keep this
     * download alive because one of the two is still using
     * it. */
    void ref();
    bool deref();

    unsigned fileSize() const { return m_fileSize; }
    unsigned downloadedSize() const { return m_downloadedSize; }
    unsigned readPosition() const { return m_readPos; }
    bool isFinished() const { return m_isFinished; }

    QString bufferFilePath() const;

    bool hasData(unsigned int offset, unsigned int bytes);
    /* Read (up to) 'size' bytes at 'position' into 'buffer'
     *
     * This may block for a long time, because some reads may spawn
     * a new range request and wait for the requested amount of data to
     * be available.
     *
     * Pending calls to read will be aborted (returning 0) if seek()
     * is called. */
    QByteArray read(unsigned position, int size);

    void start();

    QUrl url() const { return m_url; }

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
    void newDataAvailable();

private slots:
    /* Called on the network thread, beware! */
    void requestReady(unsigned fileSize);
    void incomingData(const QByteArray &data, unsigned position);

    void taskError(const QString &errorMessage);
    void taskFinished();

    void sendError(const QString &errorMessage);

private:
    QUrl m_url;
    QList<QNetworkCookie> m_cookies;
    QThread *m_thread;
    MediaDownloadTask *m_task;
    QMutex m_bufferLock;
    QWaitCondition m_bufferWait;
    QTemporaryFile m_bufferFile;
    QFile m_readFile;
    unsigned m_fileSize, m_downloadedSize, m_readPos, m_writePos;
    RangeMap m_bufferRanges;
    QAtomicInt m_refCount;
    bool m_isFinished, m_hasError;

    /* size may be 0, which will continue to the end of the file */
    Q_INVOKABLE void startRequest(unsigned position, unsigned size);
    bool openFiles();
};

#endif // MEDIADOWNLOAD_H
