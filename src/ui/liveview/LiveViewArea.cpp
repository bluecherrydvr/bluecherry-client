#include "LiveViewArea.h"
#include "LiveViewLayout.h"
#include "LiveFeedItem.h"
#include "MJpegFeedItem.h"
#include "core/CameraPtzControl.h"
#include "core/BluecherryApp.h"
#include <QGLWidget>
#include <QDeclarativeContext>
#include <QSettings>

LiveViewArea::LiveViewArea(QWidget *parent)
    : QDeclarativeView(parent)
{
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(settingsChanged()));

    qmlRegisterType<LiveViewLayout>("Bluecherry", 1, 0, "LiveViewLayout");
    qmlRegisterType<MJpegFeedItem>("Bluecherry", 1, 0, "MJpegFeed");
    qmlRegisterType<LiveFeedItem>("Bluecherry", 1, 0, "LiveFeedBase");
    qmlRegisterUncreatableType<CameraPtzControl>("Bluecherry", 1, 0, "CameraPtzControl", QLatin1String(""));

    QSettings settings;
    if (!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), false).toBool())
        setViewport(new QGLWidget);
    else
        qDebug("Hardware-accelerated live view is DISABLED");

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl(QLatin1String("qrc:qml/liveview/LiveView.qml")));

    m_layout = rootObject()->findChild<LiveViewLayout*>(QLatin1String("viewLayout"));
    Q_ASSERT(m_layout);
}

bool LiveViewArea::isHardwareAccelerated() const
{
    return viewport()->inherits("QGLWidget");
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
