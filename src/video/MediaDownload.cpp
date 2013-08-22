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

#include "MediaDownload.h"
#include "MediaDownload_p.h"
#include "core/BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QMutexLocker>
#include <QThread>
#include <QDir>
#include <QApplication>
#include <QTextDocument>
#include <QDebug>

/* Minimum number of bytes delta between the current write position
 * and the forward seek position before we will launch a new ranged
 * request. This parameter should probably be based on a rough approximation
 * of time rather than any fixed number of bytes. */
static const unsigned seekMinimumSkip = 96000;

QThreadStorage<QNetworkAccessManager*> MediaDownloadTask::threadNAM;

MediaDownload::MediaDownload(const QUrl &url, const QList<QNetworkCookie> &cookies, QObject *parent)
    : QObject(parent), m_url(url), m_cookies(cookies), m_thread(0), m_task(0), m_fileSize(0),
      m_downloadedSize(0), m_readPos(0), m_writePos(0), m_refCount(0), m_isFinished(false), m_hasError(false)
{
    Q_ASSERT(m_url.isValid());

    m_bufferFile.setFileTemplate(QDir::tempPath() + QLatin1String("/bc_vbuf_XXXXXX.mkv"));
}

MediaDownload::~MediaDownload()
{
    if (m_task)
    {
        m_task->metaObject()->invokeMethod(m_task, "abort");
        m_task->deleteLater();
        m_task = 0;
    }

    if (m_thread)
    {
        m_thread->quit();
        m_thread = 0;
    }

    if (m_bufferFile.isOpen())
        m_bufferFile.close();
}

void MediaDownload::ref()
{
    m_refCount.fetchAndAddOrdered(1);
}

bool MediaDownload::deref()
{
    Q_ASSERT(m_refCount > 0);
    if (m_refCount.fetchAndAddOrdered(-1) == 1)
    {
        deleteLater();
        return true;
    }

    return false;
}

void MediaDownload::start()
{
    if (m_thread)
        return; // already started

    if (!openFiles())
    {
        /* openFiles calls sendError */
        return;
    }

    m_thread = new QThread();
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    m_thread->start();

    qDebug() << "MediaDownload: started for" << m_url;

    startRequest(0, 0);
}

void MediaDownload::cancel()
{

}

void MediaDownload::sendError(const QString &message)
{
    qDebug() << Q_FUNC_INFO << message;

    m_hasError = true;
    emit error(message);
    emit stopped();
    m_bufferWait.wakeAll();
}

bool MediaDownload::openFiles()
{
    if (m_bufferFile.isOpen())
        return true;

    if (!m_bufferFile.open())
    {
        sendError(QLatin1String("Failed to open write buffer: ") + m_bufferFile.errorString());
        return false;
    }

    m_readFile.setFileName(m_bufferFile.fileName());
    if (!m_readFile.open(QIODevice::ReadOnly))
    {
        sendError(QLatin1String("Failed to open read buffer: ") + m_readFile.errorString());
        return false;
    }

    return true;
}

QString MediaDownload::bufferFilePath() const
{
    /* Safe on account of this being static after initialization, which is
     * prior to any other usage in our case. */
    return m_bufferFile.fileName();
}

bool MediaDownload::seek(unsigned offset)
{
    QMutexLocker l(&m_bufferLock);
    if (m_hasError)
        return false;

    if (offset > m_fileSize)
    {
        qDebug() << "MediaDownload: seek offset" << offset << "is past the end of filesize" << m_fileSize;
        return false;
    }

    if (m_readPos == offset)
        return true;

    m_readPos = offset;

    Range missingRange = m_bufferRanges.nextMissingRange(Range::fromStartEnd(m_readPos - qMin(m_readPos, seekMinimumSkip), m_fileSize));
    if (missingRange.isValid())
    {
        if (missingRange.start() >= m_writePos && missingRange.start() - m_writePos < seekMinimumSkip)
        {
            /* We're already downloading at the correct position; nothing to do */
        }
        else
        {
            qDebug() << "MediaDownload: launching new request after seek";
            m_writePos = missingRange.start();
            startRequest(missingRange.start(), missingRange.size());
        }
    }

    m_bufferWait.wakeAll();
    return true;
}

