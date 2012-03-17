#include "DVRCamera.h"
#include "DVRServer.h"
#include "BluecherryApp.h"
#include "MJpegStream.h"
#include "LiveStream.h"
#include <QXmlStreamReader>
#include <QMimeData>
#include <QSettings>

QHash<QPair<int,int>,DVRCameraData*> DVRCameraData::instances;

DVRCamera DVRCamera::getCamera(int serverID, int cameraID)
{
    DVRServer *server = bcApp->findServerID(serverID);
    if (!server)
        return 0;

    return getCamera(server, cameraID);
}

DVRCamera DVRCamera::getCamera(DVRServer *server, int cameraID)
{
    DVRCameraData *data = DVRCameraData::instances.value(qMakePair(server->configId, cameraID), 0);
    if (!data)
        data = new DVRCameraData(server, cameraID);

    return DVRCamera(data);
}

void DVRCamera::setOnline(bool on)
{
    if (!d || on == d->isOnline)
        return;

    d->isOnline = on;
    emit d->onlineChanged(on);
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
    url.setPort(7002);
    url.setPath(QString::fromLatin1("live/") + QString::number(d->uniqueID));
    d->streamUrl = url.toString().toLatin1();
    d->isLoaded = true;

    d->doDataUpdated();
    return true;
}

QSharedPointer<LiveStream> DVRCamera::liveStream()
{
    QSharedPointer<LiveStream> re;

    if (!d || d->liveStream.isNull())
    {
        if (d && !d->streamUrl.isEmpty())
        {
            re = QSharedPointer<LiveStream>(new LiveStream(*this, d.data()));
            QObject::connect(d.data(), SIGNAL(onlineChanged(bool)), re.data(), SLOT(setOnline(bool)));
            QObject::connect(re.data(), SIGNAL(recordingStateChanged(int)), d.data(), SLOT(setRecordingState(int)));
            d->liveStream = re;
        }
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
    Q_ASSERT(instances.find(qMakePair(s->configId, i)) == instances.end());
    instances.insert(qMakePair(server->configId, uniqueID), this);

    loadSavedSettings();
}

DVRCameraData::~DVRCameraData()
{
    instances.remove(qMakePair(server->configId, uniqueID));
}

void DVRCameraData::loadSavedSettings()
{
    QSettings settings;
    displayName = settings.value(QString::fromLatin1("servers/%1/cameras/%2").arg(server->configId).arg(uniqueID)).toString();
}

void DVRCameraData::doDataUpdated()
{
    if (server)
    {
        QSettings settings;
        settings.beginGroup(QString::fromLatin1("servers/%1/cameras/").arg(server->configId));
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
        s << camera.server()->configId << camera.uniqueId();
    return s;
}

QDataStream &operator>>(QDataStream &s, DVRCamera &camera)
{
    int serverId = -1, cameraId = -1;
    s >> serverId;

    if (s.status() != QDataStream::Ok || serverId < 0)
    {
        camera = DVRCamera();
        return s;
    }

    s >> cameraId;
    camera = DVRCamera::getCamera(serverId, cameraId);
    return s;
}

QList<DVRCamera> DVRCamera::fromMimeData(const QMimeData *mimeData)
{
    QByteArray data = mimeData->data(QLatin1String("application/x-bluecherry-dvrcamera"));
    QDataStream stream(&data, QIODevice::ReadOnly);

    QList<DVRCamera> re;
    while (!stream.atEnd() && stream.status() == QDataStream::Ok)
    {
        DVRCamera c;
        stream >> c;
        if (c)
            re.append(c);
    }

    return re;
}
