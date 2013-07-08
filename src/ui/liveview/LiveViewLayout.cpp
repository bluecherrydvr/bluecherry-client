/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LiveViewLayout.h"
#include "camera/DVRCamera.h"
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

#define MAX_ROWS 16
#define MAX_COLUMNS 16

struct LiveViewLayout::DragDropData
{
    QDeclarativeItem *item;
    int sourceRow, sourceColumn;
    QDeclarativeItem *target;
    int targetRow, targetColumn;
    DragDropMode mode;

    DragDropData()
        : item(0), sourceRow(-1), sourceColumn(-1),
          target(0), targetRow(-1), targetColumn(-1),
          mode(DragReplace)
    {
    }
};

LiveViewLayout::LiveViewLayout(QDeclarativeItem *parent) :
    QDeclarativeItem(parent), m_rows(0), m_columns(0), m_serverRepository(0),
    m_itemComponent(0), drag(0), layoutChanges(NoLayoutChanges)
{
    setAcceptDrops(true);
    setGridSize(1, 1);
}

LiveViewLayout::~LiveViewLayout()
{
    if (drag)
        delete drag;
}

int LiveViewLayout::maxRows()
{
    return MAX_ROWS;
}

int LiveViewLayout::maxColumns()
{
    return MAX_COLUMNS;
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
    connect(LiveViewLayoutProps::get(element), SIGNAL(sizeHintChanged(QSizeF)), SLOT(updateIdealSize()));

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

void LiveViewLayout::scheduleLayout(int changes)
{
    layoutChanges |= LayoutChanges(changes);

    if (layoutChanges && !m_layoutTimer.isActive())
        m_layoutTimer.start(0, this);
}

void LiveViewLayout::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_layoutTimer.timerId())
    {
        if (layoutChanges & DoItemsLayout)
            doLayout();
        if (layoutChanges & EmitLayoutChanged)
            emit layoutChanged();
        layoutChanges = NoLayoutChanges;
        m_layoutTimer.stop();
    }
}

static const qreal maxAspectVariance = 0.4;

void LiveViewLayout::doLayout()
{
    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_items.isEmpty())
        return;

    QSizeF cellSz(floor(width() / m_columns), floor(height() / m_rows));

    int extraX = floor(width() - (cellSz.width() * m_columns)),
        extraY = floor(height() - (cellSz.height() * m_rows));

    qreal x = 0, y = 0;

    for (int r = 0, c = 0;;)
    {
        QDeclarativeItem *i = at(r, c);

        QSizeF sz = cellSz;
        if (c < extraX)
            sz.rwidth()++;
        if (r < extraY)
            sz.rheight()++;

        if (i)
        {
            LiveViewLayoutProps *ip = LiveViewLayoutProps::get(i);

            QSizeF size = ip->sizeHint(), padding = ip->sizePadding();
            if (ip->fixedAspectRatio())
            {
                if (size.isValid())
                {
                    size.scale(sz - padding, Qt::KeepAspectRatio);
                    size += padding;
                }
                else
                    size = sz;
            }
            else
            {
                if (size.isValid() && !size.isEmpty() && size.width() > padding.width()
                        && size.height() > padding.height())
                {
                    qreal desiredAspect = size.width() / size.height();
                    size = sz - padding;
                    qreal actualAspect = size.width() / size.height();

                    if (qAbs(desiredAspect - actualAspect) > maxAspectVariance)
                    {
                        desiredAspect += (desiredAspect > actualAspect) ? -maxAspectVariance : maxAspectVariance;

                        if (size.width() < size.height())
                            size.rheight() = qRound(size.width() / desiredAspect);
                        else
                            size.rwidth() = qRound(size.height() * desiredAspect);
                    }

                    size += padding;
                }
                else
                    size = sz;
            }

            i->setWidth(size.width());
            i->setHeight(size.height());
            i->setX(x + ((sz.width()-size.width()) / 2.0));
            i->setY(y + ((sz.height()-size.height()) / 2.0));
        }

        if (++c == m_columns)
        {
            if (++r == m_rows)
                break;
            c = 0;

            y += sz.height();
            x = 0;
        }
        else
            x += sz.width();
    }

    if (!(layoutChanges &= ~DoItemsLayout))
        m_layoutTimer.stop();
}

