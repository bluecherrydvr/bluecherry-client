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

#include "DVRCameraXMLReader.h"
#include "camera/DVRCamera.h"
#include "server/DVRServer.h"
#include <QUrl>
#include <QUrlQuery>

bool DVRCameraXMLReader::readCamera(DVRCamera *camera, QXmlStreamReader &xmlStreamReader) const
{
    Q_ASSERT(xmlStreamReader.isStartElement() && xmlStreamReader.name() == QLatin1String("device"));

    QString name;
    camera->data().setPtzProtocol(DVRCamera::UnknownProtocol);

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
            camera->data().setPtzProtocol(DVRCamera::parseProtocol(xmlStreamReader.readElementText()));
        }
        else if (xmlStreamReader.name() == QLatin1String("disabled"))
        {
            bool ok = false;
            camera->data().setDisabled(xmlStreamReader.readElementText().toInt(&ok));
            if (!ok)
                camera->data().setDisabled(false);
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
    url.setPath(QString::fromLatin1("/live/") + QString::number(camera->data().id()));
    camera->setRtspStreamUrl(url);

    QUrl mjpegUrl;
    mjpegUrl.setUserName(camera->data().server()->configuration().username());
    mjpegUrl.setPassword(camera->data().server()->configuration().password());
    mjpegUrl.setScheme(QLatin1String("https"));
    mjpegUrl.setHost(camera->data().server()->url().host());
    mjpegUrl.setPort(camera->data().server()->serverPort());
    mjpegUrl.setPath(QLatin1String("/media/mjpeg.php"));
    QUrlQuery urlQuery(mjpegUrl);
    urlQuery.addQueryItem(QLatin1String("id"), QString::number(camera->data().id()));
    urlQuery.addQueryItem(QLatin1String("multipart"), QLatin1String("true"));
    mjpegUrl.setQuery(urlQuery);

    camera->setMjpegStreamUrl(mjpegUrl);

    camera->streamsInitialized();

    return true;
}
