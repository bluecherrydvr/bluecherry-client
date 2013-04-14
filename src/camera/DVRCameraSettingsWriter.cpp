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

#include "DVRCamera.h"
#include "DVRCameraSettingsWriter.h"
#include "server/DVRServer.h"
#include <QSettings>

void DVRCameraSettingsWriter::writeCamera(DVRCamera *camera) const
{
    Q_ASSERT(camera);
    Q_ASSERT(camera->server());

    int serverId = camera->server()->configuration().id();
    Q_ASSERT(serverId >= 0);

    QSettings settings;
    settings.beginGroup(QString::fromLatin1("servers/%1/cameras/").arg(serverId));
    settings.setValue(QString::number(camera->id()), camera->data().displayName());
}
