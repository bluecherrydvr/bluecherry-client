#include "MJpegFeedItem.h"
#include "core/MJpegStream.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

MJpegFeedItem::MJpegFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void MJpegFeedItem::setStream(const QSharedPointer<MJpegStream> &stream)
{
    if (stream == m_stream)
        return;

    if (m_stream)
        m_stream->disconnect(this);

    m_stream = stream;

    if (m_stream)
    {
        connect(m_stream.data(), SIGNAL(updateFrame(QPixmap,QVector<QImage>)), SLOT(updateFrame()));
        m_stream->start();
    }

    updateFrame();
}

void MJpegFeedItem::clear()
{
    setStream(QSharedPointer<MJpegStream>());
}

void MJpegFeedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    Q_UNUSED(widget);

    if (!m_stream)
        return;

    QPixmap frame = m_stream->currentFrame();

    if (!frame.isNull())
    {
        p->save();
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setCompositionMode(QPainter::CompositionMode_Source);

        QRect r = opt->rect;
        QSize renderSz = frame.size();

        /* Force aspect ratio */
        renderSz.scale(r.size(), Qt::KeepAspectRatio);

        /* Center the image within the area */
        r.adjust((r.width() - renderSz.width()) / 2, (r.height() - renderSz.height()) / 2, 0, 0);
        r.setSize(renderSz);

        p->drawPixmap(r, frame);
        p->restore();
    }
    else
        p->fillRect(opt->rect, Qt::blue);
}