int MediaDownload::read(unsigned position, char *buffer, int reqSize)
{
    QMutexLocker l(&m_bufferLock);
    if (m_hasError)
        return -1;

    unsigned oldRdPos = m_readPos;
    int size = qMin(reqSize, (m_fileSize >= position) ? int(m_fileSize - position) : 0);

    while (!m_bufferRanges.contains(Range::fromStartSize(position, size)))
    {
        m_bufferWait.wait(&m_bufferLock);
        if (oldRdPos != m_readPos)
        {
            /* reading stream has seeked, abort this read */
            return 0;
        }
        if (m_hasError)
            return -1;

        size = qMin(reqSize, (m_fileSize >= position) ? int(m_fileSize - position) : 0);
    }

    l.unlock();

    if (!m_readFile.seek(position))
    {
        sendError(QLatin1String("Buffer seek error: ") + m_readFile.errorString());
        return -1;
    }

    int re = m_readFile.read(buffer, size);
    if (re < 0)
    {
        /* Called from VideoHttpBuffer::needData. We handle error reporting,
         * as we have more information here. */
        sendError(QLatin1String("Buffer read error: ") + m_readFile.errorString());
        return -1;
    }

    if (m_readPos == position)
        m_readPos += re;

    return re;
}

void MediaDownload::startRequest(unsigned position, unsigned size)
{
    Q_ASSERT(m_url.isValid());

    if (m_task)
    {
        m_task->abortLater();
        m_task->deleteLater();
    }

    m_task = new MediaDownloadTask;
    m_task->moveToThread(m_thread);

    connect(m_task, SIGNAL(requestReady(uint)), SLOT(requestReady(uint)),
            Qt::DirectConnection);
    connect(m_task, SIGNAL(dataRead(QByteArray,uint)), SLOT(incomingData(QByteArray,uint)),
            Qt::DirectConnection);
    connect(m_task, SIGNAL(finished()), SLOT(taskFinished()), Qt::DirectConnection);
    connect(m_task, SIGNAL(error(QString)), SLOT(taskError(QString)), Qt::DirectConnection);

    /* If size will reach the end of what we believe the file size to be, make it infinite instead,
     * to ease behavior with still active files */
    if (position + size >= m_fileSize)
        size = 0;

    bool ok = m_task->metaObject()->invokeMethod(m_task, "start", Q_ARG(QUrl, m_url),
                                                 Q_ARG(QList<QNetworkCookie>, m_cookies),
                                                 Q_ARG(unsigned, position),
                                                 Q_ARG(unsigned, size));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}

void MediaDownload::requestReady(unsigned fileSize)
{
    /* This may happen multiple times, with new requests, and the file size
     * may change as a result (for active events) */

    QMutexLocker l(&m_bufferLock);

    bool emitSizeChanged = false;
    if (fileSize > m_fileSize)
    {
        m_fileSize = fileSize;
        emitSizeChanged = true;

        Q_ASSERT(m_bufferFile.isOpen());
        if (!m_bufferFile.resize(m_fileSize))
        {
            sendError(QLatin1String("Buffer resize failed: ") + m_bufferFile.errorString());
            return;
        }
    }

    l.unlock();

    if (emitSizeChanged)
        emit fileSizeChanged(fileSize);
}

void MediaDownload::incomingData(const QByteArray &data, unsigned position)
{
    QMutexLocker l(&m_bufferLock);
    bool emitFileSize = false;

    if (position+data.size() > m_fileSize)
    {
        qDebug() << "MediaDownload: file size is less than write position, adjusting size";
        m_fileSize = position + data.size();
        if (!m_bufferFile.resize(m_fileSize))
        {
            sendError(QLatin1String("Buffering failed: ") + m_bufferFile.errorString());
            return;
        }
        emitFileSize = true;
    }

    Q_ASSERT(m_bufferFile.size() == m_fileSize);

    if (!m_bufferFile.seek(position))
    {
        sendError(QLatin1String("Buffer write failed: ") + m_bufferFile.errorString());
        return;
    }

    qint64 re = m_bufferFile.write(data);
    if (re < 0)
    {
        sendError(QLatin1String("Buffer write failed: ") + m_bufferFile.errorString());
        return;
    }

    if (!m_bufferFile.flush())
        qDebug() << "MediaDownload: Buffer flush after write failed (non-critical):" << m_bufferFile.errorString();

    Q_ASSERT(re == data.size());

    m_bufferRanges.insert(Range::fromStartSize(position, re));

    if (m_writePos == position)
        m_writePos += re;

    m_downloadedSize += re;

    l.unlock();

    if (emitFileSize)
        emit fileSizeChanged(m_fileSize);

    m_bufferWait.wakeAll();
}

