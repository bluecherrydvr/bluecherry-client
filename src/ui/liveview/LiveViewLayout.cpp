#include "LiveViewLayout.h"

LiveViewLayout::LiveViewLayout(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_rows(0), m_columns(0)
{
}

void LiveViewLayout::doLayout()
{
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
            i->setPos(x, y);
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

void LiveViewLayout::insertRow(int row)
{
    row = qBound(0, row, m_rows);
    m_rows++;

    for (int i = (row * m_columns), n = i+m_columns; i < n; ++i)
        m_items.insert(i, 0);

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
        /* XXX destroy the item */
    }

    QList<QDeclarativeItem*>::Iterator st = m_items.begin() + (row * m_columns);
    m_items.erase(st, st+m_columns);
    --m_rows;

    doLayout();
}

void LiveViewLayout::insertColumn(int column)
{
    column = qBound(0, column, m_columns);

    for (int i = column; i <= m_items.size(); i += m_columns)
    {
        m_items.insert(i, 0);
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
        /* XXX destroy the item */
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
