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

#include "DVRCameraData.h"
#include "DVRCamera.h"
#include "server/DVRServer.h"

DVRCameraData::DVRCameraData(int id, DVRServer *server)
    : m_id(id), m_server(server), m_disabled(false), m_ptzProtocol(DVRCamera::UnknownProtocol)
{
}

DVRCameraData::~DVRCameraData()
{
}

void DVRCameraData::setDisplayName(const QString &name)
{
    if (m_displayName == name)
        return;

    m_displayName = name;
    emit changed();
}

void DVRCameraData::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
        return;

    m_disabled = disabled;
    emit changed();
}

void DVRCameraData::setPtzProtocol(qint8 ptzProtocol)
{
    if (m_ptzProtocol == ptzProtocol)
        return;

    m_ptzProtocol = ptzProtocol;
    emit changed();
}

int DVRCameraData::id() const
{
    return m_id;
}

DVRServer* DVRCameraData::server() const
{
    return m_server;
}

QString DVRCameraData::displayName() const
{
    return m_displayName;
}

bool DVRCameraData::disabled() const
{
    return m_disabled;
}

qint8 DVRCameraData::ptzProtocol() const
{
    return m_ptzProtocol;
}
