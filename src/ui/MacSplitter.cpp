#include "MacSplitter.h"
#include <QPainter>
#include <QLinearGradient>
#include <QApplication>
#include <QMouseEvent>

MacSplitterHandle::MacSplitterHandle(Qt::Orientation o, QSplitter *parent)
    : QSplitterHandle(o, parent), isMouseGrabbed(false)
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

#ifdef Q_OS_WIN
    QColor baseColor(171, 175, 183);
#else
    QColor baseColor(182, 182, 182);
#endif

    if (orientation() == Qt::Horizontal)
    {
#ifdef Q_OS_MAC
        QColor topColor(105, 105, 105);
        QColor bottomColor(111, 111, 111);

        QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height()));
        linearGrad.setColorAt(0, topColor);
        linearGrad.setColorAt(1, bottomColor);
        painter.fillRect(QRect(0, 0, 1, height()), QBrush(linearGrad));
#else
        painter.setPen(baseColor);
        painter.drawLine(0, 0, 0, height());
#endif
    }
    else
    {
        painter.setPen(baseColor);
        painter.drawLine(0, 0, width()-1, 0);
    }
}

void MacSplitterHandle::enterEvent(QEvent *e)
{
    if (isMouseGrabbed)
        return;

    Qt::CursorShape cursor = (orientation() == Qt::Horizontal) ? Qt::SplitHCursor : Qt::SplitVCursor;
    grabMouse(cursor);
    qApp->setOverrideCursor(cursor);
    isMouseGrabbed = true;
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
        isMouseGrabbed = false;
    }
}

void MacSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
    QSplitterHandle::mouseReleaseEvent(e);
    releaseMouse();
    qApp->restoreOverrideCursor();
    isMouseGrabbed = false;
}
