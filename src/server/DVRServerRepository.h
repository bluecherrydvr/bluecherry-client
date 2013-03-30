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

#ifndef DVRSERVERREPOSITORY_H
#define DVRSERVERREPOSITORY_H

#include <QObject>

class DVRServer;

class DVRServerRepository : public QObject
{
    Q_OBJECT

public:
    explicit DVRServerRepository(QObject *parent = 0);
    virtual ~DVRServerRepository();

    DVRServer * createServer(const QString &name);

    void loadServers();

    int serverCount() const;
    const QList<DVRServer *> & servers() const;
    DVRServer * serverByID(int id) const;
    QList<DVRServer *> serversWithAlerts() const;

signals:
    void serverAdded(DVRServer *server);
    void serverRemoved(DVRServer *server);
    void serverAlertsChanged();

private slots:
    void onServerRemoved(DVRServer *server);

private:
    QList<DVRServer *> m_servers;
    int m_maxServerId;

};

#endif // DVRSERVERREPOSITORY_H
