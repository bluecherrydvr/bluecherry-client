#ifndef LIVESTREAMITEM_H
#define LIVESTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"
#include "core/LiveStream.h"

class LiveStreamItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<LiveStream> stream READ stream WRITE setStream NOTIFY streamChanged)
    Q_PROPERTY(QSizeF frameSize READ frameSize NOTIFY frameSizeChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(int fps READ fps CONSTANT)

public:
    explicit LiveStreamItem(QDeclarativeItem *parent = 0);
    virtual ~LiveStreamItem();

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    QSharedPointer<LiveStream> stream() const { return m_stream; }
    void setStream(const QSharedPointer<LiveStream> &stream);
    void clear();

    QSizeF frameSize() const { return m_stream ? m_stream->streamSize() : QSize(0, 0); }

    bool isPaused() const;
    bool isConnected() const;
    int fps() const;

public slots:
    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }

signals:
    void streamChanged(const QSharedPointer<LiveStream> &stream);
    void frameSizeChanged(const QSizeF &frameSize);
    void errorTextChanged(const QString &errorText);
    void pausedChanged(bool paused);
    void connectedChanged(bool connected);
    void fpsChanged();

private slots:
    void updateFrame()
    {
        update();
    }

    void updateFrameSize();
    void streamStateChanged(int state);
    void updateSettings();

private:
    QSharedPointer<LiveStream> m_stream;
    bool m_useAdvancedGL;
    unsigned m_texId;
    const uchar *m_texDataPtr;
};

#endif // LIVESTREAMITEM_H
