#include "LiveFeedItem.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>

LiveFeedItem::LiveFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
}

void LiveFeedItem::setCamera(const DVRCamera &camera)
{
    if (camera == m_camera)
        return;

    if (m_camera)
        static_cast<QObject*>(m_camera)->disconnect(this);

    m_camera = camera;
    connect(m_camera, SIGNAL(dataUpdated()), SLOT(cameraDataUpdated()));

    emit cameraChanged(camera);
    cameraDataUpdated();
}

void LiveFeedItem::cameraDataUpdated()
{
    emit cameraNameChanged(cameraName());
}

void LiveFeedItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

    QMenu menu(event->widget());
    menu.addAction(tr("Test"));
    menu.exec(event->screenPos());
}
