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
#include <QDebug>

/* Minimum number of bytes delta between the current write position
 * and the forward seek position before we will launch a new ranged
 * request. This parameter should probably be based on a rough approximation
 * of time rather than any fixed number of bytes. */
static const unsigned seekMinimumSkip = 256000;

QThreadStorage<QNetworkAccessManager*> MediaDownloadTask::threadNAM;

MediaDownload::MediaDownload(QObject *parent)
    : QObject(parent), m_thread(0), m_task(0), m_fileSize(0), m_downloadedSize(0),
      m_readPos(0), m_writePos(0), m_isFinished(false)
{
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
        m_thread->quit();
}

void MediaDownload::start(const QUrl &url, const QList<QNetworkCookie> &cookies)
{
    Q_ASSERT(!m_thread && !m_url.isValid());

    m_bufferFile.setFileTemplate(QDir::tempPath() + QLatin1String("/bc_vbuf_XXXXXX.mkv"));

    if (!openFiles())
    {
        qDebug() << "MediaDownload: ERROR!";
        return;
    }

    m_thread = new QThread(this);
    m_thread->start();

    qDebug() << "MediaDownload: started for" << url;

    m_url = url;
    m_cookies = cookies;
    startRequest(0, 0);
}

void MediaDownload::cancel()
{

}

bool MediaDownload::openFiles()
{
    Q_ASSERT(!m_bufferFile.isOpen());
    if (!m_bufferFile.open())
    {
        qDebug() << "MediaDownload: failed to open buffer file for write:" << m_bufferFile.errorString();
        return false;
    }

    m_readFile.setFileName(m_bufferFile.fileName());
    if (!m_readFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "MediaDownload: failed to open buffer file for read:" << m_readFile.errorString();
        return false;
    }

    return true;
}

bool MediaDownload::seek(unsigned offset)
{
    QMutexLocker l(&m_bufferLock);
    //qDebug() << "MediaDownload: seek from" << m_readPos << "to" << offset << "of" << m_fileSize;
    if (offset > m_fileSize)
        return false;

    if (m_readPos == offset)
        return true;

    m_readPos = offset;

    unsigned npos, nsize;
    if (m_bufferRanges.nextMissingRange(m_readPos - qMin(m_readPos, seekMinimumSkip), m_fileSize, npos, nsize))
    {
        if (npos >= m_writePos && npos - m_writePos < seekMinimumSkip)
        {
            qDebug() << "MediaDownload: but writepos is" << m_writePos << "so nothing is needed";
        }
        else
        {
            qDebug() << "MediaDownload: launching new request after seek";
            qDebug() << "MediaDownload: rdpos" << m_readPos << "next missing is" << npos << nsize;
            qDebug() << m_bufferRanges;
            m_writePos = npos;
            startRequest(npos, nsize);
        }
    }

    m_bufferWait.wakeAll();

    /* XXX even when the seek position is available
     * within cached ranges, we should move our request
     * to the nearest time after that which is not buffered,
     * under the assumption that the user will want to keep
     * playing the video from here (or to seek forward farther). */
    return true;
}

int MediaDownload::read(unsigned position, char *buffer, int reqSize)
{
    QMutexLocker l(&m_bufferLock);
    unsigned oldRdPos = m_readPos;
    int size = qMin(reqSize, (m_fileSize >= position) ? int(m_fileSize - position) : 0);

    while (!m_bufferRanges.contains(position, size))
    {
        qDebug() << "MediaDownload: blocking" << QThread::currentThread() << "to wait for read of"
                 << size << "at" << position;
        qDebug() << m_bufferRanges;
        m_bufferWait.wait(&m_bufferLock);
        if (oldRdPos != m_readPos)
        {
            qDebug() << "MediaDownload: aborting blocked read(), readPos changed";
            /* reading stream has seeked, abort this read */
            return 0;
        }

        size = qMin(reqSize, (m_fileSize >= position) ? int(m_fileSize - position) : 0);
    }

    if (m_readPos == position)
        m_readPos += size;

    l.unlock();

    //qDebug() << "MediaDownload: read of" << size << "at" << position;

    m_readFile.seek(position);
    int re = m_readFile.read(buffer, size);
    if (re < 0)
    {
        qDebug() << "MediaDownload: read error:" << m_readFile.errorString();
        return -1;
    }

    return re;
}

