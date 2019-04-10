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

#include "camera/DVRCamera.h"
#include "server/DVRServer.h"
#include "server/DVRServerRepository.h"
#include "DVRCameraStreamReader.h"

DVRCameraStreamReader::DVRCameraStreamReader(DVRServerRepository *serverRepository, QDataStream &dataStream)
    : m_serverRepository(serverRepository), m_dataStream(dataStream)
{
    Q_ASSERT(m_serverRepository);
}

DVRCamera * DVRCameraStreamReader::readCamera()
{
    int serverId = -1;
    m_dataStream >> serverId;

    if (m_dataStream.status() != QDataStream::Ok || serverId < 0)
        return 0;

    int cameraId = -1;
    m_dataStream >> cameraId;
    return getCamera(serverId, cameraId);
}

DVRCamera * DVRCameraStreamReader::getCamera(int serverID, int cameraID)
{
    DVRServer *server = m_serverRepository->serverByID(serverID);
    if (!server)
        return 0;

    return server->getCamera(cameraID);
}
