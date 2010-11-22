#include "VideoHttpBuffer.h"
#include "core/BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

void VideoHttpBuffer::needDataWrap(GstAppSrc *src, guint length, gpointer user_data)
{
    Q_UNUSED(src);
    static_cast<VideoHttpBuffer*>(user_data)->needData(length);
}

gboolean VideoHttpBuffer::seekDataWrap(GstAppSrc *src, quint64 offset, gpointer user_data)
{
    Q_UNUSED(src);
    return static_cast<VideoHttpBuffer*>(user_data)->seekData(offset) ? TRUE : FALSE;
}

VideoHttpBuffer::VideoHttpBuffer(GstAppSrc *element, GstElement *pipeline, QObject *parent)
    : QObject(parent), m_networkReply(0), m_fileSize(0), m_readPos(0), m_writePos(0), m_element(element), m_pipeline(pipeline),
      m_streamInit(true), m_bufferBlocked(false), m_finished(false), ratePos(0), rateMax(0)
{
    m_bufferFile.setFileTemplate(QDir::tempPath() + QLatin1String("/bc_vbuf_XXXXXX.mkv"));

    gst_app_src_set_max_bytes(m_element, 512*1024);
    gst_app_src_set_stream_type(m_element, GST_APP_STREAM_TYPE_SEEKABLE);

    GstAppSrcCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.need_data = needDataWrap;
    callbacks.seek_data = (gboolean (*)(GstAppSrc*,guint64,void*))seekDataWrap;
    gst_app_src_set_callbacks(m_element, &callbacks, this, 0);
}

VideoHttpBuffer::~VideoHttpBuffer()
{
    if (m_networkReply)
        delete m_networkReply;

    clearPlayback();
}

bool VideoHttpBuffer::start(const QUrl &url)
{
    Q_ASSERT(!m_networkReply);
    if (m_networkReply)
        delete m_networkReply;

    m_networkReply = bcApp->nam->get(QNetworkRequest(url));
    connect(m_networkReply, SIGNAL(readyRead()), SLOT(networkRead()));
    connect(m_networkReply, SIGNAL(finished()), SLOT(networkFinished()));
    connect(m_networkReply, SIGNAL(metaDataChanged()), SLOT(networkMetaData()));

    QMutexLocker lock(&m_lock);
    if (!m_bufferFile.open())
    {
        lock.unlock();
        sendStreamError(QString::fromLatin1("Failed to create buffer file: %1") + m_bufferFile.errorString());
        return false;
    }

    m_readPos = m_writePos = 0;
    lock.unlock();

    qDebug("VideoHttpBuffer: started");
    return true;
}

void VideoHttpBuffer::clearPlayback()
{
    QMutexLocker lock(&m_lock);

    if (m_element)
    {
        GstAppSrcCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        gst_app_src_set_callbacks(m_element, &callbacks, 0, 0);
    }

    m_element = 0;
    m_pipeline = 0;
    m_streamInit = m_bufferBlocked = false;

    lock.unlock();

    m_bufferWait.wakeAll();
}

void VideoHttpBuffer::cancelNetwork()
{
    /* This is just a test to make sure the mutex is locked by this thread. If it were recursive, this
     * would fail incorrectly. */
    Q_ASSERT(!m_lock.tryLock());

    if (m_networkReply)
        m_networkReply->abort();
}

void VideoHttpBuffer::sendStreamError(const QString &message)
{
    bool b = blockSignals(true);
    cancelNetwork();
    blockSignals(b);
    emit streamError(message);
}

void VideoHttpBuffer::networkMetaData()
{
    QMutexLocker lock(&m_lock);
    if (m_fileSize)
        return;

    bool ok = false;
    m_fileSize = m_networkReply->header(QNetworkRequest::ContentLengthHeader).toLongLong(&ok);

    if (!ok)
    {
        sendStreamError(QLatin1String("No content length for video"));
        m_fileSize = 0;
        return;
    }

    Q_ASSERT(m_bufferFile.isOpen());
    if (!m_bufferFile.resize(m_fileSize))
        qDebug() << "VideoHttpBuffer: Failed to resize buffer file:" << m_bufferFile.errorString();

    lock.unlock();

    gst_app_src_set_size(m_element, m_fileSize);
    emit bufferUpdated();
}

