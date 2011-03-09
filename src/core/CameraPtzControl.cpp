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

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 401)
    {
        qWarning() << "PTZ query failed with HTTP 401";
        return;
    }
    else if (status == 403)
    {
        m_protocol = NoPtz;
        m_capabilities = NoCapabilities;
        emit infoUpdated();
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "PTZ query failed:" << reply->errorString();
        return;
    }

    QByteArray data = reply->readAll();
    QXmlStreamReader xml(data);

    status = -1;
    QString error;

    if (xml.hasError() || !xml.readNextStartElement() || xml.name() != QLatin1String("response"))
    {
        status = 0;
        error = QLatin1String("invalid command response");
    }
    else
    {
        QStringRef s = xml.attributes().value(QLatin1String("status"));
        if (s == QLatin1String("OK"))
        {
            status = 1;
        }
        else
        {
            status = 0;
            if (s != QLatin1String("ERROR"))
                error = QLatin1String("invalid command status");
            else if (!xml.readNextStartElement() || xml.name() != QLatin1String("error"))
                error = QLatin1String("unknown error");
            else
                error = xml.readElementText();
        }
    }

    if (status != 1)
    {
        qDebug() << "PTZ query error:" << error;
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
    qDebug() << "PTZ move:" << movements;
}

void CameraPtzControl::cancel()
{

}
