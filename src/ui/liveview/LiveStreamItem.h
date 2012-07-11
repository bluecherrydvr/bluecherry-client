#ifndef LIVESTREAMITEM_H
#define LIVESTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/MJpegStream.h"
#include "core/LiveStream.h"

class QGLContext;

class LiveStreamItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<LiveStream> stream READ stream WRITE setStream NOTIFY streamChanged)
    Q_PROPERTY(QSizeF frameSize READ frameSize NOTIFY frameSizeChanged)

public:
    explicit LiveStreamItem(QDeclarativeItem *parent = 0);
    virtual ~LiveStreamItem();

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    QSharedPointer<LiveStream> stream() const { return m_stream; }
    void setStream(const QSharedPointer<LiveStream> &stream);
    void clear();

    QSizeF frameSize() const { return m_stream ? m_stream->streamSize() : QSize(0, 0); }

signals:
    void streamChanged(const QSharedPointer<LiveStream> &stream);
    void frameSizeChanged(const QSizeF &frameSize);

private slots:
    void updateFrame()
    {
        update();
    }

    void updateFrameSize();
    void updateSettings();

private:
    QSharedPointer<LiveStream> m_stream;
    bool m_useAdvancedGL;
    unsigned m_texId;
    const QGLContext *m_texLastContext;
    bool m_texInvalidate;
    const uchar *m_texDataPtr;

    void clearTexture();
};

#endif // LIVESTREAMITEM_H
