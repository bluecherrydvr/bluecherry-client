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

#include "DVRServersModel.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerRepository.h"
#include "camera/DVRCamera.h"
#include "camera/DVRCameraStreamWriter.h"
#include "core/ServerRequestManager.h"
#include <QTextDocument>
#include <QApplication>
#include <QStyle>
#include <QMimeData>
#include <QDataStream>

DVRServersModel::DVRServersModel(DVRServerRepository *serverRepository, QObject *parent)
    : QAbstractItemModel(parent), m_offlineDisabled(false)
{
    Q_ASSERT(serverRepository);

    statusIcon = QIcon(QLatin1String(":/icons/status.png"));
    statusIcon.addFile(QLatin1String(":/icons/status-offline.png"), QSize(), QIcon::Disabled, QIcon::Off);

    statusErrorIcon = QIcon(QLatin1String(":/icons/status-error.png"));
    statusErrorIcon.addPixmap(statusErrorIcon.pixmap(16), QIcon::Disabled);

    statusAlertIcon = QIcon(QLatin1String(":/icons/status-alert.png"));

    connect(serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(serverAdded(DVRServer*)));
    connect(serverRepository, SIGNAL(serverRemoved(DVRServer*)), SLOT(serverRemoved(DVRServer*)));

    const QList<DVRServer *> &servers = serverRepository->servers();
    items.reserve(servers.size());

    blockSignals(true);
    for (int i = 0; i < servers.size(); ++i)
        serverAdded(servers[i]);
    blockSignals(false);
}

void DVRServersModel::serverAdded(DVRServer *server)
{
    Item i;
    i.server = server;

    foreach (DVRCamera *camera, server->cameras())
        i.cameras.append(camera);

    beginInsertRows(QModelIndex(), items.size(), items.size());
    items.append(i);
    endInsertRows();

    connect(server, SIGNAL(changed()), SLOT(serverDataChanged()));
    connect(server, SIGNAL(statusChanged(int)), SLOT(serverDataChanged()));
    connect(server, SIGNAL(cameraAdded(DVRCamera*)), SLOT(cameraAdded(DVRCamera*)));
    connect(server, SIGNAL(cameraRemoved(DVRCamera*)), SLOT(cameraRemoved(DVRCamera*)));
    connect(server, SIGNAL(statusAlertMessageChanged(QString)), SLOT(serverDataChanged()));
}

void DVRServersModel::serverRemoved(DVRServer *server)
{
    int row = indexForServer(server).row();
    if (row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    items.remove(row);
    endRemoveRows();

    server->disconnect(this);
}

void DVRServersModel::cameraAdded(DVRCamera *camera)
{
    Q_ASSERT(camera);

    QModelIndex parent = indexForServer(camera->server());
    if (!parent.isValid())
        return;

    Item &it = items[parent.row()];

    connect(camera, SIGNAL(dataUpdated()), SLOT(cameraDataChanged()));

    beginInsertRows(parent, it.cameras.size(), it.cameras.size());
    it.cameras.append(camera);
    endInsertRows();
}

void DVRServersModel::cameraRemoved(DVRCamera *camera)
{
    QModelIndex parent = indexForServer(camera->server());
    if (!parent.isValid())
        return;

    int row = items[parent.row()].cameras.indexOf(camera);
    if (row < 0)
        return;

    camera->disconnect(this);

    beginRemoveRows(parent, row, row);
    items[parent.row()].cameras.removeAt(row);
    endRemoveRows();
}

void DVRServersModel::cameraDataChanged()
{
    DVRCamera *camera = qobject_cast<DVRCamera *>(sender());
    if (!camera)
        return;

    QModelIndex index = indexForCamera(camera);
    if (!index.isValid())
        return;

    emit dataChanged(index, index);
}

DVRServer * DVRServersModel::serverForRow(const QModelIndex &index) const
{
    if (index.internalPointer() || index.row() < 0 || index.row() >= items.size())
        return 0;

    return items[index.row()].server;
}

DVRCamera * DVRServersModel::cameraForRow(const QModelIndex &index) const
{
    DVRServer *server = static_cast<DVRServer*>(index.internalPointer());
    QModelIndex serverIndex;
    if (!server || !(serverIndex = indexForServer(server)).isValid() || serverIndex.row() >= items.size()
        || index.row() < 0 || index.row() >= items[serverIndex.row()].cameras.size())
        return 0;

    return items[serverIndex.row()].cameras[index.row()];
}

QModelIndex DVRServersModel::indexForServer(DVRServer *server) const
{
    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i].server == server)
            return index(i, 0);
    }

    return QModelIndex();
}

QModelIndex DVRServersModel::indexForCamera(DVRCamera *camera) const
{
    for (int i = 0; i < items.size(); ++i)
    {
        const Item &it = items[i];

        if (it.server == camera->server())
        {
            int r = it.cameras.indexOf(camera);
            if (r < 0)
                return QModelIndex();

            return index(r, 0, index(i, 0));
        }
    }

    return QModelIndex();
}

void DVRServersModel::setOfflineDisabled(bool offlineDisabled)
{
    if (m_offlineDisabled == offlineDisabled)
        return;

    m_offlineDisabled = offlineDisabled;
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

int DVRServersModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return items.size();

    int row = parent.row();
    if (row < 0 || row >= items.size() || !serverForRow(parent))
        return 0;

    return items[row].cameras.size();
}

int DVRServersModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QModelIndex DVRServersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        if (row < 0 || column < 0 || row >= items.size() || column >= columnCount())
            return QModelIndex();

        return createIndex(row, column, 0);
    }
    else
    {
        DVRServer *s;
        if (!(s = serverForRow(parent)) || row < 0 || column < 0 || column >= columnCount(parent))
            return QModelIndex();

        if (row >= items[parent.row()].cameras.size())
            return QModelIndex();

        return createIndex(row, column, s);
    }
}

QModelIndex DVRServersModel::parent(const QModelIndex &child) const
{
    DVRServer *server = static_cast<DVRServer*>(child.internalPointer());
    return server ? indexForServer(server) : QModelIndex();
}

#include <QDebug>

Qt::ItemFlags DVRServersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags re = Qt::ItemIsSelectable;
    DVRServer *s = 0;
    DVRCamera *camera = 0;

    if (index.internalPointer())
    {
        camera = cameraForRow(index);
        if (camera)
            s = camera->server();
    }
    else
        s = serverForRow(index);

    if (!m_offlineDisabled || (s && s->isOnline() && (!camera || !camera->isDisabled())))
        re |= Qt::ItemIsEnabled;
    else
        return re;

    if (!index.parent().isValid())
        re |= Qt::ItemIsEditable;
    re |= Qt::ItemIsDragEnabled;

    return re;
}

QVariant DVRServersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DVRServer *server = serverForRow(index);
    if (server)
    {
        if (role == Qt::ToolTipRole)
        {
            return tr("<span style='white-space:nowrap'><b>%1</b><br>%3 @ %2</span>", "tooltip")
                    .arg(Qt::escape(server->configuration()->displayName()))
                    .arg(Qt::escape(server->configuration()->hostname()))
                    .arg(Qt::escape(server->configuration()->username()));
        }
        else if (role == DVRServerRole)
            return QVariant::fromValue(server);

        switch (index.column())
        {
        case 0:
            if (role == Qt::DisplayRole || role == Qt::EditRole)
                return server->configuration()->displayName();
            else if (role == Qt::DecorationRole)
            {
                if (server->status() == DVRServer::LoginError)
                    return statusErrorIcon;

                if (!server->statusAlertMessage().isEmpty())
                    return statusAlertIcon;

                if (m_offlineDisabled)
                    return statusIcon;
                else
                    return statusIcon.pixmap(16, server->isOnline() ? QIcon::Normal : QIcon::Disabled);
            }
            break;
        case 1:
            if (role == Qt::DisplayRole || role == Qt::EditRole)
                return server->configuration()->hostname();
            break;
        case 2:
            if (role == Qt::DisplayRole || role == Qt::EditRole)
                return server->configuration()->username();
            break;
        }
    }

    DVRCamera *camera = cameraForRow(index);
    if (camera)
    {
        switch (role)
        {
        case DVRCameraRole:
            return QVariant::fromValue(camera);
        case Qt::DisplayRole:
            return camera->displayName();
        case Qt::DecorationRole:
            return QIcon(QLatin1String(":/icons/webcam.png"));
        }
    }

    return QVariant();
}

QVariant DVRServersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case 0:
        return tr("DVR Server");
    case 1:
        return tr("Hostname");
    case 2:
        return tr("Username");
    }

    return QVariant();
}

bool DVRServersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    DVRServer *server = serverForRow(index);
    if (!server)
        return false;

    switch (index.column())
    {
    case 0:
        {
            QString name = value.toString().trimmed();
            if (name.isEmpty())
                return false;

            /* dataChanged will be emitted in response to the DVRServer::changed() signal */
            server->configuration()->setDisplayName(name);
        }
        break;
    case 1:
        server->configuration()->setHostname(value.toString());
        break;
    case 2:
        server->configuration()->setUsername(value.toString());
        break;
    case 3:
        server->configuration()->setPassword(value.toString());
        break;
    default:
        return false;
    }

    return true;
}

QStringList DVRServersModel::mimeTypes() const
{
    return QStringList() << QLatin1String("application/x-bluecherry-dvrcamera");
}

/* application/x-bluecherry-dvrcamera is a list of int,int where each pair has the
 * IDs of the server and the camera respectively. This is only used internally. */

QMimeData *DVRServersModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    DVRCameraStreamWriter cameraWriter(stream);

    foreach (QModelIndex index, indexes)
    {
        DVRServer *server = serverForRow(index);
        if (server)
        {
            foreach (DVRCamera *camera, server->cameras())
                cameraWriter.writeCamera(camera);
        }
        else
        {
            DVRCamera *camera = cameraForRow(index);
            if (camera)
                cameraWriter.writeCamera(camera);
        }
    }

    QMimeData *mime = new QMimeData;
    mime->setData(QLatin1String("application/x-bluecherry-dvrcamera"), data);
    return mime;
}

void DVRServersModel::serverDataChanged()
{
    DVRServer *server = qobject_cast<DVRServer*>(sender());
    if (!server)
    {
        ServerRequestManager *srm = qobject_cast<ServerRequestManager*>(sender());
        if (!srm)
            return;
        server = srm->server;
    }

    int row = indexForServer(server).row();
    if (row < 0)
        return;

    emit dataChanged(index(row, 0), index(row, columnCount()-1));
}
