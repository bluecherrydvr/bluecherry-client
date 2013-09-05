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

#include "GstSinkWidget.h"
#include "core/BluecherryApp.h"
#include "video/gst/GstVideoPlayerBackend.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QApplication>
#include <QGLWidget>
#include <QSettings>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

GstSinkWidget::GstSinkWidget(QWidget *parent)
    : VideoWidget(parent), m_viewport(0), m_element(0), m_framePtr(0), m_frameWidth(-1),
      m_frameHeight(-1), m_normalFrameStyle(0)
{
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMinimumSize(320, 240);
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    p.setColor(QPalette::WindowText, Qt::white);
    setPalette(p);

    QSettings settings;
    if (!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), true).toBool())
        setViewport(new QGLWidget);
    else
    {
        qDebug("Hardware-accelerated video output is DISABLED");
        setViewport(new QWidget);
    }

    connect(bcApp, SIGNAL(settingsChanged()), SLOT(settingsChanged()));
}

GstSinkWidget::~GstSinkWidget()
{
    destroyElement();
}

void GstSinkWidget::initVideo(VideoPlayerBackend *videoPlayerBackend)
{
    GstVideoPlayerBackend *gstVideoPlayerBackend = reinterpret_cast<GstVideoPlayerBackend *>(videoPlayerBackend);

    GstElement *sink = createElement();
    Q_ASSERT(sink);

    gstVideoPlayerBackend->setSink(sink);
}

void GstSinkWidget::clearVideo()
{
    destroyElement();
}

void GstSinkWidget::setViewport(QWidget *w)
{
    if (m_viewport)
    {
        m_viewport->removeEventFilter(this);
        m_viewport->deleteLater();
    }

    m_viewport = w;
    m_viewport->setParent(this);
    m_viewport->setGeometry(contentsRect());
    m_viewport->setAutoFillBackground(false);
    m_viewport->setAttribute(Qt::WA_OpaquePaintEvent);
    m_viewport->installEventFilter(this);
    m_viewport->show();
}

void GstSinkWidget::settingsChanged()
{
    QSettings settings;
    bool hwaccel = !settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), true).toBool();
    bool old = m_viewport->inherits("QGLWidget");
    if (hwaccel != old)
    {
        qDebug("%s hardware acceleration for video", hwaccel ? "Enabled" : "Disabled");
        if (hwaccel)
            setViewport(new QGLWidget);
        else
            setViewport(new QWidget);
    }
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
    m_viewport->update();
}

void GstSinkWidget::setBufferStatus(int percent)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    if (percent == 100)
        clearOverlayMessage();
}

void GstSinkWidget::setFullScreen(bool on)
{
    if (on)
    {
        setWindowFlags(windowFlags() | Qt::Window);
        m_normalFrameStyle = frameStyle();
        setFrameStyle(QFrame::NoFrame);
        showFullScreen();
    }
    else
    {
        setWindowFlags(windowFlags() & ~Qt::Window);
        setFrameStyle(m_normalFrameStyle);
        showNormal();
    }

    QSettings settings;
    if (settings.value(QLatin1String("ui/disableScreensaver/onFullscreen")).toBool())
        bcApp->setScreensaverInhibited(on);
}

void GstSinkWidget::resizeEvent(QResizeEvent *ev)
{
    QFrame::resizeEvent(ev);
    m_viewport->setGeometry(contentsRect());
}

void GstSinkWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ev->accept();
    toggleFullScreen();
}

void GstSinkWidget::keyPressEvent(QKeyEvent *ev)
{
    if (ev->modifiers() != 0)
        return;

    switch (ev->key())
    {
    case Qt::Key_Escape:
        setFullScreen(false);
        break;
    default:
        return;
    }

    ev->accept();
}

GstElement *GstSinkWidget::createElement()
{
    Q_ASSERT(!m_element);
    if (m_element)
        return GST_ELEMENT(m_element);

    m_element = GST_APP_SINK(gst_element_factory_make("appsink", "sinkwidget"));
    if (!m_element)
    {
        qWarning() << "GstSinkWidget: Creating appsink element failed";
        return NULL;
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

    return GST_ELEMENT(m_element);
}

void GstSinkWidget::destroyElement()
{
    if (m_element)
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
    }

    m_frameLock.lock();
    if (m_framePtr)
    {
        /* Corresponding to the ref done in updateFrame prior to setting the value */
        gst_buffer_unref(m_framePtr);
        m_framePtr = 0;
    }
    m_frameLock.unlock();

    m_frameWidth = -1;
    m_frameHeight = -1;
    clearOverlayMessage();
}

QImage GstSinkWidget::currentFrame()
{
    if (m_frameWidth < 0 || m_frameHeight < 0)
        return QImage();

    QMutexLocker locker(&m_frameLock);
    GstBuffer *buffer = m_framePtr;
    if (buffer)
        gst_buffer_ref(buffer);

    if (!buffer)
        return QImage();

    QImage result = QImage(GST_BUFFER_DATA(buffer), m_frameWidth, m_frameHeight, QImage::Format_RGB32).copy();
    gst_buffer_unref(buffer);

    return result;
}

bool GstSinkWidget::eventFilter(QObject *obj, QEvent *ev)
{
    Q_ASSERT(obj == m_viewport);

    if (ev->type() != QEvent::Paint)
        return QFrame::eventFilter(obj, ev);

    QPainter p(m_viewport);
    if (p.device() && p.device()->paintEngine() && p.device()->paintEngine()->type() == QPaintEngine::OpenGL2)
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
        return true;
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

    return true;
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

    QMetaObject::invokeMethod(m_viewport, "update", Qt::QueuedConnection);
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
