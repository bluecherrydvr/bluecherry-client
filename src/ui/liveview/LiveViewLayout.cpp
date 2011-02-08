#include "LiveViewLayout.h"
#include "core/DVRCamera.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QTimerEvent>
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

LiveViewLayout::LiveViewLayout(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_rows(0), m_columns(0), m_itemComponent(0)
{
    m_dragDrop.dragItem = 0;
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

    for (int i = 0; i < m_items.size(); ++i)
    {
        QDeclarativeItem *&item = m_items[i];
        if (!item)
            item = createNewItem();
    }
}

void LiveViewLayout::scheduleLayout()
{
    if (!m_layoutTimer.isActive())
        m_layoutTimer.start(0, this);
}

void LiveViewLayout::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_layoutTimer.timerId())
    {
        m_layoutTimer.stop();
        doLayout();
    }
}

void LiveViewLayout::doLayout()
{
    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_items.isEmpty())
        return;

    qreal w = floor(width() / m_columns),
          h = floor(height() / m_rows);

    qreal left = qRound(width()) % m_columns,
          x = left,
          y = qRound(height()) % m_rows;

    for (int r = 0, c = 0;;)
    {
        QDeclarativeItem *i = at(r, c);

        if (i)
        {
            i->setWidth(w);
            i->setHeight(h);
            i->setX(x);
            i->setY(y);
        }

        if (++c == m_columns)
        {
            if (++r == m_rows)
                break;
            c = 0;

            y += h;
            x = left;
        }
        else
            x += w;
    }
}

void LiveViewLayout::gridPos(const QPointF &pos, int *row, int *column)
{
    Q_ASSERT(row && column);

    qreal w = floor(width() / m_columns),
          h = floor(height() / m_rows);

    qreal left = qRound(width()) % m_columns,
          top = qRound(height()) % m_rows;

    *row = floor(qMax(0.0,pos.y()-top)/h);
    *column = floor(qMax(0.0,pos.x()-left)/w);
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
        m_items.insert(i, createNewItem());

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
        m_items.insert(i, createNewItem());
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

void LiveViewLayout::updateDrag(QGraphicsSceneDragDropEvent *event)
{
    if (!m_dragDrop.dragItem)
        return;

    m_dragDrop.dragItem->setX(event->pos().x());
    m_dragDrop.dragItem->setY(event->pos().y());

    int row, column;
    gridPos(event->pos(), &row, &column);

    if (row != m_dragDrop.dropRow || column != m_dragDrop.dropColumn)
    {
        QDeclarativeItem *dropTarget = at(m_dragDrop.dropRow, m_dragDrop.dropColumn);
        if (dropTarget)
            LiveViewLayoutProps::get(dropTarget)->setIsDropTarget(false);

        m_dragDrop.dropRow    = row;
        m_dragDrop.dropColumn = column;

        dropTarget = at(row, column);
        if (dropTarget)
            LiveViewLayoutProps::get(dropTarget)->setIsDropTarget(true);
    }

    event->acceptProposedAction();
}

void LiveViewLayout::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!event->mimeData()->hasFormat(QLatin1String("application/x-bluecherry-dvrcamera")))
        return;

    QList<DVRCamera> cameras = DVRCamera::fromMimeData(event->mimeData());
    if (cameras.isEmpty())
        return;

    Q_ASSERT(!m_dragDrop.dragItem);

    m_dragDrop.dragItem = createNewItem();
    LiveViewLayoutProps::get(m_dragDrop.dragItem)->setIsDragItem(true);
    bool dragItemCameraProperty = m_dragDrop.dragItem->setProperty("camera", QVariant::fromValue(cameras[0]));
    Q_ASSERT(dragItemCameraProperty);
    Q_UNUSED(dragItemCameraProperty);

    updateDrag(event);

    event->acceptProposedAction();
}

void LiveViewLayout::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!m_dragDrop.dragItem)
        return;

    updateDrag(event);
    event->acceptProposedAction();
}

void LiveViewLayout::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    updateDrag(event);
    if (m_dragDrop.dragItem)
        m_dragDrop.dragItem->deleteLater();
    m_dragDrop.dragItem = 0;
}

void LiveViewLayout::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!m_dragDrop.dragItem)
        return;

    updateDrag(event);

    int row, column;
    gridPos(event->pos(), &row, &column);

    set(row, column, m_dragDrop.dragItem);
    LiveViewLayoutProps::get(m_dragDrop.dragItem)->setIsDragItem(false);

    m_dragDrop.dragItem = 0;
    event->acceptProposedAction();
}
