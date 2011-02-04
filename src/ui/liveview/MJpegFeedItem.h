#ifndef MJPEGSTREAMITEM_H
#define MJPEGSTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"

class MJpegFeedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<MJpegStream> stream READ stream WRITE setStream NOTIFY streamChanged)

public:
    explicit MJpegFeedItem(QDeclarativeItem *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    QSharedPointer<MJpegStream> stream() const { return m_stream; }
    void setStream(const QSharedPointer<MJpegStream> &stream);

signals:
    void streamChanged(const QSharedPointer<MJpegStream> &stream);

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
