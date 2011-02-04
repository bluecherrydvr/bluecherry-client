#include "MJpegFeedItem.h"
#include "core/MJpegStream.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

QSharedPointer<MJpegStream> MJpegFeedItem::hackStream;

MJpegFeedItem::MJpegFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);

    if (!hackStream)
        hackStream = QSharedPointer<MJpegStream>(new MJpegStream(QUrl(QLatin1String("https://Admin:bluecherry@192.168.0.3:7001/media/mjpeg.php?id=1&multipart"))));

    setStream(hackStream);
}

void MJpegFeedItem::setStream(const QSharedPointer<MJpegStream> &stream)
{
    if (stream == m_stream)
        return;

    if (m_stream)
        m_stream->disconnect(this);

    m_stream = stream;
    connect(m_stream.data(), SIGNAL(updateFrame(QPixmap,QVector<QImage>)), SLOT(updateFrame()));

    m_stream->start();
    updateFrame();
}

void MJpegFeedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    Q_UNUSED(widget);

    if (!m_stream)
    {
        p->fillRect(opt->rect, Qt::red);
        return;
    }

    if (!m_stream->currentFrame().isNull())
    {
        p->save();
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setCompositionMode(QPainter::CompositionMode_Source);
        p->drawPixmap(opt->rect, m_stream->currentFrame());
        p->restore();
    }
    else
        p->fillRect(opt->rect, Qt::blue);
}
