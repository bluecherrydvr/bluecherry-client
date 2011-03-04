#ifndef MJPEGSTREAMITEM_H
#define MJPEGSTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"

class MJpegFeedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<MJpegStream> stream READ stream WRITE setStream NOTIFY streamChanged)
    Q_PROPERTY(QSizeF frameSize READ frameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

public:
    explicit MJpegFeedItem(QDeclarativeItem *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    QSharedPointer<MJpegStream> stream() const { return m_stream; }
    void setStream(const QSharedPointer<MJpegStream> &stream);
    void clear();

    QSizeF frameSize() const { return m_stream ? m_stream->streamSize() : QSize(0, 0); }

    bool isPaused() const;

public slots:
    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }

signals:
    void streamChanged(const QSharedPointer<MJpegStream> &stream);
    void frameSizeChanged(const QSizeF &frameSize);
    void errorTextChanged(const QString &errorText);
    void pausedChanged(bool paused);

private slots:
    void updateFrame()
    {
        update();
    }

    void updateFrameSize()
    {
        emit frameSizeChanged(frameSize());
    }

    void streamStateChanged(int state);

private:
    QSharedPointer<MJpegStream> m_stream;
};

#endif // MJPEGSTREAMITEM_H
