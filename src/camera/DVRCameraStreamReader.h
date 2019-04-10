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

#ifndef DVRCAMERASTREAMREADER_H
#define DVRCAMERASTREAMREADER_H

class QDataStream;
class DVRCamera;
class DVRServerRepository;

class DVRCameraStreamReader
{
public:
    explicit DVRCameraStreamReader(DVRServerRepository *serverRepository, QDataStream &dataStream);

    DVRCamera * readCamera();

private:
    DVRServerRepository *m_serverRepository;
    QDataStream &m_dataStream;

    DVRCamera * getCamera(int serverID, int cameraID);

};

#endif // DVRCAMERASTREAMREADER_H
