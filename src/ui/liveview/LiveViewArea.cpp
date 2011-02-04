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
