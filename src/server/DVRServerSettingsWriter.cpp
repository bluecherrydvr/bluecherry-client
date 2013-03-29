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

#include "DVRServerSettingsWriter.h"
#include "DVRServer.h"
#include <QSettings>

void DVRServerSettingsWriter::writeServer(DVRServer *server) const
{
    Q_ASSERT(server);

    int serverId = server->configId;
    Q_ASSERT(serverId >= 0);

    writeSetting(serverId, QLatin1String("displayName"), server->displayName());
    writeSetting(serverId, QLatin1String("hostname"), server->hostname());
    writeSetting(serverId, QLatin1String("port"), server->port());
    writeSetting(serverId, QLatin1String("username"), server->username());
    writeSetting(serverId, QLatin1String("password"), server->password());
    writeSetting(serverId, QLatin1String("autoConnect"), server->autoConnect());
    writeSetting(serverId, QLatin1String("sslDigest"), server->sslDigest());
}

void DVRServerSettingsWriter::writeSetting(int serverId, const QString &key, const QVariant &value) const
{
    QSettings settings;
    settings.setValue(QString::fromLatin1("servers/%1/").arg(serverId).append(key), value);
}
