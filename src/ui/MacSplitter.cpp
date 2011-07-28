#include "MacSplitter.h"
#include <QPainter>
#include <QLinearGradient>
#include <QApplication>
#include <QMouseEvent>

MacSplitterHandle::MacSplitterHandle(Qt::Orientation o, QSplitter *parent)
    : QSplitterHandle(o, parent)
{
    setMouseTracking(true);
}

QSize MacSplitterHandle::sizeHint() const
{
    QSize parent = QSplitterHandle::sizeHint();
    if (orientation() == Qt::Vertical)
        return QSize(parent.width(), 1);
    else
        return QSize(1, parent.height());
}

void MacSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QColor topColor(167, 167, 167);
    QColor bottomColor(131, 131, 131);

    if (orientation() == Qt::Horizontal)
    {
        QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height()));
        linearGrad.setColorAt(0, topColor);
        linearGrad.setColorAt(1, bottomColor);
        painter.fillRect(QRect(0, 0, 1, height()), QBrush(linearGrad));
    }
    else
    {
        painter.setPen(QColor(182, 182, 182));
        painter.drawLine(0, 0, width()-1, 0);
    }
}

void MacSplitterHandle::enterEvent(QEvent *e)
{
    Qt::CursorShape cursor = (orientation() == Qt::Horizontal) ? Qt::SplitHCursor : Qt::SplitVCursor;
    grabMouse(cursor);
    qApp->setOverrideCursor(cursor);
}

void MacSplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
    QSplitterHandle::mouseMoveEvent(e);

    QPoint p = e->pos();
    if (!(e->buttons() & Qt::LeftButton) &&
        (p.x() <= -5 || p.y() <= -5 || (p.x()-width() > 5) || (p.y()-height() > 5)))
    {
        releaseMouse();
        qApp->restoreOverrideCursor();
    }
}

void MacSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    QSplitterHandle::mouseReleaseEvent(e);
    releaseMouse();
    qApp->restoreOverrideCursor();
}
