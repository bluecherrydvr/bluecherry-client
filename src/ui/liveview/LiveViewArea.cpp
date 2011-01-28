#include "LiveViewArea.h"
#include "MJpegFeedItem.h"
#include <QGLWidget>

LiveViewArea::LiveViewArea(QWidget *parent)
    : QDeclarativeView(parent)
{
    qmlRegisterType<MJpegFeedItem>("Bluecherry", 1, 0, "MJpegFeed");

    setViewport(new QGLWidget);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl(QLatin1String("qrc:qml/liveview/LiveView.qml")));
}
