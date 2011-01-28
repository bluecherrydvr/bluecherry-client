#ifndef MJPEGSTREAMITEM_H
#define MJPEGSTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"

class MJpegFeedItem : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit MJpegFeedItem(QDeclarativeItem *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    void setStream(const QSharedPointer<MJpegStream> &stream);

private slots:
    void updateFrame()
    {
        update();
    }

private:
    QSharedPointer<MJpegStream> m_stream;

    static QSharedPointer<MJpegStream> hackStream;
};

#endif // MJPEGSTREAMITEM_H
