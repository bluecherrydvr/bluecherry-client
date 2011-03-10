#include "CameraPtzControl.h"
#include "BluecherryApp.h"
#include "DVRServer.h"
#include <QVariant>
#include <QDebug>
#include <QXmlStreamReader>

CameraPtzControl::CameraPtzControl(const DVRCamera &camera, QObject *parent)
    : QObject(parent), m_camera(camera), m_protocol(UnknownProtocol), m_capabilities(NoCapabilities)
{
    Q_ASSERT(m_camera.isValid());
    sendQuery();
}

CameraPtzControl::Movements CameraPtzControl::pendingMovements() const
{
    return NoMovement;
}

bool CameraPtzControl::hasPendingActions() const
{
    return false;
}

bool CameraPtzControl::parseResponse(QNetworkReply *reply, QXmlStreamReader &xml, QString &errorMessage)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 401)
    {
        errorMessage = QLatin1String("authentication needed");
        return false;
    }
    else if (status == 403)
    {
        errorMessage = QLatin1String("access denied");
        return false;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        errorMessage = reply->errorString();
        return false;
    }

    QByteArray data = reply->readAll();
    xml.addData(data);

    if (xml.hasError() || !xml.readNextStartElement() || xml.name() != QLatin1String("response"))
    {
        errorMessage = QLatin1String("invalid command response");
        return false;
    }
    else
    {
        QStringRef s = xml.attributes().value(QLatin1String("status"));
        if (s != QLatin1String("OK"))
        {
            if (s != QLatin1String("ERROR"))
                errorMessage = QLatin1String("invalid command status");
            else if (!xml.readNextStartElement() || xml.name() != QLatin1String("error"))
                errorMessage = QLatin1String("unknown error");
            else
                errorMessage = xml.readElementText();
            return false;
        }
    }

    return true;
}

void CameraPtzControl::sendQuery()
{
    QUrl url(QLatin1String("/ptz.php"));
    url.addEncodedQueryItem("device", QByteArray::number(m_camera.uniqueId()));
    url.addEncodedQueryItem("command", "query");

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    connect(reply, SIGNAL(finished()), SLOT(queryResult()));
}

void CameraPtzControl::queryResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QXmlStreamReader xml;
    QString error;

    if (!parseResponse(reply, xml, error))
    {
        qWarning() << "PTZ query failed:" << error;
        return;
    }

    m_protocol = UnknownProtocol;
    m_capabilities = NoCapabilities;

    while (xml.readNextStartElement())
    {
        if (xml.name() == QLatin1String("protocol"))
        {
            QString s = xml.readElementText();
            if (s == QLatin1String("none"))
                m_protocol = NoPtz;
            else if (s.contains(QLatin1String("basic")))
                m_protocol = BasicPtz;
        }
        else if (xml.name() == QLatin1String("capabilities"))
        {
            QXmlStreamAttributes attr = xml.attributes();
            if (attr.value(QLatin1String("pan")) == "1")
                m_capabilities |= CanPan;
            if (attr.value(QLatin1String("tilt")) == "1")
                m_capabilities |= CanTilt;
            if (attr.value(QLatin1String("zoom")) == "1")
                m_capabilities |= CanZoom;
        }
        else
            xml.skipCurrentElement();
    }

    if (xml.hasError())
        qDebug() << "PTZ parsing error:" << xml.errorString();

    emit infoUpdated();
}

void CameraPtzControl::move(Movements movements, int panSpeed, int tiltSpeed, int duration)
{
    if (!movements)
        return;

    QUrl url(QLatin1String("/ptz.php"));
    url.addEncodedQueryItem("device", QByteArray::number(m_camera.uniqueId()));
    url.addEncodedQueryItem("command", "move");

    if (movements & MoveNorth)
        url.addEncodedQueryItem("tilt", "u");
    else if (movements & MoveSouth)
        url.addEncodedQueryItem("tilt", "d");
    if (movements & MoveWest)
        url.addEncodedQueryItem("pan", "l");
    else if (movements & MoveEast)
        url.addEncodedQueryItem("pan", "r");
    if (movements & MoveWide)
        url.addEncodedQueryItem("zoom", "w");
    else if (movements & MoveTele)
        url.addEncodedQueryItem("zoom" ,"t");

    if (panSpeed > 0)
        url.addEncodedQueryItem("panspeed", QByteArray::number(panSpeed));
    if (tiltSpeed > 0)
        url.addEncodedQueryItem("tiltspeed", QByteArray::number(tiltSpeed));
    if (duration > 0)
        url.addEncodedQueryItem("duration", QByteArray::number(duration));

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    connect(reply, SIGNAL(finished()), SLOT(moveResult()));
}

void CameraPtzControl::moveResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QXmlStreamReader xml;
    QString error;

    if (!parseResponse(reply, xml, error))
    {
        qWarning() << "PTZ move failed:" << error;
        return;
    }

    qDebug() << "PTZ move succeeded";
}

void CameraPtzControl::moveToPreset(int preset)
{
    QUrl url(QLatin1String("/ptz.php"));
    url.addEncodedQueryItem("device", QByteArray::number(m_camera.uniqueId()));
    url.addEncodedQueryItem("command", "go");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void CameraPtzControl::savePreset(int preset, const QString &name)
{
    QUrl url(QLatin1String("/ptz.php"));
    url.addEncodedQueryItem("device", QByteArray::number(m_camera.uniqueId()));
    url.addEncodedQueryItem("command", "save");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));
    url.addQueryItem(QLatin1String("name"), name);

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void CameraPtzControl::clearPreset(int preset)
{
    QUrl url(QLatin1String("/ptz.php"));
    url.addEncodedQueryItem("device", QByteArray::number(m_camera.uniqueId()));
    url.addEncodedQueryItem("command", "clear");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void CameraPtzControl::cancel()
{

}
