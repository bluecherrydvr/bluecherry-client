#include "LiveViewArea.h"
#include "LiveViewLayout.h"
#include "LiveFeedItem.h"
#include "MJpegFeedItem.h"
#include <QGLWidget>
#include <QDeclarativeContext>

LiveViewArea::LiveViewArea(QWidget *parent)
    : QDeclarativeView(parent)
{
    qmlRegisterType<LiveViewLayout>("Bluecherry", 1, 0, "LiveViewLayout");
    qmlRegisterType<MJpegFeedItem>("Bluecherry", 1, 0, "MJpegFeed");
    qmlRegisterType<LiveFeedItem>("Bluecherry", 1, 0, "LiveFeedBase");

    setViewport(new QGLWidget);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl(QLatin1String("qrc:qml/liveview/LiveView.qml")));

    m_layout = rootObject()->findChild<LiveViewLayout*>(QLatin1String("viewLayout"));
    Q_ASSERT(m_layout);
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
