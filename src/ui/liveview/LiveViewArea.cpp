#include "LiveViewArea.h"
#include "LiveViewLayout.h"
#include "LiveFeedItem.h"
#include "LiveStreamItem.h"
#include "LiveViewGradients.h"
#include "core/CameraPtzControl.h"
#include "core/BluecherryApp.h"
#include <QGLWidget>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QSettings>
#include <QShowEvent>
#include <QApplication>

LiveViewArea::LiveViewArea(QWidget *parent)
    : QDeclarativeView(parent)
{
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(settingsChanged()));

    qmlRegisterType<LiveViewLayout>("Bluecherry", 1, 0, "LiveViewLayout");
    qmlRegisterType<LiveStreamItem>("Bluecherry", 1, 0, "LiveStreamDisplay");
    qmlRegisterType<LiveFeedItem>("Bluecherry", 1, 0, "LiveFeedBase");
    qmlRegisterUncreatableType<CameraPtzControl>("Bluecherry", 1, 0, "CameraPtzControl", QLatin1String(""));
    qmlRegisterUncreatableType<LiveStream>("Bluecherry", 1, 0, "LiveStream", QLatin1String(""));

    QSettings settings;
    if (!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), false).toBool())
        setViewport(new QGLWidget);
    else
        qDebug("Hardware-accelerated live view is DISABLED");

    engine()->addImageProvider(QLatin1String("liveviewgradients"), new LiveViewGradients);

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl(QLatin1String("qrc:qml/liveview/LiveView.qml")));

    m_layout = rootObject()->findChild<LiveViewLayout*>(QLatin1String("viewLayout"));
    Q_ASSERT(m_layout);
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

void LiveViewArea::addCamera(const DVRCamera &camera)
{
    QDeclarativeItem *item = m_layout->addItemAuto();
    Q_ASSERT(item);

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
    bool hwaccel = !settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), false).toBool();
    if (hwaccel != isHardwareAccelerated())
    {
        qDebug("%s hardware acceleration for live view", hwaccel ? "Enabled" : "Disabled");
        if (hwaccel)
            setViewport(new QGLWidget);
        else
            setViewport(new QWidget);
    }
}
