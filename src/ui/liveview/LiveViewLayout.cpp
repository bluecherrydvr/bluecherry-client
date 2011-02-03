#include "LiveViewLayout.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDebug>

LiveViewLayout::LiveViewLayout(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_rows(0), m_columns(0), m_itemComponent(0)
{
}

QDeclarativeItem *LiveViewLayout::createNewItem()
{
    QDeclarativeContext *context = QDeclarativeEngine::contextForObject(this);
    Q_ASSERT(context);

    if (!m_itemComponent)
        m_itemComponent = new QDeclarativeComponent(context->engine(), QUrl(QLatin1String("qrc:qml/liveview/LiveFeed.qml")), this);

    QDeclarativeItem *element = qobject_cast<QDeclarativeItem*>(m_itemComponent->create(context));
    Q_ASSERT(element);
    if (!element)
        return 0;

    element->setParentItem(this);
    return element;
}

void LiveViewLayout::doLayout()
{
    Q_ASSERT(m_items.size() == (m_rows*m_columns));

    if (m_items.isEmpty())
        return;

    qreal w = floor(width() / m_columns),
          h = floor(height() / m_rows);

    qreal x = 0, y = 0;

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

            y += h+1;
            x = 0;
        }
        else
            x += w+1;
    }
}

void LiveViewLayout::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    doLayout();
}

void LiveViewLayout::insertRow(int row)
{
    row = qBound(0, row, m_rows);
    m_rows++;

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
        m_items.insert(i, createNewItem());

    doLayout();
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

    doLayout();
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
    doLayout();
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
    doLayout();
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
    doLayout();
}
