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

#include "camera/DVRCamera.h"
#include "server/DVRServer.h"
#include <QAbstractItemModel>
#include <QSet>
#include <QVector>

class DVRCamera;
class DVRServer;
class DVRServerRepository;
class QStringList;

class EventSourcesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit EventSourcesModel(DVRServerRepository *serverRepository, QObject *parent = 0);
    virtual ~EventSourcesModel();

    QMap<DVRServer *, QList<int> > checkedSources();

    int rowOfServer(DVRServer *server) const;
    QModelIndex indexOfServer(DVRServer *server) const;
    QModelIndex indexOfCamera(DVRCamera *camera) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:
    void checkedSourcesChanged(QMap<DVRServer*,QList<int> > sources);

private slots:
    void repositoryServerAboutToBeAdded(DVRServer *server);
    void repositoryServerAdded(DVRServer *server);
    void repositoryServerAboutToBeRemoved(DVRServer *server);
    void repositoryServerRemoved(DVRServer *server);

    void serverCameraAboutToBeAdded(DVRCamera *camera);
    void serverCameraAdded(DVRCamera *camera);
    void cameraAboutToBeRemoved(DVRCamera *camera);
    void serverCameraRemoved(DVRCamera *camera);

private:
    DVRServerRepository *m_serverRepository;
    QMap<DVRServer *, DVRCamera *> m_systemCameras;
    QSet<DVRServer *> m_checkedServers;
    QSet<DVRServer *> m_partiallyCheckedServers;
    QSet<DVRCamera *> m_checkedCameras;

    bool m_checkedSourcesDirty;
    QMap<DVRServer *, QList<int> > m_checkedSources;

    void addServer(DVRServer *server);
    void removeServer(DVRServer *server);

    void addSystemCamera(DVRServer *server);
    void removeSystemCamera(DVRServer *server);
    void checkServer(DVRServer *server);
    void uncheckServer(DVRServer *server);
    void setServerCheckedState(DVRServer *server, bool checked);
    void checkCamera(DVRCamera *camera);
    void uncheckCamera(DVRCamera *camera);
    void setCameraCheckedState(DVRCamera *camera, bool checked);
    void updateServerCheckState(DVRServer *server);

    DVRServer * serverForRow(int row) const;
    DVRCamera * cameraForRow(DVRServer *server, int row) const;
    DVRCamera * cameraForRow(int serverRow, int row) const;
    DVRCamera * systemCameraForRow(int serverRow) const;

    QVariant everythingData(int role) const;
    QVariant data(DVRServer *server, int role) const;
    QVariant data(DVRCamera *camera, int role) const;

    QMap<DVRServer *, QList<int> > computeCheckedSources() const;

};

#endif // EVENTSOURCESMODEL_H
