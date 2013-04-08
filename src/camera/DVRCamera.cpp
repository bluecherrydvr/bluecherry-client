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
#include "core/MJpegStream.h"
#include "core/LiveStream.h"
#include <QXmlStreamReader>
#include <QMimeData>
#include <QSettings>

DVRCamera::DVRCamera(DVRCameraData *dt)
    : QObject(), d(dt)
{
    connectData();
}

void DVRCamera::connectData()
{
    if (!d)
        return;

    connect(d.data(), SIGNAL(onlineChanged(bool)), this, SIGNAL(onlineChanged(bool)));
    connect(d.data(), SIGNAL(dataUpdated()), this, SIGNAL(dataUpdated()));
    connect(d.data(), SIGNAL(recordingStateChanged(int)), this, SIGNAL(recordingStateChanged(int)));
}

void DVRCamera::disconnectData()
{
    if (!d)
        return;

    disconnect(d.data(), SIGNAL(onlineChanged(bool)), this, SIGNAL(onlineChanged(bool)));
    disconnect(d.data(), SIGNAL(dataUpdated()), this, SIGNAL(dataUpdated()));
    disconnect(d.data(), SIGNAL(recordingStateChanged(int)), this, SIGNAL(recordingStateChanged(int)));
}

void DVRCamera::setOnline(bool on)
{
    if (!d || on == d->isOnline)
        return;

    d->isOnline = on;
    emit d->onlineChanged(isOnline());
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

bool DVRCamera::parseXML(QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("device"));

    QString name;
    d->ptzProtocol = UnknownProtocol;

    while (xml.readNext() != QXmlStreamReader::Invalid)
    {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("device"))
            break;
        else if (xml.tokenType() != QXmlStreamReader::StartElement)
            continue;

        if (xml.name() == QLatin1String("device_name"))
        {
            name = xml.readElementText();
        }
        else if (xml.name() == QLatin1String("ptz_control_protocol"))
        {
            d->ptzProtocol = parseProtocol(xml.readElementText());
        }
        else if (xml.name() == QLatin1String("disabled"))
        {
            bool ok = false;
            d->isDisabled = xml.readElementText().toInt(&ok);
            if (!ok)
                d->isDisabled = false;
        }
        else
            xml.skipCurrentElement();
    }

    if (name.isEmpty())
        name = QString::fromLatin1("#%2").arg(uniqueId());

    d->displayName = name;
    QUrl url;
    url.setScheme(QLatin1String("rtsp"));
    url.setUserName(server()->configuration()->username());
    url.setPassword(server()->configuration()->password());
    url.setHost(server()->url().host());
    url.setPort(server()->rtspPort());
    url.setPath(QString::fromLatin1("live/") + QString::number(d->uniqueID));
    d->streamUrl = url.toString().toLatin1();
    d->isLoaded = true;

    d->doDataUpdated();
    /* Changing stream URL or disabled flag will change online state */
    emit d->onlineChanged(isOnline());
    return true;
}

LiveStream * DVRCamera::liveStream()
{
    if (!d->liveStream)
    {
        LiveStream * re = new LiveStream(this);
        QObject::connect(d.data(), SIGNAL(onlineChanged(bool)), re, SLOT(setOnline(bool)));
        re->setOnline(isOnline());
        d->liveStream = re;
    }

    d->liveStream.data();
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
