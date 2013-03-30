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
#include "BluecherryApp.h"
#include "MJpegStream.h"
#include "LiveStream.h"
#include <QXmlStreamReader>
#include <QMimeData>
#include <QSettings>

QHash<QPair<int,int>,DVRCameraData*> DVRCameraData::instances;

DVRCamera DVRCamera::getCamera(int serverID, int cameraID)
{
    DVRServer *server = bcApp->serverByID(serverID);
    if (!server)
        return 0;

    return getCamera(server, cameraID);
}

DVRCamera DVRCamera::getCamera(DVRServer *server, int cameraID)
{
    DVRCameraData *data = DVRCameraData::instances.value(qMakePair(server->id(), cameraID), 0);
    if (!data)
        data = new DVRCameraData(server, cameraID);

    return DVRCamera(data);
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
    if (!isValid())
        return false;

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
    url.setUserName(server()->username());
    url.setPassword(server()->password());
    url.setHost(server()->api->serverUrl().host());
    url.setPort(server()->rtspPort());
    url.setPath(QString::fromLatin1("live/") + QString::number(d->uniqueID));
    d->streamUrl = url.toString().toLatin1();
    d->isLoaded = true;

    d->doDataUpdated();
    /* Changing stream URL or disabled flag will change online state */
    emit d->onlineChanged(isOnline());
    return true;
}

QSharedPointer<LiveStream> DVRCamera::liveStream()
{
    QSharedPointer<LiveStream> re;
    if (!d)
        return re;

    if (d->liveStream.isNull())
    {
        re = QSharedPointer<LiveStream>(new LiveStream(*this));
        QObject::connect(d.data(), SIGNAL(onlineChanged(bool)), re.data(), SLOT(setOnline(bool)));
        re->setOnline(isOnline());
        d->liveStream = re;
    }
    else
        re = d->liveStream.toStrongRef();

    return re;
}

void DVRCamera::removed()
{
    emit d->removed();
}

DVRCameraData::DVRCameraData(DVRServer *s, int i)
    : server(s), uniqueID(i), isLoaded(false), isOnline(false), isDisabled(false),
      ptzProtocol(DVRCamera::UnknownProtocol), recordingState(DVRCamera::NoRecording)
{
    Q_ASSERT(instances.find(qMakePair(s->id(), i)) == instances.end());
    instances.insert(qMakePair(server->id(), uniqueID), this);

    loadSavedSettings();
}

DVRCameraData::~DVRCameraData()
{
    instances.remove(qMakePair(server->id(), uniqueID));
}

void DVRCameraData::loadSavedSettings()
{
    QSettings settings;
    displayName = settings.value(QString::fromLatin1("servers/%1/cameras/%2").arg(server->id()).arg(uniqueID)).toString();
}

void DVRCameraData::doDataUpdated()
{
    if (server)
    {
        QSettings settings;
        settings.beginGroup(QString::fromLatin1("servers/%1/cameras/").arg(server->id()));
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

QDataStream &operator<<(QDataStream &s, const DVRCamera &camera)
{
    if (!camera.isValid())
        s << -1;
    else
        s << camera.server()->id() << camera.uniqueId();
    return s;
}

QList<DVRCamera> DVRCamera::fromMimeData(const QMimeData *mimeData)
{
    QByteArray data = mimeData->data(QLatin1String("application/x-bluecherry-dvrcamera"));
    QDataStream stream(&data, QIODevice::ReadOnly);
    DVRCameraStreamReader reader(stream);

    QList<DVRCamera> re;
    while (!stream.atEnd() && stream.status() == QDataStream::Ok)
    {
        DVRCamera c = reader.readCamera();
        if (c)
            re.append(c);
    }

    return re;
}
