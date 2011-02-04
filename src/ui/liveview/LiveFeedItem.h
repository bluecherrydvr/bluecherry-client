#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif // LIVEFEEDITEM_H
