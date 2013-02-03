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

#ifndef EVENTTAGSDELEGATE_H
#define EVENTTAGSDELEGATE_H

#include <QStyledItemDelegate>

class EventTagsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit EventTagsDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool hitTestDelButton(const QModelIndex &index, const QSize &size, const QPoint &pos) const;
};

#endif // EVENTTAGSDELEGATE_H
