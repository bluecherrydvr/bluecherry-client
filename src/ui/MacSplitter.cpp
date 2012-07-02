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

MacSplitterHandle::~MacSplitterHandle()
{
    if (isMouseGrabbed)
    {
        qApp->restoreOverrideCursor();
        qApp->removeEventFilter(this);
    }
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
    Q_UNUSED(e);

    if (isMouseGrabbed)
        return;

    Qt::CursorShape cursor = (orientation() == Qt::Horizontal) ? Qt::SplitHCursor : Qt::SplitVCursor;
    qApp->setOverrideCursor(cursor);
    qApp->installEventFilter(this);
    isMouseGrabbed = true;
}

bool MacSplitterHandle::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj);

    QMouseEvent *me;
    QPoint p;

    switch (e->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        return true;

    case QEvent::MouseMove:
        me = static_cast<QMouseEvent*>(e);
        p = mapFromGlobal(me->globalPos());
        if (me->buttons() & Qt::LeftButton)
        {
            QPoint sp = parentWidget()->mapFromGlobal(me->globalPos());
            if (orientation() == Qt::Horizontal)
                moveSplitter(sp.x());
            else
                moveSplitter(sp.y());
        }
        else if (p.x() <= -7 || p.y() <= -7 || (p.x()-width() > 7) || (p.y()-height() > 7))
        {
            qApp->restoreOverrideCursor();
            qApp->removeEventFilter(this);
            isMouseGrabbed = false;
            break;
        }
        return true;

    case QEvent::MouseButtonRelease:
        qApp->restoreOverrideCursor();
        qApp->removeEventFilter(this);
        isMouseGrabbed = false;
        if (e->type() == QEvent::Leave)
            break;
        return true;
    }

    return false;
}
