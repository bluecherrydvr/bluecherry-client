#include "LiveViewLayout.h"
#include "core/DVRCamera.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QTimerEvent>
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QCursor>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <math.h>

LiveViewLayout::LiveViewLayout(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_rows(0), m_columns(0), m_itemComponent(0)
{
    m_dragDrop.dragItem = 0;
    m_dragDrop.dropRow = -1, m_dragDrop.dropColumn = -1;
    setAcceptDrops(true);
}

QDeclarativeItem *LiveViewLayout::createNewItem()
{
    if (!m_itemComponent)
        return 0;

    QDeclarativeContext *context = QDeclarativeEngine::contextForObject(this);
    Q_ASSERT(context);

    QDeclarativeItem *element = qobject_cast<QDeclarativeItem*>(m_itemComponent->create(context));
    Q_ASSERT(element);
    if (!element)
        return 0;

    element->setParentItem(this);
    connect(LiveViewLayoutProps::get(element), SIGNAL(layoutNeeded()), SLOT(scheduleLayout()));

    return element;
}

void LiveViewLayout::setItem(QDeclarativeComponent *c)
{
    Q_ASSERT(!m_itemComponent || m_itemComponent == c);
    if (m_itemComponent || !c)
        return;

    if (c->isError())
    {
        qWarning() << "LiveViewLayout item errors:" << c->errors();
        return;
    }

    m_itemComponent = c;
}

void LiveViewLayout::scheduleLayout()
{
    if (!m_layoutTimer.isActive())
        m_layoutTimer.start(0, this);
}

void LiveViewLayout::timerEvent(QTimerEvent *event)
{
    /* doLayout stops the schedule timer */
    if (event->timerId() == m_layoutTimer.timerId())
        doLayout();
}

void LiveViewLayout::doLayout()
{
    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_items.isEmpty())
        return;

    QSizeF cellSz(floor(width() / m_columns), floor(height() / m_rows));

    qreal left = qRound(width()) % m_columns,
          x = left,
          y = qRound(height()) % m_rows;

    for (int r = 0, c = 0;;)
    {
        QDeclarativeItem *i = at(r, c);

        if (i)
        {
            LiveViewLayoutProps *ip = LiveViewLayoutProps::get(i);

            QSizeF size = ip->sizeHint(), padding = ip->sizePadding();
            if (size.isValid())
            {
                size.scale(cellSz - padding, ip->fixedAspectRatio() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
                size += padding;
            }
            else
                size = cellSz;

            i->setWidth(size.width());
            i->setHeight(size.height());
            i->setX(x);
            i->setY(y);
        }

        if (++c == m_columns)
        {
            if (++r == m_rows)
                break;
            c = 0;

            y += cellSz.height();
            x = left;
        }
        else
            x += cellSz.width();
    }

    m_layoutTimer.stop();
}

void LiveViewLayout::gridPos(const QPointF &pos, int *row, int *column)
{
    Q_ASSERT(row && column);

    qreal w = floor(width() / m_columns),
          h = floor(height() / m_rows);

    qreal left = qRound(width()) % m_columns,
          top = qRound(height()) % m_rows;

    if (pos.x() < 0 || pos.x() > width() || pos.y() < 0 || pos.y() > height())
    {
        *row = *column = -1;
    }
    else
    {
        *row = floor(qMax(0.0,pos.y()-top)/h);
        *column = floor(qMax(0.0,pos.x()-left)/w);
    }
}

void LiveViewLayout::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    scheduleLayout();
}

void LiveViewLayout::insertRow(int row)
{
    row = qBound(0, row, m_rows);
    m_rows++;

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
        m_items.insert(i, 0);

    scheduleLayout();
}

void LiveViewLayout::removeRow(int row)
{
    if (!m_rows)
        return;

    row = qBound(0, row, m_rows-1);

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
    {
        QDeclarativeItem *item = m_items[i];
        if (item)
            item->deleteLater();
    }

    QList<QDeclarativeItem*>::Iterator st = m_items.begin() + (row * m_columns);
    m_items.erase(st, st+m_columns);
    --m_rows;

    scheduleLayout();
}

void LiveViewLayout::insertColumn(int column)
{
    column = qBound(0, column, m_columns);

    for (int i = column, n = 0; n < m_rows; i += m_columns, ++n)
    {
        m_items.insert(i, 0);
        ++i;
    }

    m_columns++;
    scheduleLayout();
}

