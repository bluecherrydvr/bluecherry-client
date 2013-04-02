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

#include "EventSourcesModel.h"
#include "DVRServersModel.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerRepository.h"
#include "core/DVRCamera.h"
#include <QFont>
#include <QStringList>

EventSourcesModel::EventSourcesModel(DVRServerRepository *serverRepository, QObject *parent)
    : QAbstractItemModel(parent)
{
    Q_ASSERT(serverRepository);

    connect(serverRepository, SIGNAL(serverAdded(DVRServer*)), this, SLOT(serverAdded(DVRServer*)));
    connect(serverRepository, SIGNAL(serverRemoved(DVRServer*)), this, SLOT(serverRemoved(DVRServer*)));

    QList<DVRServer*> sl = serverRepository->servers();
    servers.reserve(sl.size());

    blockSignals(true);
    foreach (DVRServer *s, sl)
        serverAdded(s);
    blockSignals(false);
}

void EventSourcesModel::serverAdded(DVRServer *server)
{
    beginInsertRows(QModelIndex(), servers.count(), servers.count());

    ServerData sd;
    sd.server = server;
    sd.cameras = QVector<DVRCamera>::fromList(server->cameras());
    sd.checkState.fill(true, sd.cameras.size()+1);
    servers.append(sd);

    endInsertRows();
}

void EventSourcesModel::serverRemoved(DVRServer *server)
{
    for (int i = 0; i < servers.count(); i++)
        if (server == servers.at(i).server)
        {
            beginRemoveRows(QModelIndex(), i, i);
            servers.remove(i);
            endRemoveRows();

            return;
        }
}

QModelIndex EventSourcesModel::indexOfCamera(const DVRCamera &camera) const
{
    for (int r = 0; r < servers.size(); ++r)
    {
        if (servers[r].server == camera.server())
        {
            int cr = servers[r].cameras.indexOf(camera);
            if (cr < 0)
                return QModelIndex();

            return index(cr+1, 0, index(r+1, 0));
        }
    }

    return QModelIndex();
}

QMap<DVRServer*,QList<int> > EventSourcesModel::checkedSources() const
{
    QMap<DVRServer*,QList<int> > re;

    for (QVector<ServerData>::ConstIterator it = servers.begin(); it != servers.end(); ++it)
    {
        QList<int> sl;

        for (int i = 0; i < it->checkState.size(); ++i)
        {
            if (!i && it->checkState[i])
                sl.append(-1);
            else if (it->checkState[i])
                sl.append(it->cameras[i-1].uniqueId());
        }

        if (!sl.isEmpty())
            re.insert(it->server, sl);
    }

    return re;
}

int EventSourcesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if (!parent.row() || parent.internalId() >= 0)
            return 0;
        const ServerData &sd = servers[parent.row()-1];
        return sd.cameras.size()+1;
    }
    else
        return servers.size()+1;
}

int EventSourcesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex EventSourcesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0)
        return QModelIndex();

    if (parent.isValid())
        return createIndex(row, column, parent.row());
    else
        return createIndex(row, column, -1);
}

QModelIndex EventSourcesModel::parent(const QModelIndex &child) const
{
    int r = child.internalId();
    if (r < 0)
        return QModelIndex();
    return index(r, 0, QModelIndex());
}

Qt::ItemFlags EventSourcesModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    Qt::ItemFlags re = Qt::ItemIsUserCheckable;

    QModelIndex server = index.parent();
    if (!server.isValid())
        server = index;

    if (!server.row() || servers[server.row()-1].server->isOnline())
        re |= Qt::ItemIsEnabled;

    return re;
}

QVariant EventSourcesModel::data(const QModelIndex &index, int role) const
{
    if (index.internalId() < 0)
    {
        if (!index.row())
        {
            if (role == Qt::DisplayRole)
            {
                return tr("Everything");
            }
            else if (role == Qt::FontRole)
            {
                QFont f;
                f.setBold(true);
                return f;
            }
            else if (role == Qt::CheckStateRole)
            {
                foreach (const ServerData &sd, servers)
                {
                    if (sd.checkState.count(true) != sd.cameras.size()+1)
                        return Qt::Unchecked;
                }

                return Qt::Checked;
            }
        }
        else
        {
            const ServerData &sd = servers[index.row()-1];
            if (role == Qt::DisplayRole)
            {
                return sd.server->configuration()->displayName();
            }
            else if (role == Qt::CheckStateRole)
            {
                int c = sd.checkState.count(true);
                if (!c || !sd.server->isOnline())
                    return Qt::Unchecked;
                else if (c == sd.cameras.size()+1)
                    return Qt::Checked;
                else
                    return Qt::PartiallyChecked;
            }
            else if (role == DVRServersModel::ServerPtrRole)
                return QVariant::fromValue(sd.server);
        }
    }
    else
    {
        const ServerData &sd = servers[index.internalId()-1];
        if (!index.row())
        {
            switch (role)
            {
            case Qt::DisplayRole: return tr("System");
            }
        }
        else
        {
            const DVRCamera &camera = sd.cameras[index.row()-1];
            switch (role)
            {
            case Qt::DisplayRole: return camera.displayName();
            }
        }

        switch (role)
        {
        case Qt::CheckStateRole:
            return (sd.checkState[index.row()] && sd.server->isOnline()) ? Qt::Checked : Qt::Unchecked;
        }
    }

    return QVariant();
}

bool EventSourcesModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;

    bool state = (value.toInt() == Qt::Checked);

    if (idx.internalId() < 0)
    {
        if (!idx.row())
        {
            /* Everything */
            for (QVector<ServerData>::Iterator it = servers.begin(); it != servers.end(); ++it)
                it->checkState.fill(state);
        }
        else
        {
            ServerData &sd = servers[idx.row()-1];
            if (sd.checkState.count(state) == sd.checkState.size())
                return true;

            sd.checkState.fill(state);
        }
    }
    else
    {
        ServerData &sd = servers[idx.internalId()-1];
        if (sd.checkState[idx.row()] == state)
            return true;

        sd.checkState[idx.row()] = state;
    }

    emit dataChanged(index(0, 0), index(rowCount()-1, 0));

    if (receivers(SIGNAL(checkedSourcesChanged(QMap<DVRServer*,QList<int>>))))
        emit checkedSourcesChanged(checkedSources());

    return true;
}