void VideoHttpBuffer::networkRead()
{
    QMutexLocker lock(&m_lock);
    Q_ASSERT(m_bufferFile.isOpen());

    if (!m_bufferFile.seek(m_writePos))
    {
        sendStreamError(tr("Seek in buffer file failed: %1").arg(m_bufferFile.errorString()));
        return;
    }

    char data[5120];
    qint64 r;
    while ((r = m_networkReply->read(data, sizeof(data))) > 0)
    {
        qint64 wr = m_bufferFile.write(data, r);
        if (wr < 0)
        {
            sendStreamError(tr("Write to buffer file failed: %1").arg(m_bufferFile.errorString()));
            return;
        }
        else if (wr < r)
        {
            sendStreamError(tr("Write to buffer file failed: %1").arg(QLatin1String("Data written to buffer is incomplete")));
            return;
        }

        m_writePos += wr;

        if ((unsigned)r < sizeof(data))
            break;
    }

    if (r < 0)
    {
        sendStreamError(tr("Failed to read video stream: %1").arg(m_networkReply->errorString()));
        return;
    }

    m_fileSize = qMax(m_fileSize, (qint64)m_writePos);
    lock.unlock();

    if (m_streamInit && (m_writePos >= 40960 || m_writePos == m_fileSize))
    {
        /* Attempt to initialize the stream with 40KB in the buffer */
        GstState stateNow, statePending;
        GstStateChangeReturn re = gst_element_get_state(m_pipeline, &stateNow, &statePending, GST_CLOCK_TIME_NONE);
        if (re == GST_STATE_CHANGE_FAILURE || stateNow < GST_STATE_PAUSED)
        {
            re = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
            if (re == GST_STATE_CHANGE_FAILURE)
                qWarning() << "VideoHttpBuffer: FAILED to transition pipeline into PAUSED state for initialization";
        }

        m_streamInit = false;
    }

    m_bufferWait.wakeOne();
    emit bufferUpdated();
}

void VideoHttpBuffer::networkFinished()
{
    qDebug("VideoHttpBuffer: Download finished");
    if (m_networkReply->error() == QNetworkReply::NoError)
    {
        if (m_fileSize != m_writePos)
        {
            qDebug() << "VideoHttpBuffer: Adjusting filesize to match actual downloaded amount";
            m_fileSize = m_writePos;
            gst_app_src_set_size(m_element, m_fileSize);
        }

        m_finished = true;
        emit bufferingFinished();
    }
    else
    {
        sendStreamError(QString::fromLatin1("Network error: %1").arg(m_networkReply->errorString()));
    }

    m_networkReply->deleteLater();
    m_networkReply = 0;

    m_bufferWait.wakeAll();
}

