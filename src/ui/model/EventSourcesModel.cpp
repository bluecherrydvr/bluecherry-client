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
#include "camera/DVRCamera.h"
#include <QFont>
#include <QStringList>

EventSourcesModel::EventSourcesModel(DVRServerRepository *serverRepository, QObject *parent) :
    QAbstractItemModel(parent), m_serverRepository(serverRepository), m_checkedSourcesDirty(false)
{
    Q_ASSERT(m_serverRepository);

    connect(m_serverRepository, SIGNAL(serverAboutToBeAdded(DVRServer*)),
            this, SLOT(repositoryServerAboutToBeAdded(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)),
            this, SLOT(repositoryServerAdded(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverAboutToBeRemoved(DVRServer*)),
            this, SLOT(repositoryServerAboutToBeRemoved(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverRemoved(DVRServer*)),
            this, SLOT(repositoryServerRemoved(DVRServer*)));

    foreach (DVRServer *server, m_serverRepository->servers())
        addServer(server);
}

EventSourcesModel::~EventSourcesModel()
{
    foreach (DVRServer *server, m_serverRepository->servers())
        removeSystemCamera(server);
}

void EventSourcesModel::addServer(DVRServer *server)
{
    addSystemCamera(server);
    checkServer(server);

    connect(server, SIGNAL(cameraAboutToBeAdded(DVRCamera*)),
            this, SLOT(serverCameraAboutToBeAdded(DVRCamera*)));
    connect(server, SIGNAL(cameraAdded(DVRCamera*)),
            this, SLOT(serverCameraAdded(DVRCamera*)));
    connect(server, SIGNAL(cameraAboutToBeRemoved(DVRCamera*)),
            this, SLOT(serverCameraAboutToBeRemoved(DVRCamera*)));
    connect(server, SIGNAL(cameraRemoved(DVRCamera*)),
            this, SLOT(serverCameraRemoved(DVRCamera*)));
}

void EventSourcesModel::removeServer(DVRServer *server)
{
    disconnect(server, SIGNAL(cameraAboutToBeAdded(DVRCamera*)),
               this, SLOT(serverCameraAboutToBeAdded(DVRCamera*)));
    disconnect(server, SIGNAL(cameraAdded(DVRCamera*)),
               this, SLOT(serverCameraAdded(DVRCamera*)));
    disconnect(server, SIGNAL(cameraAboutToBeRemoved(DVRCamera*)),
               this, SLOT(serverCameraAboutToBeRemoved(DVRCamera*)));
    disconnect(server, SIGNAL(cameraRemoved(DVRCamera*)),
               this, SLOT(serverCameraRemoved(DVRCamera*)));

    uncheckServer(server);
    removeSystemCamera(server);
}

void EventSourcesModel::addSystemCamera(DVRServer *server)
{
    DVRCamera *camera = new DVRCamera(-1, server);
    camera->data().setDisplayName(tr("System"));
    m_systemCameras.insert(server, camera);
}

void EventSourcesModel::removeSystemCamera(DVRServer *server)
{
    delete m_systemCameras.value(server);
    m_systemCameras.remove(server);
}

void EventSourcesModel::setAllCheckedState(bool checked)
{
    foreach (DVRServer *server, m_serverRepository->servers())
        setServerCheckedState(server, checked);
}

void EventSourcesModel::checkServer(DVRServer *server)
{
    m_partiallyCheckedServers.remove(server);
    m_checkedServers.insert(server);
    foreach (DVRCamera *camera, server->cameras())
        m_checkedCameras.insert(camera);
    m_checkedCameras.insert(m_systemCameras.value(server));
}

void EventSourcesModel::uncheckServer(DVRServer* server)
{
    m_partiallyCheckedServers.remove(server);
    m_checkedServers.remove(server);
    foreach (DVRCamera *camera, server->cameras())
        m_checkedCameras.remove(camera);
    m_checkedCameras.remove(m_systemCameras.value(server));
}

void EventSourcesModel::setServerCheckedState(DVRServer *server, bool checked)
{
    if (checked)
        checkServer(server);
    else
        uncheckServer(server);
}

void EventSourcesModel::checkCamera(DVRCamera *camera)
{
    if (!m_checkedCameras.contains(camera))
    {
        m_checkedCameras.insert(camera);
        updateServerCheckState(camera->data().server());
    }
}

void EventSourcesModel::uncheckCamera(DVRCamera *camera)
{
    if (m_checkedCameras.remove(camera))
        updateServerCheckState(camera->data().server());
}

void EventSourcesModel::setCameraCheckedState(DVRCamera *camera, bool checked)
{
    if (checked)
        checkCamera(camera);
    else
        uncheckCamera(camera);
}

void EventSourcesModel::updateServerCheckState(DVRServer *server)
{
    int checkedCameras = 0;
    foreach (DVRCamera *camera, server->cameras())
        if (m_checkedCameras.contains(camera))
            checkedCameras++;
    if (m_checkedCameras.contains(m_systemCameras.value(server)))
        checkedCameras++;

    m_checkedServers.remove(server);
    m_partiallyCheckedServers.remove(server);

    if (checkedCameras == server->cameras().size() + 1)
        m_checkedServers.insert(server);
    else if (checkedCameras > 0)
        m_partiallyCheckedServers.insert(server);
}

DVRServer * EventSourcesModel::serverForRow(int row) const
{
    return m_serverRepository->servers().at(row - 1);
}

DVRCamera * EventSourcesModel::cameraForRow(DVRServer *server, int row) const
{
    Q_ASSERT(row > 0);

    if (server->cameras().size() < row)
        return 0;
    else
        return server->cameras().at(row - 1);
}

DVRCamera * EventSourcesModel::cameraForRow(int serverRow, int row) const
{
    Q_ASSERT(serverRow > 0);

    return cameraForRow(serverForRow(serverRow), row);
}

DVRCamera * EventSourcesModel::systemCameraForRow(int serverRow) const
{
    return m_systemCameras.value(serverForRow(serverRow));
}

void EventSourcesModel::repositoryServerAboutToBeAdded(DVRServer *server)
{
    Q_UNUSED(server);

    beginInsertRows(QModelIndex(), m_serverRepository->servers().count(), m_serverRepository->servers().count());
}

void EventSourcesModel::repositoryServerAdded(DVRServer *server)
{
    addServer(server);
    endInsertRows();
}

void EventSourcesModel::repositoryServerAboutToBeRemoved(DVRServer *server)
{
    int row = rowOfServer(server);
    beginRemoveRows(QModelIndex(), row, row);
    removeServer(server);
}

void EventSourcesModel::repositoryServerRemoved(DVRServer *server)
{
    Q_UNUSED(server);

    endRemoveRows();
}

void EventSourcesModel::serverCameraAboutToBeAdded(DVRCamera *camera)
{
    DVRServer *server = camera->data().server();
    QModelIndex serverIndex = indexOfServer(camera->data().server());

    beginInsertRows(serverIndex, server->cameras().count(), server->cameras().count());
}

void EventSourcesModel::serverCameraAdded(DVRCamera *camera)
{
    if (m_checkedServers.contains(camera->data().server()))
        m_checkedCameras.insert(camera);

    endInsertRows();
}

void EventSourcesModel::serverCameraAboutToBeRemoved(DVRCamera *camera)
{
    DVRServer *server = camera->data().server();
    QModelIndex serverIndex = indexOfServer(camera->data().server());
    int cameraRow = server->cameras().indexOf(camera);

    beginRemoveRows(serverIndex, cameraRow, cameraRow);
}

void EventSourcesModel::serverCameraRemoved(DVRCamera *camera)
{
    m_checkedCameras.remove(camera);

    endRemoveRows();
}

int EventSourcesModel::rowOfServer(DVRServer *server) const
{
    return m_serverRepository->servers().indexOf(server) + 1;
}

QModelIndex EventSourcesModel::indexOfServer(DVRServer *server) const
{
    if (!server)
        return QModelIndex();

    return index(rowOfServer(server), 0);
}

QModelIndex EventSourcesModel::indexOfCamera(DVRCamera *camera) const
{
    if (!camera)
        return QModelIndex();

    QModelIndex serverIndex = indexOfServer(camera->data().server());
    if (!serverIndex.isValid())
        return QModelIndex();

    int cameraRow = camera->data().server()->cameras().indexOf(camera);
    return index(cameraRow, 0, serverIndex);
}

QMap<DVRServer *, QSet<int> > EventSourcesModel::computeCheckedSources() const
{
    QMap<DVRServer*,QSet<int> > result;

    foreach (DVRServer *server, m_serverRepository->servers())
    {
        foreach (DVRCamera *camera, server->cameras())
            if (m_checkedCameras.contains(camera))
            {
                if (!result.contains(server))
                    result.insert(server, QSet<int>());
                result[server].insert(camera->data().id());
            }

        if (m_checkedCameras.contains(m_systemCameras.value(server)))
        {
            if (!result.contains(server))
                result.insert(server, QSet<int>());
            result[server].insert(-1);
        }
    }

    return result;
}

QMap<DVRServer *, QSet<int> > EventSourcesModel::checkedSources()
{
    if (m_checkedSourcesDirty)
    {
        m_checkedSources = computeCheckedSources();
        m_checkedSourcesDirty = false;
    }

    return m_checkedSources;
}

int EventSourcesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_serverRepository->serverCount() + 1;

    DVRServer *server = parent.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    if (!server)
        return 0;

    return server->cameras().size() + 1;
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

    if (row == 0)
    {
        if (parent.isValid())
            return createIndex(row, column, systemCameraForRow(parent.row()));
        else
            return createIndex(row, column);
    }

    if (parent.isValid())
        return createIndex(row, column, cameraForRow(parent.row(), row));
    else
        return createIndex(row, column, serverForRow(row));
}

QModelIndex EventSourcesModel::parent(const QModelIndex &child) const
{
    if (!child.internalPointer())
        return QModelIndex();

    DVRCamera *camera = child.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();
    if (!camera)
        return QModelIndex();

    return indexOfServer(camera->data().server());
}

Qt::ItemFlags EventSourcesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags re = Qt::ItemIsUserCheckable;

    DVRServer *server = index.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    DVRCamera *camera = index.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();

    if (server && server->isOnline())
        re |= Qt::ItemIsEnabled;
    if (camera && camera->data().server()->isOnline())
        re |= Qt::ItemIsEnabled;
    if (!server && !camera)
        re |= Qt::ItemIsEnabled;

    return re;
}

