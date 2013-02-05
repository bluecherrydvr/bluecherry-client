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

#include "EventTagsModel.h"

EventTagsModel::EventTagsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    /* Placeholder for tags that will be parsed out of the event data */
    tags << QLatin1String("fake tag") << QLatin1String("more tags") << QLatin1String("third tag");
}

int EventTagsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return tags.size();
    else
        return 0;
}

QVariant EventTagsModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());

    switch (role)
    {
    case Qt::DisplayRole:
        return tags[index.row()];
    }

    return QVariant();
}

void EventTagsModel::removeTag(const QModelIndex &index)
{
    /* TODO: Actually remove the tag from wherever we got it */
    if (!index.isValid() || index.row() < 0 || index.row() >= tags.size())
        return;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    tags.removeAt(index.row());
    endRemoveRows();
}
