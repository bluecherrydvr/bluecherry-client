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
#include "camera/DVRCameraStreamReader.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "core/BluecherryApp.h"
#include "core/CameraPtzControl.h"
#include "core/MJpegStream.h"
#include "core/LiveStream.h"
#include <QMimeData>
#include <QSettings>
#include <QXmlStreamReader>

DVRCamera::DVRCamera(int id, DVRServer *server)
    : QObject(), m_data(id, server), m_isOnline(false), m_recordingState(NoRecording)
{
    connect(&m_data, SIGNAL(changed()), this, SIGNAL(dataUpdated()));
}

DVRCamera::~DVRCamera()
{
}

void DVRCamera::setDisplayName(const QString &displayName)
{
    m_data.setDisplayName(displayName);
}

void DVRCamera::setOnline(bool on)
{
    if (on == m_isOnline)
        return;

    m_isOnline = on;
    emit onlineChanged(isOnline());
}

DVRCamera::PtzProtocol DVRCamera::parseProtocol(const QString &protocol)
{
    if (protocol == QLatin1String("none") || protocol.isEmpty())
        return NoPtz;
    else if (protocol.startsWith(QLatin1String("PELCO")))
        return PelcoPtz;
    else
        return UnknownProtocol;
}

LiveStream * DVRCamera::liveStream()
{
    if (!m_liveStream)
    {
        LiveStream * re = new LiveStream(this);
        connect(this, SIGNAL(onlineChanged(bool)), re, SLOT(setOnline(bool)));
        re->setOnline(isOnline());
        m_liveStream = re;
    }

    return m_liveStream.data();
}

QList<DVRCamera *> DVRCamera::fromMimeData(const QMimeData *mimeData)
{
    QByteArray data = mimeData->data(QLatin1String("application/x-bluecherry-dvrcamera"));
    QDataStream stream(&data, QIODevice::ReadOnly);
    DVRCameraStreamReader reader(bcApp->serverRepository(), stream);

    QList<DVRCamera *> result;
    while (!stream.atEnd() && stream.status() == QDataStream::Ok)
    {
        DVRCamera *camera = reader.readCamera();
        if (camera)
            result.append(camera);
    }

    return result;
}

QSharedPointer<CameraPtzControl> DVRCamera::sharedPtzControl()
{
    QSharedPointer<CameraPtzControl> result = m_ptzControl;
    if (!result)
    {
        result = QSharedPointer<CameraPtzControl>(new CameraPtzControl(this));
        m_ptzControl = result.toWeakRef();
    }

    return result;
}
