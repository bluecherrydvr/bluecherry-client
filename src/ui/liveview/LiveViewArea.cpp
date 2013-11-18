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

#include "LiveViewArea.h"
#include "LiveViewLayout.h"
#include "LiveFeedItem.h"
#include "LiveStreamItem.h"
#include "LiveViewGradients.h"
#include "core/CameraPtzControl.h"
#include "core/BluecherryApp.h"
#include "server/DVRServerRepository.h"
#include <QGLWidget>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QSettings>
#include <QShowEvent>
#include <QApplication>
#include <QTimer>

LiveViewArea::LiveViewArea(DVRServerRepository *serverRepository, QWidget *parent)
    : QDeclarativeView(parent)
{
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(settingsChanged()));

    qmlRegisterType<LiveViewLayout>("Bluecherry", 1, 0, "LiveViewLayout");
    qmlRegisterType<LiveStreamItem>("Bluecherry", 1, 0, "RtspStreamDisplay");
    qmlRegisterType<LiveFeedItem>("Bluecherry", 1, 0, "LiveFeedBase");
    qmlRegisterUncreatableType<CameraPtzControl>("Bluecherry", 1, 0, "CameraPtzControl", QLatin1String(""));
    qmlRegisterUncreatableType<RtspStream>("Bluecherry", 1, 0, "RtspStream", QLatin1String(""));

    rootContext()->setContextProperty(QLatin1String("mainServerRepository"), QVariant::fromValue(serverRepository));

    QSettings settings;
    if (!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), true).toBool())
        setViewport(new QGLWidget);
    else
        qDebug("Hardware-accelerated live view is DISABLED");

    engine()->addImageProvider(QLatin1String("liveviewgradients"), new LiveViewGradients);

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl(QLatin1String("qrc:qml/liveview/LiveView.qml")));

    m_layout = rootObject()->findChild<LiveViewLayout*>(QLatin1String("viewLayout"));
    Q_ASSERT(m_layout);
}

LiveViewArea::~LiveViewArea()
{
    /* For GL viewports, make that context current prior to clearing the scene
     * to allow items a chance to clean up GL resources properly, though this is
     * non-critical because the context destruction would implicitly free them.
     * See LiveStreamItem. */
    QGLWidget *gl = qobject_cast<QGLWidget*>(viewport());
    if (gl)
        gl->makeCurrent();
    scene()->clear();
}

bool LiveViewArea::isHardwareAccelerated() const
{
    return viewport()->inherits("QGLWidget");
}

void LiveViewArea::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && isHardwareAccelerated())
    {
        /* Hack around a bug that causes the surface to never paint anything on some systems
         * running OpenGL 1.x (specifically witnessed on 1.4), by creating a new viewport shortly
         * after the actual window has been shown. */
        static_cast<QGLWidget*>(viewport())->makeCurrent();
        Q_ASSERT(QGLContext::currentContext());
        if (QGLFormat::openGLVersionFlags() < QGLFormat::OpenGL_Version_2_0)
        {
            qDebug("Using OpenGL 1.x late viewport hack");
            QTimer::singleShot(0, this, SLOT(setViewportHack()));
        }
    }

    QDeclarativeView::showEvent(event);
}

void LiveViewArea::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous() && isHardwareAccelerated())
    {
        static_cast<QGLWidget*>(viewport())->makeCurrent();
        Q_ASSERT(QGLContext::currentContext());
        if (QGLFormat::openGLVersionFlags() < QGLFormat::OpenGL_Version_2_0)
        {
            /* Run the viewport hack on any other LiveViewAreas that exist */
            QWidgetList widgets = qApp->allWidgets();
            foreach (QWidget *w, widgets)
            {
                if (qobject_cast<LiveViewArea*>(w) && w != this)
                    QTimer::singleShot(0, w, SLOT(setViewportHack()));
            }
        }
    }
}

void LiveViewArea::setViewportHack()
{
    setViewport(new QGLWidget);
}

void LiveViewArea::addCamera(DVRCamera *camera)
{
    QDeclarativeItem *item = m_layout->addItemAuto();
    if (!item || !camera)
        return;

    bool setCameraProperty = item->setProperty("camera", QVariant::fromValue(camera));
    Q_ASSERT(setCameraProperty);
    Q_UNUSED(setCameraProperty);
}

QSize LiveViewArea::sizeHint() const
{
    if (!m_sizeHint.isValid())
        m_sizeHint = layout()->idealSize();

    return m_sizeHint;
}

void LiveViewArea::settingsChanged()
{
    QSettings settings;
    bool hwaccel = !settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), true).toBool();
    if (hwaccel != isHardwareAccelerated())
    {
        qDebug("%s hardware acceleration for live view", hwaccel ? "Enabled" : "Disabled");
        if (hwaccel)
            setViewport(new QGLWidget);
        else
            setViewport(new QWidget);
    }
}
