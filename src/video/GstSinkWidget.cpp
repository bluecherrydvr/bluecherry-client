#include "GstSinkWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

GstSinkWidget::GstSinkWidget(QWidget *parent)
    : QWidget(parent), m_frameWidth(-1), m_frameHeight(-1)
{
    m_element = GST_APP_SINK(gst_element_factory_make("appsink", "sinkwidget"));
    if (!m_element)
    {
        qWarning() << "GstSinkWidget: Creating appsink element failed";
        return;
    }

    GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                                        "red_mask", G_TYPE_INT, 0xff00,
                                        "blue_mask", G_TYPE_INT, 0xff000000,
                                        "green_mask", G_TYPE_INT, 0xff0000,
                                        NULL);
    gst_app_sink_set_caps(m_element, caps);

    GstAppSinkCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.eos = &GstSinkWidget::wrapEos;
    callbacks.new_buffer = &GstSinkWidget::wrapNewBuffer;
    callbacks.new_preroll = &GstSinkWidget::wrapNewPreroll;
    gst_app_sink_set_callbacks(m_element, &callbacks, this, NULL);
}

QSize GstSinkWidget::sizeHint() const
{
    return QSize();
}

void GstSinkWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawImage(QPoint(0, 0), m_latestFrame);
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
    m_latestFrame = QImage(GST_BUFFER_DATA(buffer), m_frameWidth, m_frameHeight, QImage::Format_RGB32);
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

    updateFrame(buffer);
    gst_buffer_unref(buffer);

    qDebug() << "GstSinkWiget: preroll is ready; caps:" << gst_caps_to_string(caps); // MEMORY LEAK
    if (!gst_caps_is_fixed(caps))
    {
        qWarning() << "GstSinkWidget: Expecting fixed caps";
        return GST_FLOW_ERROR;
    }

    GstStructure *cs = gst_caps_get_structure(caps, 0);
    Q_ASSERT(cs);

    if (!gst_structure_get_int(cs, "width", &m_frameWidth) ||
        !gst_structure_get_int(cs, "height", &m_frameHeight))
    {
        qWarning() << "GstSinkWidget: No frame dimensions available";
        gst_caps_unref(caps);
        return GST_FLOW_ERROR;
    }

    gst_caps_unref(caps);

    qDebug() << "GstSinkWidget: video is" << m_frameWidth << m_frameHeight;
    return GST_FLOW_OK;
}

GstFlowReturn GstSinkWidget::newBuffer()
{
    if (!m_element || m_frameWidth < 0 || m_frameHeight < 0)
    {
        qDebug() << "GstSinkWidget: Unexpected newBuffer";
        return GST_FLOW_UNEXPECTED;
    }

    qDebug() << "GstSinkWidget: new buffer is ready";
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