void LiveViewLayout::removeColumn(int column)
{
    if (!m_columns)
        return;
    column = qBound(0, column, m_columns-1);

    for (int i = column; i < m_items.size(); i += m_columns)
    {
        QDeclarativeItem *item = m_items[i];
        if (item)
            item->deleteLater();
        m_items.removeAt(i);
        --i;
    }

    --m_columns;
    scheduleLayout();
}

void LiveViewLayout::setGridSize(int rows, int columns)
{
    rows = qMax(0, rows);
    columns = qMax(0, columns);
    if (rows == m_rows && columns == m_columns)
        return;

    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_rows > rows)
    {
        int remove = m_rows - rows;

        /* If there are any empty rows, remove those first */
        for (int r = 0; r < m_rows; ++r)
        {
            bool empty = true;
            for (int c = 0; c < m_columns; ++c)
            {
                if (at(r, c))
                {
                    empty = false;
                    break;
                }
            }

            if (empty)
            {
                removeRow(r);
                if (!--remove)
                    break;
                --r;
            }
        }

        /* Otherwise, take rows from the bottom */
        for (int r = m_rows-1; remove && r >= 0; --r, --remove)
            removeRow(r);

        Q_ASSERT(!remove);
    }

    if (m_columns > columns)
    {
        int remove = m_columns - columns;

        for (int c = 0; c < m_columns; ++c)
        {
            bool empty = true;
            for (int r = 0; r < m_rows; ++r)
            {
                if (at(r, c))
                {
                    empty = false;
                    break;
                }
            }

            if (empty)
            {
                removeColumn(c);
                if (!--remove)
                    break;
                --c;
            }
        }

        for (int c = m_columns-1; remove && c >= 0; --c, --remove)
            removeColumn(c);

        Q_ASSERT(!remove);
    }

    while (m_columns < columns)
        insertColumn(m_columns);

    while (m_rows < rows)
        insertRow(m_rows);

    Q_ASSERT(m_items.size() == (m_rows*m_columns));
}

void LiveViewLayout::set(int row, int col, QDeclarativeItem *item)
{
    if (row >= m_rows || col >= m_columns || (item == at(row, col)))
        return;

    QDeclarativeItem *&ip = m_items[(row * m_columns) + col];
    if (ip)
        ip->deleteLater();
    ip = item;
    scheduleLayout();
}

void LiveViewLayout::removeItem(QDeclarativeItem *item)
{
    int index = m_items.indexOf(item);
    if (index < 0 || !item)
        return;

    m_items[index] = 0;
    item->deleteLater();
}

QDeclarativeItem *LiveViewLayout::addItemAuto()
{
    /* Put the item in the first empty space, top-left to bottom-right */
    int index = m_items.indexOf(0);

    if (index < 0)
    {
        /* Add a row or a column to make space, whichever has fewer */
        if (columns() < rows())
            setGridSize(qMax(1, rows()), qMax(1, columns())+1);
        else
            setGridSize(qMax(1, rows()+1), qMax(1, columns()));

        index = m_items.indexOf(0);
        Q_ASSERT(index >= 0);
    }

    m_items[index] = createNewItem();
    doLayout();

    return m_items[index];
}

QDeclarativeItem *LiveViewLayout::addItem(int row, int column)
{
    if (row < 0 || column < 0)
        return 0;

    setGridSize(qMax(row, rows()), qMax(column, columns()));

    QDeclarativeItem *re = m_items[(row * columns()) + column] = createNewItem();
    doLayout();

    return re;
}

QDeclarativeItem *LiveViewLayout::takeItem(QDeclarativeItem *item)
{
    int i = m_items.indexOf(item);
    if (i < 0)
        return 0;

    m_items[i] = 0;
    return item;
}

QDeclarativeItem *LiveViewLayout::dropTarget() const
{
    if (m_dragDrop.dropRow >= 0 && m_dragDrop.dropRow < rows() &&
        m_dragDrop.dropColumn >= 0 && m_dragDrop.dropColumn < columns())
        return at(m_dragDrop.dropRow, m_dragDrop.dropColumn);
    return 0;
}

