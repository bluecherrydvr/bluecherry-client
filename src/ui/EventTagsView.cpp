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

#include "EventTagsView.h"
#include "EventTagsDelegate.h"
#include "model/EventTagsModel.h"
#include <QMouseEvent>

EventTagsView::EventTagsView(QWidget *parent)
    : QListView(parent), cachedSizeHint(0, 0)
{
    setItemDelegate(new EventTagsDelegate(this));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
}

QSize EventTagsView::minimumSizeHint() const
{
    return QSize();
}

QSize EventTagsView::sizeHint() const
{
    QMargins margin = contentsMargins();
    return cachedSizeHint + QSize(margin.left() + margin.right(), margin.top() + margin.bottom());
}

void EventTagsView::setModel(QAbstractItemModel *model)
{
    QListView::setModel(model);
    calculateSizeHint();
}

void EventTagsView::reset()
{
    QListView::reset();
    calculateSizeHint();
}

void EventTagsView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i)
    {
        QSize isz = sizeHintForIndex(model()->index(i, modelColumn(), parent));
        cachedSizeHint.rwidth() = qMax(cachedSizeHint.width(), isz.width());
        cachedSizeHint.rheight() += isz.height();
    }

    if (isHidden())
        show();

    updateGeometry();
}

void EventTagsView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; ++i)
    {
        QSize isz = sizeHintForIndex(model()->index(i, modelColumn(), parent));
        cachedSizeHint.rheight() -= isz.height();
    }

    if (cachedSizeHint.isEmpty())
        hide();

    updateGeometry();
}

void EventTagsView::calculateSizeHint()
{
    ensurePolished();
    QSize size(0, 0);

    QModelIndex parent = rootIndex();
    for (int i = 0, n = model()->rowCount(parent); i < n; ++i)
    {
        QSize isz = sizeHintForIndex(model()->index(i, modelColumn(), parent));
        size.rwidth() = qMax(size.width(), isz.width());
        size.rheight() += isz.height();
    }

    cachedSizeHint = size;
    if (cachedSizeHint.isEmpty())
        hide();

    updateGeometry();
}

void EventTagsView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index;
    if (event->button() == Qt::LeftButton && (index = indexAt(event->pos())).isValid())
    {
        Q_ASSERT(qobject_cast<EventTagsDelegate*>(itemDelegate(index)));
        EventTagsDelegate *delegate = static_cast<EventTagsDelegate*>(itemDelegate(index));

        QRect indexRect = visualRect(index);
        QPoint indexPos = event->pos() - indexRect.topLeft();

        if (delegate->hitTestDelButton(index, indexRect.size(), indexPos))
        {
            removeTag(index);
            event->accept();
            return;
        }
    }

    QListView::mousePressEvent(event);
}

void EventTagsView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid())
        update(idx);

    QListView::mouseMoveEvent(event);
}

void EventTagsView::removeTag(const QModelIndex &index)
{
    EventTagsModel *m = qobject_cast<EventTagsModel*>(model());
    Q_ASSERT(m);
    if (!m)
        return;

    m->removeTag(index);
}
