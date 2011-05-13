#include "VideoHttpBuffer.h"
#include "core/BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QNetworkCookieJar>
#include <QApplication>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

void VideoHttpBuffer::needDataWrap(GstAppSrc *src, guint length, gpointer user_data)
{
    Q_UNUSED(src);
    Q_ASSERT(length < INT_MAX);
    static_cast<VideoHttpBuffer*>(user_data)->needData(int(length));
}

gboolean VideoHttpBuffer::seekDataWrap(GstAppSrc *src, quint64 offset, gpointer user_data)
{
    Q_UNUSED(src);
    return static_cast<VideoHttpBuffer*>(user_data)->seekData(offset) ? TRUE : FALSE;
}

VideoHttpBuffer::VideoHttpBuffer(QObject *parent)
    : QObject(parent), media(0), m_element(0), m_pipeline(0)
{
}

VideoHttpBuffer::~VideoHttpBuffer()
{
    clearPlayback();
}

GstElement *VideoHttpBuffer::setupSrcElement(GstElement *pipeline)
{
    Q_ASSERT(!m_element && !m_pipeline);
    if (m_element || m_pipeline)
    {
        if (m_pipeline == pipeline)
            return GST_ELEMENT(m_element);
        return 0;
    }

    m_element = GST_APP_SRC(gst_element_factory_make("appsrc", "source"));
    if (!m_element)
        return 0;

    g_object_ref(m_element);

    m_pipeline = pipeline;

    gst_app_src_set_max_bytes(m_element, 0);
    gst_app_src_set_stream_type(m_element, GST_APP_STREAM_TYPE_RANDOM_ACCESS);

    GstAppSrcCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.need_data = needDataWrap;
    callbacks.seek_data = (gboolean (*)(GstAppSrc*,guint64,void*))seekDataWrap;
    gst_app_src_set_callbacks(m_element, &callbacks, this, 0);

    if (media && media->fileSize())
        gst_app_src_set_size(m_element, media->fileSize());

    gst_bin_add(GST_BIN(m_pipeline), GST_ELEMENT(m_element));
    return GST_ELEMENT(m_element);
}

bool VideoHttpBuffer::start(const QUrl &url)
{
    Q_ASSERT(!media);
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    media = new MediaDownload(this);
    connect(media, SIGNAL(fileSizeChanged(uint)), SLOT(fileSizeChanged(uint)));
    connect(media, SIGNAL(finished()), SIGNAL(bufferingFinished()));
    connect(media, SIGNAL(stopped()), SIGNAL(bufferingStopped()));

    media->start(url, bcApp->nam->cookieJar()->cookiesForUrl(url));

    qDebug("VideoHttpBuffer: started");
    emit bufferingStarted();

    return true;
}

void VideoHttpBuffer::clearPlayback()
{
    if (m_element)
    {
        qDebug("gstreamer: Destroying source element");

        GstAppSrcCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        gst_app_src_set_callbacks(m_element, &callbacks, 0, 0);

        GstStateChangeReturn re = gst_element_set_state(GST_ELEMENT(m_element), GST_STATE_NULL);
        Q_UNUSED(re);
        Q_ASSERT(re == GST_STATE_CHANGE_SUCCESS);

        g_object_unref(m_element);
    }

    m_element = 0;
    m_pipeline = 0;
}

void VideoHttpBuffer::sendStreamError(const QString &message)
{
    bool b = blockSignals(true);
    blockSignals(b);
    emit streamError(message);
    emit bufferingStopped();
}

void VideoHttpBuffer::fileSizeChanged(unsigned fileSize)
{
    if (!fileSize)
        qDebug() << "VideoHttpBuffer: fileSize is 0, may cause problems!";

    if (m_element)
    {
        qDebug("VideoHttpBuffer: setting filesize");
        gst_app_src_set_size(m_element, fileSize);
    }
}

#if 0
void VideoHttpBuffer::networkRead()
{
    qDebug("trying to acquire lock for networkRead...");
    QMutexLocker lock(&m_lock);
    qDebug("acquired");
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

    if (m_streamInit && false && (m_writePos >= 40960 || m_writePos == m_fileSize))
    {
        qDebug("streamInit");
        /* Attempt to initialize the stream with 40KB in the buffer */
        GstState stateNow, statePending;
        GstStateChangeReturn re = gst_element_get_state(m_pipeline, &stateNow, &statePending, GST_CLOCK_TIME_NONE);
        if (re == GST_STATE_CHANGE_FAILURE || stateNow < GST_STATE_PAUSED)
        {
            qDebug("got state");
            re = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
            qDebug("set state re=%d", (int)re);
            if (re == GST_STATE_CHANGE_FAILURE)
                qWarning() << "VideoHttpBuffer: FAILED to transition pipeline into PAUSED state for initialization";
        }

        m_streamInit = false;
    }

    qDebug("waking bufferwait");
    m_bufferWait.wakeOne();
    emit bufferUpdated();
}

void VideoHttpBuffer::networkFinished()
{
    QMutexLocker lock(&m_lock);
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
        m_networkReply->deleteLater();
        m_networkReply = 0;
        emit bufferingFinished();
        emit bufferingStopped();
    }
    else
    {
        QString error = m_networkReply->errorString();
        m_networkReply->deleteLater();
        m_networkReply = 0;
        sendStreamError(QString::fromLatin1("Network error: %1").arg(error));
    }

    m_bufferWait.wakeAll();
}
#endif

void VideoHttpBuffer::needData(int size)
{
    Q_ASSERT(media);

    /* Refactor to use gst_pad_alloc_buffer? Probably wouldn't provide any benefit. */
    GstBuffer *buffer = gst_buffer_new_and_alloc(size);

    int re = media->read(media->readPosition(), (char*)GST_BUFFER_DATA(buffer), size);
    if (re < 0)
    {
        qDebug() << "VideoHttpBuffer: read error";
        return;
    }
    else if (re == 0)
    {
        if (media->readPosition() >= media->fileSize() && media->isFinished())
        {
            qDebug() << "VideoHttpBuffer: end of stream";
            gst_app_src_end_of_stream(m_element);
        }
        else
            qDebug() << "VideoHttpBuffer: read aborted";
        return;
    }

    GST_BUFFER_SIZE(buffer) = re;

    GstFlowReturn flow = gst_app_src_push_buffer(m_element, buffer);
    if (flow != GST_FLOW_OK)
        qDebug() << "VideoHttpBuffer: Push result is" << flow;
}

bool VideoHttpBuffer::seekData(qint64 offset)
{
    Q_ASSERT(media);
    return media->seek((unsigned)offset);
}
