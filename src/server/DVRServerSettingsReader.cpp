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

#include "DVRServerSettingsReader.h"
#include "DVRServer.h"
#include <QSettings>

DVRServer * DVRServerSettingsReader::readServer(int serverId)
{
    Q_ASSERT(serverId >= 0);

    DVRServer *server = new DVRServer(serverId);
    server->setDisplayName(readSetting(serverId, QLatin1String("displayName")).toString());
    server->setHostname(readSetting(serverId, QLatin1String("hostname")).toString());
    server->setPort(readSetting(serverId, QLatin1String("port")).toInt());
    server->setUsername(readSetting(serverId, QLatin1String("username")).toString());
    server->setPassword(readSetting(serverId, QLatin1String("password")).toString());
    server->setAutoConnect(readSetting(serverId, QLatin1String("autoConnect"), true).toBool());
    server->setSslDigest(readSetting(serverId, QLatin1String("sslDigest")).toByteArray());

    return server;
}

QVariant DVRServerSettingsReader::readSetting(int serverId, const QString &key, const QVariant &def) const
{
    QSettings settings;
    return settings.value(QString::fromLatin1("servers/%1/").arg(serverId).append(key), def);
}
