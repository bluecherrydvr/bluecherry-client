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

#ifndef EVENTSOURCESMODEL_H
#define EVENTSOURCESMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include <QBitArray>
#include "camera/DVRCamera.h"

class DVRServer;
class DVRServerRepository;
class QStringList;

class EventSourcesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit EventSourcesModel(DVRServerRepository *serverRepository, QObject *parent = 0);

    virtual QMap<DVRServer*,QList<int> > checkedSources() const;

    QModelIndex indexOfCamera(DVRCamera *camera) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:
    void checkedSourcesChanged(const QMap<DVRServer*,QList<int> > &checkedSources);

private slots:
    void serverAdded(DVRServer *server);
    void serverRemoved(DVRServer *server);

private:
    struct ServerData
    {
        DVRServer *server;
        QVector<QWeakPointer<DVRCamera> > cameras;
        /* Note that the indexes are +1 from cameras */
        QBitArray checkState;
    };

    QVector<ServerData> servers;

};

#endif // EVENTSOURCESMODEL_H
