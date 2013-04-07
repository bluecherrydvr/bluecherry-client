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

#include "DVRCameraData.h"
#include "DVRCamera.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include <QSettings>

DVRCameraData::DVRCameraData(DVRServer *s, int i)
    : server(s), uniqueID(i), isLoaded(false), isOnline(false), isDisabled(false),
      ptzProtocol(DVRCamera::UnknownProtocol), recordingState(DVRCamera::NoRecording)
{
    loadSavedSettings();
}

DVRCameraData::~DVRCameraData()
{
}

void DVRCameraData::loadSavedSettings()
{
    QSettings settings;
    displayName = settings.value(QString::fromLatin1("servers/%1/cameras/%2").arg(server->configuration()->id()).arg(uniqueID)).toString();
}

void DVRCameraData::doDataUpdated()
{
    if (server)
    {
        QSettings settings;
        settings.beginGroup(QString::fromLatin1("servers/%1/cameras/").arg(server->configuration()->id()));
        settings.setValue(QString::number(uniqueID), displayName);
    }

    emit dataUpdated();
}

void DVRCameraData::setRecordingState(int state)
{
    if (state == recordingState)
        return;

    recordingState = DVRCamera::RecordingState(state);
    emit recordingStateChanged(state);
}