void LiveViewLayout::startDrag(QDeclarativeItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(!m_dragDrop.dragItem);

    /* Remove the item from the layout while it's being dragged */
    takeItem(item);

    m_dragDrop.dragItem = item;
    LiveViewLayoutProps::get(item)->setIsDragItem(true);

    emit dragItemChanged(item);
    updateDrag();
}

void LiveViewLayout::updateDrag()
{
    Q_ASSERT(m_dragDrop.dragItem);
    if (!m_dragDrop.dragItem)
        return;

    QPointF pos = cursorItemPos();

    int row, column;
    gridPos(pos, &row, &column);

    if (row != m_dragDrop.dropRow || column != m_dragDrop.dropColumn)
    {
        QDeclarativeItem *drop = dropTarget();
        if (drop)
            LiveViewLayoutProps::get(drop)->setIsDropTarget(false);

        m_dragDrop.dropRow    = row;
        m_dragDrop.dropColumn = column;

        drop = dropTarget();
        if (drop)
            LiveViewLayoutProps::get(drop)->setIsDropTarget(true);

        emit dropTargetChanged(drop);
    }
}

void LiveViewLayout::endDrag(bool dropped)
{
    if (!m_dragDrop.dragItem)
        return;

    QDeclarativeItem *item = m_dragDrop.dragItem;

    QDeclarativeItem *drop = dropTarget();
    m_dragDrop.dragItem = 0;
    m_dragDrop.dropRow = m_dragDrop.dropColumn = -1;

    if (drop)
        LiveViewLayoutProps::get(drop)->setIsDropTarget(false);
    LiveViewLayoutProps::get(item)->setIsDragItem(false);

    emit dragItemChanged(0);
    emit dropTargetChanged(0);

    if (!dropped)
        item->deleteLater();
}

bool LiveViewLayout::drop()
{
    if (!m_dragDrop.dragItem)
        return false;

    if (m_dragDrop.dropRow < 0 || m_dragDrop.dropColumn < 0)
    {
        endDrag(false);
        return false;
    }

    set(m_dragDrop.dropRow, m_dragDrop.dropColumn, m_dragDrop.dragItem);

    endDrag(true);
    return true;
}

QPointF LiveViewLayout::cursorItemPos() const
{
    QWidget *w = QApplication::activeWindow();
    if (w)
        w = w->childAt(w->mapFromGlobal(QCursor::pos()));

    QGraphicsView *view = qobject_cast<QGraphicsView*>(w ? w->parentWidget() : 0);
    if (view)
        return mapFromScene(view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos())));

    return QPointF(-1, -1);
}

void LiveViewLayout::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!event->mimeData()->hasFormat(QLatin1String("application/x-bluecherry-dvrcamera")))
        return;

    QList<DVRCamera> cameras = DVRCamera::fromMimeData(event->mimeData());
    if (cameras.isEmpty())
        return;

    Q_ASSERT(!m_dragDrop.dragItem);

    QDeclarativeItem *item = createNewItem();
    Q_ASSERT(item);
    if (!item)
        return;

    item->setX(event->pos().x());
    item->setY(event->pos().y());
    startDrag(item);

    bool dragItemCameraProperty = item->setProperty("camera", QVariant::fromValue(cameras[0]));
    Q_ASSERT(dragItemCameraProperty);
    Q_UNUSED(dragItemCameraProperty);

    event->acceptProposedAction();
}

void LiveViewLayout::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!m_dragDrop.dragItem)
        return;

    m_dragDrop.dragItem->setX(event->pos().x());
    m_dragDrop.dragItem->setY(event->pos().y());
    updateDrag();
    event->acceptProposedAction();
}

void LiveViewLayout::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    endDrag(false);
}

void LiveViewLayout::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (drop())
        event->acceptProposedAction();
}

void LiveViewLayoutProps::setFixedAspectRatio(bool v)
{
    if (m_fixedAspectRatio == v)
        return;

    m_fixedAspectRatio = v;
    emit layoutNeeded();
}

void LiveViewLayoutProps::setSizeHint(const QSizeF &v)
{
    if (m_sizeHint == v)
        return;

    m_sizeHint = v;
    emit sizeHintChanged(v);
    emit layoutNeeded();
}

void LiveViewLayoutProps::setSizePadding(const QSizeF &v)
{
    if (m_sizePadding == v)
        return;

    m_sizePadding = v;
    emit layoutNeeded();
}
