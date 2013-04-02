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

#include "DVRServer.h"
#include "DVRServerRepository.h"
#include "DVRServerSettingsReader.h"
#include "DVRServerSettingsWriter.h"
#include <QSettings>
#include <QStringList>

DVRServerRepository::DVRServerRepository(QObject *parent)
    : QObject(parent), m_maxServerId(-1)
{
}

DVRServerRepository::~DVRServerRepository()
{
}

DVRServer * DVRServerRepository::createServer(const QString& name)
{
    int id = ++m_maxServerId;

    DVRServer *server = new DVRServer(id, this);
    server->setDisplayName(name);

    m_servers.append(server);
        connect(server, SIGNAL(serverRemoved(DVRServer*)), this, SLOT(onServerRemoved(DVRServer*)));
        connect(server, SIGNAL(statusAlertMessageChanged(QString)), this, SIGNAL(serverAlertsChanged()));

    emit serverAdded(server);
    return server;
}

void DVRServerRepository::loadServers()
{
    Q_ASSERT(m_servers.isEmpty());

    QSettings settings;
    settings.beginGroup(QLatin1String("servers"));
    QStringList groups = settings.childGroups();

    DVRServerSettingsReader settingsReader;
    foreach (QString group, groups)
    {
        bool ok = false;
        int id = (int)group.toUInt(&ok);
        if (!ok)
        {
            qWarning("Ignoring invalid server ID from configuration");
            continue;
        }

        DVRServer *server = settingsReader.readServer(id);
        if (!server)
        {
            qWarning("Ignoring invalid server from configuration");
            continue;
        }

        server->setParent(this);
        connect(server, SIGNAL(serverRemoved(DVRServer*)), this, SLOT(onServerRemoved(DVRServer*)));
        connect(server, SIGNAL(statusAlertMessageChanged(QString)), this, SIGNAL(serverAlertsChanged()));

        m_servers.append(server);
        m_maxServerId = qMax(m_maxServerId, id);
    }
}

void DVRServerRepository::storeServers()
{
    DVRServerSettingsWriter settingsWriter;

    foreach (DVRServer *server, m_servers)
        settingsWriter.writeServer(server);
}

int DVRServerRepository::serverCount() const
{
    return m_servers.count();
}

const QList<DVRServer *> & DVRServerRepository::servers() const
{
    return m_servers;
}

DVRServer * DVRServerRepository::serverByID(int id) const
{
    foreach (DVRServer *server, m_servers)
        if (server->id() == id)
            return server;

    return 0;
}

QList<DVRServer *> DVRServerRepository::serversWithAlerts() const
{
    QList<DVRServer*> result;
    foreach (DVRServer *server, m_servers)
        if (!server->statusAlertMessage().isEmpty())
            result.append(server);

    return result;
}

void DVRServerRepository::onServerRemoved(DVRServer *server)
{
    if (m_servers.removeOne(server))
        emit serverRemoved(server);
}
