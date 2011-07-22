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
        connect(m_stream.data(), SIGNAL(updateFrame(MJpegFrame,QVector<QImage>)), SLOT(updateFrame()));
        connect(m_stream.data(), SIGNAL(streamSizeChanged(QSize)), SLOT(updateFrameSize()));
        connect(m_stream.data(), SIGNAL(stateChanged(int)), SLOT(streamStateChanged(int)));
        connect(m_stream.data(), SIGNAL(pausedChanged(bool)), SIGNAL(pausedChanged(bool)));
        m_stream->start();

        streamStateChanged(m_stream->state());
    }
    else
        emit errorTextChanged(tr("No<br>Video"));

    emit pausedChanged(isPaused());
    emit connectedChanged(isConnected());
    emit intervalChanged(interval());

    updateFrameSize();
    updateFrame();
}

void MJpegFeedItem::clear()
{
    setStream(QSharedPointer<MJpegStream>());
}

bool MJpegFeedItem::isPaused() const
{
    return m_stream ? m_stream->isPaused() : false;
}

void MJpegFeedItem::setPaused(bool paused)
{
    if (m_stream)
        m_stream->setPaused(paused);
}

int MJpegFeedItem::interval() const
{
    return m_stream ? m_stream->interval() : 1;
}

void MJpegFeedItem::setInterval(int interval)
{
    if (m_stream)
        m_stream->setInterval(interval);
}

void MJpegFeedItem::clearInterval()
{
    m_stream->clearInterval();
}

void MJpegFeedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    Q_UNUSED(widget);

    if (!m_stream)
        return;

    MJpegFrame frame = m_stream->currentFrame();

    if (!frame.isNull())
    {
        p->save();
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setCompositionMode(QPainter::CompositionMode_Source);

#ifdef MJPEGFRAME_IS_PIXMAP
        p->drawPixmap(opt->rect, frame);
#else
        p->drawImage(opt->rect, frame);
#endif

        p->restore();
    }
    else
        p->fillRect(opt->rect, Qt::black);
}

void MJpegFeedItem::streamStateChanged(int state)
{
    Q_ASSERT(m_stream);

    switch (state)
    {
    case MJpegStream::Error:
        emit errorTextChanged(tr("<span style='color:#ff0000;'>Error</span>"));
        //setToolTip(m_stream->errorMessage());
        break;
    case MJpegStream::StreamOffline:
        emit errorTextChanged(tr("Offline"));
        break;
    case MJpegStream::NotConnected:
        emit errorTextChanged(tr("Disconnected"));
        break;
    case MJpegStream::Connecting:
        emit errorTextChanged(tr("Connecting..."));
        break;
    case MJpegStream::Buffering:
        emit errorTextChanged(tr("Buffering..."));
        break;
    default:
        emit errorTextChanged(QString());
        break;
    }
}
