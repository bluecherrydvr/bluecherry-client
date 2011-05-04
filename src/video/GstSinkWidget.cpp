#include "GstSinkWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QApplication>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

GstSinkWidget::GstSinkWidget(QWidget *parent)
    : QGLWidget(parent), m_framePtr(0), m_frameWidth(-1), m_frameHeight(-1)
{
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_element = GST_APP_SINK(gst_element_factory_make("appsink", "sinkwidget"));
    if (!m_element)
    {
        qWarning() << "GstSinkWidget: Creating appsink element failed";
        return;
    }

    g_object_ref(m_element);

    GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                                        "red_mask", G_TYPE_INT, 0xff00,
                                        "blue_mask", G_TYPE_INT, 0xff000000,
                                        "green_mask", G_TYPE_INT, 0xff0000,
                                        NULL);
    gst_app_sink_set_caps(m_element, caps);
    gst_caps_unref(caps);

    GstAppSinkCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.eos = &GstSinkWidget::wrapEos;
    callbacks.new_buffer = &GstSinkWidget::wrapNewBuffer;
    callbacks.new_preroll = &GstSinkWidget::wrapNewPreroll;
    gst_app_sink_set_callbacks(m_element, &callbacks, this, NULL);
}

GstSinkWidget::~GstSinkWidget()
{
    qDebug("gstreamer: Destroying sink widget");

    /* Changing to NULL should always be synchronous; assertation should verify this. */
    GstStateChangeReturn re = gst_element_set_state(GST_ELEMENT(m_element), GST_STATE_NULL);
    Q_UNUSED(re);
    Q_ASSERT(re == GST_STATE_CHANGE_SUCCESS);

    /* At this point, it should not be possible to receive any more signals from the element,
     * and the pipeline (if it still exists) is broken. */
    g_object_unref(m_element);
    m_element = 0;

    m_frameLock.lock();
    if (m_framePtr)
    {
        /* Corresponding to the ref done in updateFrame prior to setting the value */
        gst_buffer_unref(m_framePtr);
        m_framePtr = 0;
    }
    m_frameLock.unlock();
}

QSize GstSinkWidget::sizeHint() const
{
    return QSize(m_frameWidth, m_frameHeight);
}

void GstSinkWidget::setOverlayMessage(const QString &message)
{
    if (message == m_overlayMsg)
        return;

    m_overlayMsg = message;
    update();
}

void GstSinkWidget::setBufferStatus(int percent)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    if (percent == 100)
        clearOverlayMessage();
    //else
      //  setOverlayMessage(tr("Buffering\n%1%").arg(percent));
}

QImage GstSinkWidget::currentFrame()
{
    if (m_frameWidth < 0 || m_frameHeight < 0)
        return QImage();

    m_frameLock.lock();
    GstBuffer *buffer = m_framePtr;
    if (buffer)
        gst_buffer_ref(buffer);
    m_frameLock.unlock();

    if (!buffer)
        return QImage();

    QImage re = QImage(GST_BUFFER_DATA(buffer), m_frameWidth, m_frameHeight, QImage::Format_RGB32);
    re.bits(); // force a deep copy
    gst_buffer_unref(buffer);

    return re;
}

/* It is technically possible to draw into QGLWidget from another thread, if things are changed
 * to protect the context. This would provide a big benefit in terms of latency and CPU usage
 * here. */

void GstSinkWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setBackground(QColor(Qt::black));

    m_frameLock.lock();
    GstBuffer *buffer = m_framePtr;
    if (buffer)
        gst_buffer_ref(buffer);
    m_frameLock.unlock();

    if (!buffer)
    {
        p.fillRect(rect(), Qt::black);
        return;
    }

    QImage frame = QImage(GST_BUFFER_DATA(buffer), m_frameWidth, m_frameHeight, QImage::Format_RGB32);

    QRect r = rect();
    p.eraseRect(r);

    QSize scaledSize = frame.size();
    scaledSize.scale(r.size(), Qt::KeepAspectRatio);
    r.adjust((r.width() - scaledSize.width()) / 2, (r.height() - scaledSize.height()) / 2, 0, 0);
    r.setSize(scaledSize);

    p.drawImage(r, frame);

    gst_buffer_unref(buffer);

    if (!m_overlayMsg.isEmpty())
    {
        QFont font = p.font();
        font.setPixelSize(35);
        p.setFont(font);

        QRect textRect = p.fontMetrics().boundingRect(r, Qt::AlignCenter, m_overlayMsg);
        QRect textBannerRect = QRect(r.x(), qMax(0, textRect.y() - 20), r.width(), qMin(r.height(), textRect.height() + 40));


        p.fillRect(textBannerRect, QColor(0, 0, 0, 160));
        p.setPen(Qt::white);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::HighQualityAntialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.drawText(textRect, Qt::AlignCenter, m_overlayMsg);
    }
}

void GstSinkWidget::endOfStream()
{
    qDebug() << "GstSinkWidget: end of stream";
}

void GstSinkWidget::updateFrame(GstBuffer *buffer)
{
    if (m_frameWidth < 0 || m_frameHeight < 0)
        return;

    Q_ASSERT(buffer && GST_BUFFER_DATA(buffer));

    gst_buffer_ref(buffer);
    m_frameLock.lock();
    if (m_framePtr)
        gst_buffer_unref(m_framePtr);
    m_framePtr = buffer;
    m_frameLock.unlock();

    QMetaObject::invokeMethod(this, "repaint", Qt::QueuedConnection);
}

GstFlowReturn GstSinkWidget::newPreroll()
{
    if (!m_element)
        return GST_FLOW_UNEXPECTED;

    GstBuffer *buffer = gst_app_sink_pull_preroll(m_element);
    if (!buffer)
        return GST_FLOW_ERROR;

    GstCaps *caps = GST_BUFFER_CAPS(buffer);
    Q_ASSERT(caps);

    if (!gst_caps_is_fixed(caps))
    {
        qDebug() << "GstSinkWidget: Expecting fixed caps in preroll; ignoring for now";
        gst_buffer_unref(buffer);
        return GST_FLOW_OK;
    }

    GstStructure *cs = gst_caps_get_structure(caps, 0);
    Q_ASSERT(cs);

    if (!gst_structure_get_int(cs, "width", &m_frameWidth) ||
        !gst_structure_get_int(cs, "height", &m_frameHeight))
    {
        qWarning() << "GstSinkWidget: No frame dimensions available";
        gst_buffer_unref(buffer);
        return GST_FLOW_ERROR;
    }

    updateFrame(buffer);

    gst_buffer_unref(buffer);
    return GST_FLOW_OK;
}

GstFlowReturn GstSinkWidget::newBuffer()
{
    if (!m_element || m_frameWidth < 0 || m_frameHeight < 0)
    {
        qDebug() << "GstSinkWidget: Unexpected newBuffer";
        return GST_FLOW_UNEXPECTED;
    }

    GstBuffer *buffer = gst_app_sink_pull_buffer(m_element);
    if (!buffer)
        return GST_FLOW_UNEXPECTED;

    updateFrame(buffer);
    gst_buffer_unref(buffer);

    return GST_FLOW_OK;
}

void GstSinkWidget::wrapEos(GstAppSink *sink, gpointer user_data)
{
    Q_ASSERT(user_data);
    Q_UNUSED(sink);
    static_cast<GstSinkWidget*>(user_data)->endOfStream();
}

GstFlowReturn GstSinkWidget::wrapNewBuffer(GstAppSink *sink, gpointer user_data)
{
    Q_ASSERT(user_data);
    Q_UNUSED(sink);
    return static_cast<GstSinkWidget*>(user_data)->newBuffer();
}

GstFlowReturn GstSinkWidget::wrapNewPreroll(GstAppSink *sink, gpointer user_data)
{
    Q_ASSERT(user_data);
    Q_UNUSED(sink);
    return static_cast<GstSinkWidget*>(user_data)->newPreroll();
}
