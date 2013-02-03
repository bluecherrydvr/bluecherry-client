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

#ifndef SAVEDLAYOUTSMODEL_H
#define SAVEDLAYOUTSMODEL_H

#include <QAbstractListModel>
#include <QVector>

class SavedLayoutsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum
    {
        LayoutDataRole = Qt::UserRole
    };

    /* The singleton instance of this class should generally be used, because it allows
     * synchronziation across multiple windows. It is parented to the BluecherryApp instance. */
    static SavedLayoutsModel *instance();

    explicit SavedLayoutsModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool isNewLayoutItem(int row) const { return row == items.size() - 1; }

private:
    static SavedLayoutsModel *m_instance;

    struct SavedLayoutData
    {
        QString name;
        QByteArray data;
    };

    QVector<SavedLayoutData> items;

    void populate();
};

#endif // SAVEDLAYOUTSMODEL_H
