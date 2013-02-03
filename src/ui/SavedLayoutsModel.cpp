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

#include "SavedLayoutsModel.h"
#include "core/BluecherryApp.h"
#include <QSettings>
#include <QStringList>

SavedLayoutsModel *SavedLayoutsModel::m_instance = 0;

SavedLayoutsModel *SavedLayoutsModel::instance()
{
    return m_instance ? m_instance : (m_instance = new SavedLayoutsModel(bcApp));
}

SavedLayoutsModel::SavedLayoutsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    populate();
}

void SavedLayoutsModel::populate()
{
    items.clear();

    QSettings settings;
    settings.beginGroup(QLatin1String("cameraLayouts"));
    QStringList keys = settings.childKeys();

    if (keys.isEmpty())
    {
        /* There always must be at least one layout; create the default */
        settings.setValue(tr("Default"), QByteArray());
        keys = settings.childKeys();
    }

    foreach (QString key, keys)
    {
        SavedLayoutData d;
        d.name = key;
        d.data = settings.value(key).toByteArray();
        items.append(d);
    }

    settings.endGroup();

    SavedLayoutData d;
    d.name = tr("New layout...");
    items.append(d);

    reset();
}

int SavedLayoutsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return items.size();
}

QModelIndex SavedLayoutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || column != 0 || row >= items.size())
        return QModelIndex();

    return createIndex(row, column, row);
}

QVariant SavedLayoutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const SavedLayoutData &d = items[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return d.name;
    case LayoutDataRole:
        return d.data;
    default:
        return QVariant();
    }
}

bool SavedLayoutsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > items.size() || parent.isValid() || !count)
        return false;

    beginInsertRows(QModelIndex(), row, row+count-1);
    SavedLayoutData d;
    for (int i = row; i < (row+count); ++i)
        items.insert(i, d);
    endInsertRows();

    return true;
}

bool SavedLayoutsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row+count > items.size() || parent.isValid() || !count)
        return false;

    /* You can't remove the 'New layout' item (which is the last one) */
    if (row+count == items.size())
        return false;

    /* You also must have at least one layout */
    if (items.size()-count < 2)
        return false;

    beginRemoveRows(QModelIndex(), row, row+count-1);

    QSettings settings;
    settings.beginGroup(QLatin1String("cameraLayouts"));

    for (int i = row; i < row+count; ++i)
        settings.remove(items[i].name);
    items.remove(row, count);

    endRemoveRows();

    return true;
}

bool SavedLayoutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= items.size())
        return false;

    SavedLayoutData &d = items[index.row()];
    QSettings settings;
    settings.beginGroup(QLatin1String("cameraLayouts"));

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (value.toString().isEmpty())
            return false;

        if (!d.name.isEmpty())
            settings.remove(d.name);

        d.name = value.toString();
        settings.setValue(d.name, d.data);
        break;

    case LayoutDataRole:
        d.data = value.toByteArray();
        if (!d.name.isEmpty())
            settings.setValue(d.name, d.data);
        break;

    default:
        return false;
    }

    return true;
}