QVariant EventSourcesModel::data(const QModelIndex &index, int role) const
{
    QObject *indexObject = static_cast<QObject *>(index.internalPointer());
    if (!indexObject)
        return everythingData(role);

    DVRServer *server = qobject_cast<DVRServer *>(indexObject);
    if (server)
        return data(server, role);

    DVRCamera *camera = qobject_cast<DVRCamera *>(indexObject);
    if (camera)
        return data(camera, role);

    return QVariant();
}

QVariant EventSourcesModel::everythingData(int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
            return tr("Everything");
        case Qt::FontRole:
        {
            QFont f;
            f.setBold(true);
            return f;
        }
        case Qt::CheckStateRole:
            return m_checkedServers.size() == m_serverRepository->serverCount() ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
    }
}

QVariant EventSourcesModel::data(DVRServer *server, int role) const
{
    Q_ASSERT(server);

    switch (role)
    {
        case Qt::DisplayRole:
            return server->configuration().displayName();
        case Qt::CheckStateRole:
        {
            if (m_checkedServers.contains(server))
                return Qt::Checked;
            if (m_partiallyCheckedServers.contains(server))
                return Qt::PartiallyChecked;
            return Qt::Unchecked;
        }
        case DVRServersModel::DVRServerRole:
            return QVariant::fromValue(server);
        default:
            return QVariant();
    }
}

