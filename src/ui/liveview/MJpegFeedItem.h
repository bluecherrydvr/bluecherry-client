#ifndef MJPEGSTREAMITEM_H
#define MJPEGSTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"
#include "core/LiveStream.h"

class MJpegFeedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<LiveStream> stream READ stream WRITE setStream NOTIFY streamChanged)
    Q_PROPERTY(QSizeF frameSize READ frameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval RESET clearInterval NOTIFY intervalChanged)
    Q_PROPERTY(int fps READ fps CONSTANT)

public:
    explicit MJpegFeedItem(QDeclarativeItem *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    QSharedPointer<LiveStream> stream() const { return m_stream; }
    void setStream(const QSharedPointer<LiveStream> &stream);
    void clear();

    QSizeF frameSize() const { return m_stream ? m_stream->streamSize() : QSize(0, 0); }

    bool isPaused() const;
    bool isConnected() const;
    int interval() const;
    int fps() const;

public slots:
    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }
    void setInterval(int interval);
    void clearInterval();

signals:
    void streamChanged(const QSharedPointer<LiveStream> &stream);
    void frameSizeChanged(const QSizeF &frameSize);
    void errorTextChanged(const QString &errorText);
    void pausedChanged(bool paused);
    void connectedChanged(bool connected);
    void intervalChanged(int interval);
    void fpsChanged();

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
    QSharedPointer<LiveStream> m_stream;
};

#endif // MJPEGSTREAMITEM_H