void LiveViewLayout::gridPos(const QPointF &pos, int *row, int *column)
{
    Q_ASSERT(row && column);
    if (!m_rows || !m_columns)
    {
        *row = *column = -1;
        return;
    }

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

bool LiveViewLayout::gridPos(QDeclarativeItem *item, int *row, int *column)
{
    Q_ASSERT(item && row && column);
    int i = m_items.indexOf(item);
    if (i < 0)
    {
        *row = *column = -1;
        return false;
    }

    Q_ASSERT(m_columns && m_rows);

    *row = i / m_columns;
    *column = i % m_columns;
    return true;
}

QSize LiveViewLayout::idealSize() const
{
    QSizeF sz(0, 0);

    for (int r = 0; r < m_rows; ++r)
    {
        for (int c = 0; c < m_columns; ++c)
        {
            QDeclarativeItem *item = at(r, c);
            if (item)
            {
                QSizeF hint = LiveViewLayoutProps::get(item)->sizeHint();
                sz.rwidth() += qMax(qreal(0), hint.width());
                sz.rheight() += qMax(qreal(0), hint.height());
            }
        }
    }

    return sz.toSize();
}

void LiveViewLayout::updateIdealSize()
{
    if (receivers(SIGNAL(idealSizeChanged(QSize))))
        emit idealSizeChanged(idealSize());
}

void LiveViewLayout::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size())
        doLayout();
}

