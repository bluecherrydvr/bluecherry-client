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

#ifndef DVRSERVERSMODEL_H
#define DVRSERVERSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include "core/DVRCamera.h"

class DVRServer;
class DVRServerRepository;

class DVRServersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum
    {
        ServerPtrRole = Qt::UserRole,
        DVRCameraRole
    };

    explicit DVRServersModel(DVRServerRepository *serverRepository, QObject *parent = 0);

    DVRServer *serverForRow(const QModelIndex &index) const;
    DVRCamera cameraForRow(const QModelIndex &index) const;

    void setOfflineDisabled(bool offlineDisabled);

    QModelIndex indexForServer(DVRServer *server) const;
    QModelIndex indexForCamera(const DVRCamera &camera) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;

private slots:
    void serverDataChanged();
    void serverAdded(DVRServer *server);
    void serverRemoved(DVRServer *server);
    void cameraDataChanged();
    void cameraAdded(const DVRCamera &camera);
    void cameraRemoved(const DVRCamera &camera);

private:
    struct Item
    {
        DVRServer *server;
        QList<DVRCamera> cameras;
    };

    QVector<Item> items;
    QIcon statusIcon, statusErrorIcon, statusAlertIcon;
    bool m_offlineDisabled;
};

#endif // DVRSERVERSMODEL_H
