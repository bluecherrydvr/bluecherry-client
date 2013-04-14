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
#include "DVRCameraXMLReader.h"
#include "server/DVRServer.h"
#include <QUrl>

bool DVRCameraXMLReader::readCamera(DVRCamera *camera, QXmlStreamReader &xmlStreamReader) const
{
    Q_ASSERT(xmlStreamReader.isStartElement() && xmlStreamReader.name() == QLatin1String("device"));

    QString name;
    camera->m_data.setPtzProtocol(DVRCamera::UnknownProtocol);

    while (xmlStreamReader.readNext() != QXmlStreamReader::Invalid)
    {
        if (xmlStreamReader.tokenType() == QXmlStreamReader::EndElement && xmlStreamReader.name() == QLatin1String("device"))
            break;
        else if (xmlStreamReader.tokenType() != QXmlStreamReader::StartElement)
            continue;

        if (xmlStreamReader.name() == QLatin1String("device_name"))
        {
            name = xmlStreamReader.readElementText();
        }
        else if (xmlStreamReader.name() == QLatin1String("ptz_control_protocol"))
        {
            camera->m_data.setPtzProtocol(DVRCamera::parseProtocol(xmlStreamReader.readElementText()));
        }
        else if (xmlStreamReader.name() == QLatin1String("disabled"))
        {
            bool ok = false;
            camera->m_data.setDisabled(xmlStreamReader.readElementText().toInt(&ok));
            if (!ok)
                camera->m_data.setDisabled(false);
        }
        else
            xmlStreamReader.skipCurrentElement();
    }

    if (name.isEmpty())
        name = QString::fromLatin1("#%2").arg(camera->data().id());

    camera->data().setDisplayName(name);
    QUrl url;
    url.setScheme(QLatin1String("rtsp"));
    url.setUserName(camera->data().server()->configuration().username());
    url.setPassword(camera->data().server()->configuration().password());
    url.setHost(camera->data().server()->url().host());
    url.setPort(camera->data().server()->rtspPort());
    url.setPath(QString::fromLatin1("live/") + QString::number(camera->data().id()));
    camera->m_streamUrl = url.toString().toLatin1();

    /* Changing stream URL or disabled flag will change online state */
    emit camera->onlineChanged(camera->isOnline());
    return true;
}