void LiveViewLayout::insertRow(int row)
{
    row = qBound(0, row, m_rows);
    m_rows++;

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
        m_items.insert(i, QWeakPointer<QDeclarativeItem>());

    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

void LiveViewLayout::removeRow(int row)
{
    if (!m_rows)
        return;

    row = qBound(0, row, m_rows-1);

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
    {
        QDeclarativeItem *item = m_items[i].data();
        if (item)
            item->deleteLater();
    }

    QList<QWeakPointer<QDeclarativeItem> >::Iterator st = m_items.begin() + (row * m_columns);
    m_items.erase(st, st+m_columns);

    /* Ensure that we always have at least one row, but still clear it if asked to remove */
    if (!--m_rows)
        insertRow(0);

    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

void LiveViewLayout::insertColumn(int column)
{
    column = qBound(0, column, m_columns);

    for (int i = column, n = 0; n < m_rows; i += m_columns, ++n)
    {
        m_items.insert(i, QWeakPointer<QDeclarativeItem>());
        ++i;
    }

    m_columns++;
    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

void LiveViewLayout::removeColumn(int column)
{
    if (!m_columns)
        return;
    column = qBound(0, column, m_columns-1);

    for (int i = column; i < m_items.size(); i += m_columns)
    {
        QDeclarativeItem *item = m_items[i].data();
        if (item)
            item->deleteLater();
        m_items.removeAt(i);
        --i;
    }

    if (!--m_columns)
        insertColumn(0);

    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

bool LiveViewLayout::isRowEmpty(int rowIndex) const
{
    for (int c = 0; c < m_columns; ++c)
        if (at(rowIndex, c))
            return false;

    return true;
}

bool LiveViewLayout::isColumnEmpty(int columnIndex) const
{
    for (int r = 0; r < m_rows; ++r)
        if (at(r, columnIndex))
            return false;

    return true;
}

void LiveViewLayout::removeRows(int remove)
{
    /* If there are any empty rows, remove those first */
    for (int r = 0; r < m_rows; ++r)
    {
        if (isRowEmpty(r))
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

void LiveViewLayout::removeColumns(int remove)
{
    for (int c = 0; c < m_columns; ++c)
    {
        /* If there are any empty columns, remove those first */
        if (isColumnEmpty(c))
        {
            removeColumn(c);
            if (!--remove)
                break;
            --c;
        }
    }

    /* Otherwise, take columns from the right */
    for (int c = m_columns-1; remove && c >= 0; --c, --remove)
        removeColumn(c);

    Q_ASSERT(!remove);
}

void LiveViewLayout::setGridSize(int rows, int columns)
{
    rows = qBound(1, rows, maxRows());
    columns = qBound(1, columns, maxColumns());
    if (rows == m_rows && columns == m_columns)
        return;

    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_rows > rows)
        removeRows(m_rows - rows);

    if (m_columns > columns)
        removeColumns(m_columns - columns);

    while (m_columns < columns)
        insertColumn(m_columns);

    while (m_rows < rows)
        insertRow(m_rows);

    Q_ASSERT(m_items.size() == (m_rows*m_columns));
}
QDeclarativeItem * LiveViewLayout::at(int row, int col) const
{
    int index = coordinatesToIndex(row, col);
    if (index >= 0 && index < m_items.size())
        return m_items[index].data();
    else
        return 0;
}

void LiveViewLayout::set(int row, int col, QDeclarativeItem *item)
{
    if (row >= m_rows || col >= m_columns || (item == at(row, col)))
        return;

    QDeclarativeItem *ip = m_items[coordinatesToIndex(row, col)].data();
    if (ip == item)
        return;

    if (ip)
        ip->deleteLater();

    m_items[coordinatesToIndex(row, col)] = item;

    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

void LiveViewLayout::removeItem(QDeclarativeItem *item)
{
    int index = m_items.indexOf(item);
    if (index < 0 || !item)
        return;

    m_items[index].clear();
    item->deleteLater();
    scheduleLayout(DoItemsLayout | EmitLayoutChanged);
}

QDeclarativeItem *LiveViewLayout::addItemAuto()
{
    /* Put the item in the first empty space, top-left to bottom-right */
    int index = -1;
    for (int i = 0; i < m_items.length(); i++)
        if (m_items.value(i).isNull())
            index = i;

    if (index < 0)
    {
        /* Add a row or a column to make space, whichever has fewer */
        if (columns() < rows())
            appendColumn();
        else
            appendRow();

        for (int i = 0; i < m_items.length(); i++)
            if (m_items.value(i).isNull())
                index = i;
        // it is possible that no item was added as grid has its maximum size already
        // and all grid items are already filled
        if (index < 0)
            return 0;
    }

    m_items[index] = createNewItem();

    scheduleLayout(EmitLayoutChanged);
    doLayout();

    return m_items[index].data();
}

QDeclarativeItem *LiveViewLayout::addItem(int row, int column)
{
    if (row < 0 || column < 0)
        return 0;

    setGridSize(qMax(row, rows()), qMax(column, columns()));

    // if row or column values are too big
    if (row < rows() - 1 || column < columns() - 1)
        return 0;

    QDeclarativeItem *re = createNewItem();
    m_items[coordinatesToIndex(row, column)] = re;

    scheduleLayout(EmitLayoutChanged);
    doLayout();

    return re;
}

QDeclarativeItem *LiveViewLayout::takeItem(int row, int column)
{
    if (row < 0 || column < 0 || row >= m_rows || column >= m_columns)
        return 0;

    int i = coordinatesToIndex(row, column);
    QDeclarativeItem *item = m_items[i].data();
    m_items[i].clear();

    scheduleLayout(EmitLayoutChanged);

    return item;
}

QDeclarativeItem *LiveViewLayout::takeItem(QDeclarativeItem *item)
{
    if (!item)
        return 0;

    int r, c;
    if (gridPos(item, &r, &c))
        return takeItem(r, c);
    return 0;
}

QDeclarativeItem *LiveViewLayout::dropTarget() const
{
    return drag ? drag->target : 0;
}

QDeclarativeItem *LiveViewLayout::dragItem() const
{
    return drag ? drag->item : 0;
}

void LiveViewLayout::startDrag(QDeclarativeItem *item, DragDropMode mode)
{
    Q_ASSERT(item);
    Q_ASSERT(!drag);

    drag = new DragDropData;
    if (gridPos(item, &drag->sourceRow, &drag->sourceColumn))
        takeItem(drag->sourceRow, drag->sourceColumn);

    drag->item = item;
    drag->mode = mode;

    if (drag->mode == DragSwap && (drag->sourceRow | drag->sourceColumn) < 0)
        drag->mode = DragReplace;

    LiveViewLayoutProps::get(item)->setIsDragItem(true);

    emit dragItemChanged(item);
    updateDrag();
}

void LiveViewLayout::updateDrag()
{
    Q_ASSERT(drag);
    if (!drag)
        return;

    QPointF pos = cursorItemPos();

    int row, column;
    gridPos(pos, &row, &column);

    QDeclarativeItem *newTarget = (row >= 0 && column >= 0) ? at(row, column) : 0;

    if (drag->target != newTarget || row != drag->targetRow || column != drag->targetColumn)
    {
        QDeclarativeItem *oldTarget = drag->target;

        drag->targetRow = row;
        drag->targetColumn = column;
        drag->target = newTarget;

        if (oldTarget != drag->target)
        {
            if (oldTarget)
                LiveViewLayoutProps::get(oldTarget)->setIsDropTarget(false);
            if (drag->target)
                LiveViewLayoutProps::get(drag->target)->setIsDropTarget(true);

            emit dropTargetChanged(drag->target);
        }
    }
}

void LiveViewLayout::endDrag(bool dropped)
{
    if (!drag)
        return;

    DragDropData *d = drag;
    drag = 0;

    if (d->target)
        LiveViewLayoutProps::get(d->target)->setIsDropTarget(false);
    LiveViewLayoutProps::get(d->item)->setIsDragItem(false);

    emit dragItemChanged(0);
    emit dropTargetChanged(0);

    if (!dropped)
        d->item->deleteLater();

    delete d;
}

bool LiveViewLayout::drop()
{
    if (!drag)
        return false;

    if (drag->targetRow < 0 || drag->targetColumn < 0)
    {
        endDrag(false);
        return false;
    }

    if (drag->mode == DragSwap)
    {
        QDeclarativeItem *target = takeItem(drag->targetRow, drag->targetColumn);
        Q_ASSERT(target == drag->target);
        set(drag->sourceRow, drag->sourceColumn, target);
    }

    set(drag->targetRow, drag->targetColumn, drag->item);

    endDrag(true);
    return true;
}

QPointF LiveViewLayout::cursorItemPos() const
{
    QGraphicsView *view = scene()->views().value(0);
    Q_ASSERT(view && scene()->views().size() == 1);
    if (!view)
        return QPointF(-1, -1);

    return mapFromScene(view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos())));
}

void LiveViewLayout::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!event->mimeData()->hasFormat(QLatin1String("application/x-bluecherry-dvrcamera")))
        return;

    QList<DVRCamera *> cameras = DVRCamera::fromMimeData(m_serverRepository, event->mimeData());
    if (cameras.isEmpty())
        return;

    Q_ASSERT(!drag);

    QDeclarativeItem *item = createNewItem();
    Q_ASSERT(item);
    if (!item)
        return;

    item->setX(event->pos().x());
    item->setY(event->pos().y());
    item->setFocus(true);
    startDrag(item);

    bool dragItemCameraProperty = item->setProperty("camera", QVariant::fromValue(cameras.at(0)));
    Q_ASSERT(dragItemCameraProperty);
    Q_UNUSED(dragItemCameraProperty);

    event->acceptProposedAction();
}

void LiveViewLayout::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!drag)
        return;

    drag->item->setX(event->pos().x());
    drag->item->setY(event->pos().y());
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

QByteArray LiveViewLayout::saveLayout() const
{
    QByteArray re;
    QDataStream data(&re, QIODevice::WriteOnly);
    data.setVersion(QDataStream::Qt_4_5);

    /* -1, then version */
    data << -1 << 1;
    data << m_rows << m_columns;
    foreach (QWeakPointer<QDeclarativeItem> item, m_items)
    {
        if (!item)
            data << -1;
        else if (!item.data()->metaObject()->invokeMethod(item.data(), "saveState", Qt::DirectConnection, Q_ARG(QDataStream*,&data)))
        {
            qWarning() << "Failed to save LiveViewLayout state for item" << item.data();
            data << -1;
        }
    }

    return re;
}

bool LiveViewLayout::loadLayout(const QByteArray &buf)
{
    if (buf.isEmpty())
        return false;

    QDataStream data(buf);
    data.setVersion(QDataStream::Qt_4_5);

    /* Legacy format is [rc][cc][...]
     * Newer format is [-1][version][rc][cc][...] */
    int rc = 0, cc = 0, version = 0;
    data >> rc;
    if (rc < 0)
        data >> version;

    if (version == 0)
        data >> cc;
    else if (version > 0)
        data >> rc >> cc;

    if (data.status() != QDataStream::Ok)
        return false;

    setGridSize(rc, cc);

    // update rc, cc values if were invalid
    rc = rows();
    cc = columns();

    for (int r = 0; r < rc; ++r)
    {
        for (int c = 0; c < cc; ++c)
        {
            qint64 pos = data.device()->pos();
            int value = -1;
            data >> value;

            QDeclarativeItem *item = 0;

            if (value != -1)
            {
                /* Seek back to before the field we peeked at */
                data.device()->seek(pos);

                item = createNewItem();
                if (!item->metaObject()->invokeMethod(item, "loadState", Qt::DirectConnection,
                                                      Q_ARG(QDataStream*,&data),
                                                      Q_ARG(int,version)))
                {
                    qWarning() << "Failed to load LiveViewLayout state";
                    delete item;
                    item = 0;
                }
            }

            set(r, c, item);
        }
    }

    return (data.status() == QDataStream::Ok);
}


int LiveViewLayout::coordinatesToIndex(int row, int column) const
{
    int index = row * m_columns + column;
    if (index < 0 || index >= m_items.size())
        return -1;
    else
        return index;
}

DVRServerRepository * LiveViewLayout::serverRepository() const
{
    return m_serverRepository;
}

void LiveViewLayout::setServerRepository(DVRServerRepository* serverRepository)
{
    m_serverRepository = serverRepository;
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
