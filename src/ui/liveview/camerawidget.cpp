#include "camerawidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QImage>
#include <stdlib.h>

CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

CameraWidget::~CameraWidget()
{
    m_stream.data()->unref();
}

void CameraWidget::paintEvent(QPaintEvent *event)
{
    if (!m_stream)
        return;

    QPainter p(this);

    QImage frame = m_stream.data()->currentFrame();

    if (frame.isNull())
    {
        p.fillRect(event->rect(), Qt::black);
        return;
    }
    m_framesize = frame.size();
    //p->save();
    //p->setRenderHint(QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(event->rect(), frame);
    //p->restore();
}

int CameraWidget::heightForWidth(int w) const
{
    if (m_framesize.isEmpty())
        return w * 3 / 4;
    else
        return w * (float)m_framesize.height() / (float)m_framesize.width();
}

QSize CameraWidget::sizeHint() const
{
    if (m_framesize.isEmpty())
        return QSize(320, 240);
    else
        return m_framesize;
}

void CameraWidget::setStream(QSharedPointer<LiveStream> stream)
{
    if (stream == m_stream)
        return;

    if (m_stream)
    {
        m_stream.data()->disconnect(this);
        m_stream.data()->unref();
    }

    m_stream = stream;

    if (m_stream.data())
    {
        connect(m_stream.data(), SIGNAL(updated()), SLOT(updateFrame()));
        //connect(m_stream.data(), SIGNAL(streamSizeChanged(QSize)), SLOT(updateFrameSize()));
        m_stream.data()->start();
        m_stream.data()->ref();
    }

    //updateFrameSize();
    updateFrame();
}
