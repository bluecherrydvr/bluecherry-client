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
#include "server/DVRServerConfiguration.h"
#include "DVRCameraStreamWriter.h"
#include <QDataStream>

DVRCameraStreamWriter::DVRCameraStreamWriter(QDataStream &dataStream)
    : m_dataStream(dataStream)
{
}

void DVRCameraStreamWriter::writeCamera(DVRCamera *camera)
{
    if (!camera)
        m_dataStream << (qint32)-1;
    else
        m_dataStream << (qint32) camera->data().server()->configuration().id() << (qint32)camera->data().id();
}
