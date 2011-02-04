#include "LiveFeedItem.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>

LiveFeedItem::LiveFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
}

void LiveFeedItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

    QMenu menu(event->widget());
    menu.addAction(tr("Test"));
    menu.exec(event->screenPos());
}