void VideoHttpBuffer::needData(unsigned size)
{
    /* Stop the stream if there is less than twice the amount requested, unless that is the end of the stream.
     * This gives the pipeline enough buffer to resume from the paused state properly.
     *
     * TODO: Calculate this buffer using time rather than bytes */

#if 0
    /* Record the request for rate estimation */
    GstClock *clock = gst_system_clock_obtain();
    addRateData(gst_clock_get_time(clock), size);
    gst_object_unref(GST_OBJECT(clock));

    quint64 edur;
    unsigned esize;
    getRateEstimation(&edur, &esize);
#endif

    QMutexLocker lock(&m_lock);
    if (!m_element || !m_pipeline)
        return;

    unsigned avail = unsigned(m_writePos - m_readPos);

    /* If there is less than three times as much as the current request, try to pause the stream now, but continue to
     * provide data. There may be another few requests before the stream actually can pause. */
    if (m_writePos < m_fileSize && avail < size*3)
    {
        /* Unlock prior to changing the pipeline state to prevent deadlocks.
         * Nothing here needs to be under the buffer lock. */
        m_bufferBlocked = true;
        lock.unlock();

        qDebug() << "VideoHttpBuffer: buffer is exhausted with" << (m_fileSize - m_readPos) << "bytes remaining in stream";

        GstState stateNow;
        GstStateChangeReturn re = gst_element_get_state(m_pipeline, &stateNow, 0, 0);
        Q_ASSERT(re != GST_STATE_CHANGE_FAILURE);
        if (re == GST_STATE_CHANGE_FAILURE || stateNow == GST_STATE_PLAYING)
        {
            re = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
            if (re == GST_STATE_CHANGE_FAILURE)
                qWarning() << "VideoHttpBuffer: FAILED to pause pipeline!";

            if (re == GST_STATE_CHANGE_ASYNC)
            {
                /* gst_element_get_state will block until the asynchronous state change completes */
                GstState stateNow, statePending;
                re = gst_element_get_state(m_pipeline, &stateNow, &statePending, GST_CLOCK_TIME_NONE);
            }
        }

        lock.relock();
        if (!m_element || !m_pipeline)
            return;
        avail = unsigned(m_writePos - m_readPos);
    }
    else
        m_bufferBlocked = false;

    while (avail < size && m_writePos < m_fileSize)
    {
        /* This exists mostly for stream initialization (when we don't
         * have enough buffered for smooth playback). We wake every second
         * to prevent any possibility of getting stuck in an indefinite wait. */
        bool b;
        do
        {
            b = m_bufferWait.wait(&m_lock, 1000);
            if (!m_element)
                return;
        } while (!b);
        avail = unsigned(m_writePos - m_readPos);
    }

    /* Refactor to use gst_pad_alloc_buffer? Probably wouldn't provide any benefit. */
    GstBuffer *buffer = gst_buffer_new_and_alloc(size);

    if (!m_bufferFile.seek(m_readPos))
    {
        qDebug() << "VideoHttpBuffer: Failed to seek for read:" << m_bufferFile.errorString();
        return;
    }

    GST_BUFFER_SIZE(buffer) = m_bufferFile.read((char*)GST_BUFFER_DATA(buffer), size);

    if (GST_BUFFER_SIZE(buffer) < 1)
    {
        gst_buffer_unref(buffer);

        if (m_bufferFile.atEnd())
        {
            lock.unlock();
            qDebug() << "VideoHttpBuffer: end of stream";
            gst_app_src_end_of_stream(m_element);
            return;
        }

        sendStreamError(QString::fromLatin1("Read error: %1").arg(m_bufferFile.errorString()));
        return;
    }

    m_readPos += size;
    GstAppSrc *e = m_element;
    lock.unlock();

    GstFlowReturn flow = gst_app_src_push_buffer(e, buffer);
    if (flow != GST_FLOW_OK)
        qDebug() << "VideoHttpBuffer: Push result is" << flow;
}

bool VideoHttpBuffer::seekData(qint64 offset)
{
    qDebug() << "VideoHttpBuffer: seek to" << offset;
    m_readPos = offset;
    m_bufferWait.wakeOne();
    return true;
}

void VideoHttpBuffer::addRateData(quint64 time, unsigned size)
{
    rateData[ratePos].time = time;
    rateData[ratePos].size = size;
    rateMax = qMax(rateMax, ++ratePos);
    if (ratePos == rateCount)
        ratePos = 0;
}

void VideoHttpBuffer::getRateEstimation(quint64 *duration, unsigned *size)
{
    *size = 0;
    if (!rateMax)
    {
        *duration = 0;
        return;
    }

    int end = ratePos ? (ratePos-1) : (rateMax-1);
    int begin = (ratePos == rateMax) ? 0 : ratePos;

    *duration = rateData[end].time - rateData[begin].time;
    for (int i = begin; i != end;)
    {
        *size += rateData[i].size;
        if (++i == rateMax)
            i = 0;
    }
    *size += rateData[end].size;

    qDebug() << "Data in" << *duration << "ns was" << *size;
}