QVariant EventSourcesModel::data(DVRCamera *camera, int role) const
{
    Q_ASSERT(camera);

    switch (role)
    {
        case Qt::DisplayRole:
            return camera->data().displayName();
        case Qt::CheckStateRole:
            return m_checkedCameras.contains(camera) ? Qt::Checked : Qt::Unchecked;
        case DVRServersModel::DVRCameraRole:
            return QVariant::fromValue(camera);
        default:
            return QVariant();
    }
}

bool EventSourcesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;

    bool state = (value.toInt() == Qt::Checked);

    DVRServer *server = index.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    DVRCamera *camera = index.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();

    if (server)
        setServerCheckedState(server, state);
    else if (camera)
        setCameraCheckedState(camera, state);
    else
        setAllCheckedState(state);

    if (receivers(SIGNAL(checkedSourcesChanged(QMap<DVRServer*,QSet<int>>))))
    {
        QMap<DVRServer *, QSet<int> > oldCheckedSources = checkedSources();
        m_checkedSourcesDirty = true;
        QMap<DVRServer *, QSet<int> > newCheckedSources = checkedSources();

        if (oldCheckedSources != newCheckedSources)
            emit checkedSourcesChanged(checkedSources());
    }
    else
        m_checkedSourcesDirty = true;

    emit dataChanged(EventSourcesModel::index(0, 0), EventSourcesModel::index(rowCount() - 1, 0));
    return true;
}