void MediaDownload::taskError(const QString &message)
{
    qDebug() << Q_FUNC_INFO << message;

    m_isFinished = true;
    sendError(message);
}

void MediaDownload::taskFinished()
{
    QMutexLocker l(&m_bufferLock);

    /* These should both be true or both be false, anything else is a logic error.
     * This test does assume that we will never download the same byte twice. */
    Q_ASSERT(!(m_bufferRanges.contains(Range::fromStartSize(0, m_fileSize)) ^ (m_downloadedSize >= m_fileSize)));

    if (m_bufferRanges.contains(Range::fromStartSize(0, m_fileSize)))
    {
        qDebug() << "MediaDownload: Media finished";
        m_isFinished = true;
        bool ok = metaObject()->invokeMethod(this, "finished", Qt::QueuedConnection);
        Q_ASSERT(ok);
        ok = metaObject()->invokeMethod(this, "stopped", Qt::QueuedConnection);
        Q_ASSERT(ok);
        Q_UNUSED(ok);

        if (m_thread)
        {
            m_thread->quit();
            m_thread = 0;
        }
    }
    else
    {
        /* Launch a new task to fill in gaps. Prioritize anything that is missing and is closest
         * to the current read position. */
        Range missingRange = m_bufferRanges.nextMissingRange(Range::fromStartEnd(m_readPos, m_fileSize));
        Q_ASSERT(missingRange.isValid());

        m_writePos = missingRange.start();
        startRequest(missingRange.start(), missingRange.size());
    }
}

MediaDownloadTask::MediaDownloadTask(QObject *parent)
    : QObject(parent), m_reply(0), m_writePos(0)
{
}

void MediaDownloadTask::start(const QUrl &url, const QList<QNetworkCookie> &cookies, unsigned position,
                              unsigned size)
{
    Q_ASSERT(!m_reply);

    if (!threadNAM.hasLocalData())
    {
        threadNAM.setLocalData(new QNetworkAccessManager);
        /* XXX certificate validation */
    }

    threadNAM.localData()->cookieJar()->setCookiesFromUrl(cookies, url);
    if (threadNAM.localData()->cookieJar()->cookiesForUrl(url).isEmpty())
        qDebug() << "MediaDownload: No cookies for media URL, likely to fail authentication";

    QNetworkRequest req(url);
    if (position || size)
    {
        QByteArray range = "bytes=" + QByteArray::number(position) + "-";
        if (size)
            range += QByteArray::number(position + size);
        req.setRawHeader("Range", range);
    }

    m_writePos = position;

    m_reply = threadNAM.localData()->get(req);
    m_reply->ignoreSslErrors(); // XXX Do this properly!
    connect(m_reply, SIGNAL(metaDataChanged()), SLOT(metaDataReady()));
    connect(m_reply, SIGNAL(readyRead()), SLOT(read()));
    connect(m_reply, SIGNAL(finished()), SLOT(requestFinished()));
}

MediaDownloadTask::~MediaDownloadTask()
{
    abort();
}

void MediaDownloadTask::abortLater()
{
    /* Mostly threadsafe; caller must know that the instance will not be deleted at this time. */
    if (m_reply)
        m_reply->disconnect(this);
    metaObject()->invokeMethod(this, "abort", Qt::QueuedConnection);
}

void MediaDownloadTask::abort()
{
    if (!m_reply)
        return;

    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = 0;
}

void MediaDownloadTask::metaDataReady()
{
    int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (m_reply->error() != QNetworkReply::NoError || status < 200 || status > 299) {
        if (m_reply->error() != QNetworkReply::NoError)
            emit error(m_reply->errorString());
        else
            emit error(Qt::escape(m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()));
        disconnect(m_reply, 0, this, 0);
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    unsigned size = m_reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    if (size)
        emit requestReady(m_writePos + size);
}

void MediaDownloadTask::read()
{
    QByteArray data = m_reply->readAll();
    if (data.isEmpty())
        return;

    emit dataRead(data, m_writePos);
    m_writePos += data.size();

    /* Very carefully threadsafe: bcApp and the globalRate pointer are
     * const, and 'addSampleValue' is threadsafe and lockfree for the common case. */
    bcApp->globalRate->addSampleValue(data.size());
}

void MediaDownloadTask::requestFinished()
{
    if (m_reply->error() != QNetworkReply::NoError && m_reply->error() != QNetworkReply::UnknownNetworkError)
        emit error(m_reply->errorString());
    else
        emit finished();

    m_reply->deleteLater();
    m_reply = 0;
}
