/*
 * Copyright 2010-2019 Bluecherry, LLC
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
#include "DVRServerConfiguration.h"
#include "DVRServerSettingsWriter.h"
#include <QSettings>

void DVRServerSettingsWriter::writeServer(DVRServer *server) const
{
    Q_ASSERT(server);

    int serverId = server->configuration().id();
    Q_ASSERT(serverId >= 0);
    QSettings settings;

    writeSetting(settings, serverId, QLatin1String("displayName"), server->configuration().displayName());
    writeSetting(settings, serverId, QLatin1String("hostname"), server->configuration().hostname());
    writeSetting(settings, serverId, QLatin1String("port"), server->configuration().port());
    writeSetting(settings, serverId, QLatin1String("username"), server->configuration().username());
    writeSetting(settings, serverId, QLatin1String("password"), server->configuration().password());
    writeSetting(settings, serverId, QLatin1String("autoConnect"), server->configuration().autoConnect());
    writeSetting(settings, serverId, QLatin1String("sslDigest"), server->configuration().sslDigest());
    writeSetting(settings, serverId, QLatin1String("connectionType"), server->configuration().connectionType());
}

void DVRServerSettingsWriter::writeSetting(QSettings &settings, int serverId, const QString &key, const QVariant &value) const
{
    settings.setValue(QString::fromLatin1("servers/%1/%2").arg(serverId).arg(key), value);
}