void MediaDownload::startRequest(unsigned position, unsigned size)
{
    Q_ASSERT(m_url.isValid());

    if (m_task)
    {
        qDebug() << "MediaDownload: Aborting existing download task";
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

    qDebug() << "MediaDownload: Starting task for position" << position << "of size" << size << "from thread"
             << QThread::currentThread();

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
    if (fileSize != m_fileSize)
    {
        qDebug() << "MediaDownload: file size changed from" << m_fileSize << "to" << fileSize;
        m_fileSize = fileSize;
        metaObject()->invokeMethod(this, "fileSizeChanged", Q_ARG(unsigned, fileSize));
    }
    l.unlock();

    Q_ASSERT(m_bufferFile.isOpen());
    m_bufferFile.resize(fileSize);
}

void MediaDownload::incomingData(const QByteArray &data, unsigned position)
{
    m_bufferFile.seek(position);
    m_bufferFile.write(data);

    QMutexLocker l(&m_bufferLock);
    qDebug() << "MediaDownload: Received" << data.size() << "bytes at" << position << "thread is"
                << QThread::currentThread();
    m_bufferRanges.insert(position, data.size());

    if (m_writePos == position)
        m_writePos += data.size();

    m_downloadedSize += data.size();

    if (m_fileSize < position+data.size())
    {
        qDebug() << "MediaDownload: file size is less than write position, adjusting size";
        m_fileSize = position + data.size();
        metaObject()->invokeMethod(this, "fileSizeChanged", Q_ARG(unsigned, m_fileSize));
    }

    m_bufferWait.wakeAll();
}

void MediaDownload::taskError(const QString &message)
{
    qDebug() << "MediaDownload: Task reports error:" << message;
}

void MediaDownload::taskFinished()
{    
    QMutexLocker l(&m_bufferLock);
    qDebug() << "MediaDownload: Task finished";

    /* These should both be true or both be false, anything else is a logic error. */
    Q_ASSERT(!(m_bufferRanges.contains(0, m_fileSize) ^ (m_downloadedSize >= m_fileSize)));

    if (m_bufferRanges.contains(0, m_fileSize))
    {
        qDebug() << "MediaDownload: Media finished";
        m_isFinished = true;
        bool ok = metaObject()->invokeMethod(this, "finished", Qt::QueuedConnection);
        Q_ASSERT(ok);
        ok = metaObject()->invokeMethod(this, "stopped", Qt::QueuedConnection);
        Q_ASSERT(ok);
        Q_UNUSED(ok);
    }
    else
    {
        /* Launch a new task to fill in gaps. Prioritize anything that is missing and is closest
         * to the current read position. */
        unsigned npos, nsize;
        if (!m_bufferRanges.nextMissingRange(m_readPos, m_fileSize, npos, nsize))
        {
            if (npos >= m_fileSize && !m_bufferRanges.nextMissingRange(0, m_fileSize, npos, nsize))
            {
                /* Should not be possible; this would mean that 0-m_fileSize is included. */
                Q_ASSERT_X(false, "RangeMap", "Impossible conflict between nextMissingRange and contains");
                m_isFinished = true;
                bool ok = metaObject()->invokeMethod(this, "finished", Qt::QueuedConnection);
                Q_ASSERT(ok);
                ok = metaObject()->invokeMethod(this, "stopped", Qt::QueuedConnection);
                Q_ASSERT(ok);
                Q_UNUSED(ok);
                return;
            }
        }

        qDebug() << "MediaDownload: Want to start request at" << npos << "for" << nsize;
        m_writePos = npos;
        startRequest(npos, nsize);
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
    if (position)
    {
        QByteArray range = "bytes=" + QByteArray::number(position) + "-";
        if (size)
            range += QByteArray::number(position + size);
        req.setRawHeader("Range", range);
        m_writePos = position;
    }

    m_reply = threadNAM.localData()->get(req);
    m_reply->ignoreSslErrors(); // XXX Do this properly!
    connect(m_reply, SIGNAL(metaDataChanged()), SLOT(metaDataReady()));
    connect(m_reply, SIGNAL(readyRead()), SLOT(read()));
    connect(m_reply, SIGNAL(finished()), SLOT(requestFinished()));

    qDebug() << "MediaDownloadTask: started, thread is" << QThread::currentThread();
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
    qDebug() << "MediaDownloadTask: abort called on thread" << QThread::currentThread();
    if (!m_reply)
    {
        qDebug() << "MediaDownloadTask: abort with no m_reply";
        return;
    }

    qDebug() << "MediaDownloadTask: aborting for request of" << m_reply->request().rawHeader("Range")
             << "with write position" << m_writePos << "and bytesAvailable" << m_reply->bytesAvailable();

    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = 0;
}

void MediaDownloadTask::metaDataReady()
{
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
}

void MediaDownloadTask::requestFinished()
{
    if (m_reply->error() != QNetworkReply::NoError)
        emit error(m_reply->errorString());
    else
    {
        qDebug() << "MediaDownloadTask finished" << m_reply->errorString() << m_reply->bytesAvailable();
        emit finished();
    }

    m_reply->deleteLater();
    m_reply = 0;
}
