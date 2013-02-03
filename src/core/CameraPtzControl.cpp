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

#include "CameraPtzControl.h"
#include "BluecherryApp.h"
#include "DVRServer.h"
#include <QVariant>
#include <QDebug>
#include <QXmlStreamReader>

Q_DECLARE_METATYPE(QWeakPointer<CameraPtzControl>)

CameraPtzControl::CameraPtzControl(const DVRCamera &camera, QObject *parent)
    : QObject(parent), m_camera(camera), m_protocol(DVRCamera::UnknownProtocol), m_capabilities(NoCapabilities),
      m_currentPreset(-1)
{
    Q_ASSERT(m_camera.isValid());
    updateInfo();
}

QSharedPointer<CameraPtzControl> CameraPtzControl::sharedObjectFor(const DVRCamera &camera)
{
    QSharedPointer<CameraPtzControl> ptr = static_cast<QObject*>(camera)->property("cameraPtzControl")
                                           .value<QWeakPointer<CameraPtzControl> >();
    if (ptr.isNull())
    {
        ptr = QSharedPointer<CameraPtzControl>(new CameraPtzControl(camera));
        static_cast<QObject*>(camera)->setProperty("cameraPtzControl", QVariant::fromValue(ptr.toWeakRef()));
    }

    return ptr;
}

CameraPtzControl::~CameraPtzControl()
{
    foreach (QNetworkReply *r, m_pendingCommands)
    {
        r->disconnect(this);
        r->abort();
        r->deleteLater();
    }
}

QNetworkReply *CameraPtzControl::sendCommand(const QUrl &partialUrl)
{
    QUrl url(QLatin1String("/media/ptz.php"));
    url = url.resolved(partialUrl);
    url.addEncodedQueryItem("id", QByteArray::number(m_camera.uniqueId()));

    Q_ASSERT(url.hasQueryItem(QLatin1String("command")));

    QNetworkReply *reply = m_camera.server()->api->sendRequest(url);
    connect(reply, SIGNAL(finished()), SLOT(finishCommand()));

    m_pendingCommands.append(reply);
    if (m_pendingCommands.size() == 1)
        emit hasPendingActionsChanged(true);

    return reply;
}

void CameraPtzControl::finishCommand()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (m_pendingCommands.removeOne(reply) && m_pendingCommands.isEmpty())
        emit hasPendingActionsChanged(false);
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

void CameraPtzControl::updateInfo()
{
    QNetworkReply *reply = sendCommand(QUrl(QLatin1String("?command=query")));
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

    m_protocol = DVRCamera::UnknownProtocol;
    m_capabilities = NoCapabilities;
    m_presets.clear();

    while (xml.readNext() != QXmlStreamReader::Invalid)
    {
        if (xml.tokenType() == QXmlStreamReader::EndElement
            && xml.name() == QLatin1String("response"))
            break;

        if (xml.tokenType() != QXmlStreamReader::StartElement)
            continue;

        if (xml.name() == QLatin1String("protocol"))
        {
            m_protocol = DVRCamera::parseProtocol(xml.readElementText());
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
        else if (xml.name() == QLatin1String("presets"))
        {
            while (xml.readNextStartElement())
            {
                if (xml.name() == QLatin1String("preset"))
                {
                    bool ok = false;
                    int id = xml.attributes().value(QLatin1String("id")).toString().toInt(&ok);
                    if (ok)
                        m_presets.insert(id, xml.readElementText());
                    else
                        qDebug() << "PTZ parsing error: failed to parse <preset> element";
                }
            }
        }
        else
            xml.skipCurrentElement();
    }

    if (xml.hasError())
        qDebug() << "PTZ parsing error:" << xml.errorString();

    emit infoUpdated();
    if (m_currentPreset >= 0)
        emit currentPresetChanged(m_currentPreset);
}

bool CameraPtzControl::parsePresetResponse(QXmlStreamReader &xml, int &presetId, QString &presetName)
{
    presetId = -1;
    presetName.clear();

    while (xml.readNextStartElement())
    {
        if (xml.name() == QLatin1String("preset"))
        {
            bool ok = false;
            presetId = xml.attributes().value(QLatin1String("id")).toString().toInt(&ok);
            if (!ok)
                presetId = -1;
            else
                presetName = xml.readElementText();
            break;
        }
    }

    return (presetId >= 0);
}

void CameraPtzControl::move(Movements movements, int panSpeed, int tiltSpeed, int duration)
{
    if (!movements)
        return;

    QUrl url;
    url.addEncodedQueryItem("command", "move");
    url.addEncodedQueryItem("panspeed", "32");
    url.addEncodedQueryItem("tiltspeed", "32");
    url.addEncodedQueryItem("duration", "250");

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

    QNetworkReply *reply = sendCommand(url);
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
    if (m_currentPreset >= 0)
    {
        m_currentPreset = -1;
        emit currentPresetChanged(m_currentPreset);
    }
}

void CameraPtzControl::moveToPreset(int preset)
{
    QUrl url;
    url.addEncodedQueryItem("command", "go");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));

    QNetworkReply *reply = sendCommand(url);
    connect(reply, SIGNAL(finished()), SLOT(moveToPresetResult()));
}

void CameraPtzControl::moveToPresetResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QXmlStreamReader xml;
    QString error;

    if (!parseResponse(reply, xml, error))
    {
        qWarning() << "PTZ preset move failed:" << error;
        return;
    }

    int presetId = -1;
    QString presetName;

    if (!parsePresetResponse(xml, presetId, presetName))
    {
        qWarning() << "PTZ preset move succeeded, but was not a valid response";
        return;
    }

    m_currentPreset = presetId;
    emit currentPresetChanged(presetId);
}

int CameraPtzControl::nextPresetID() const
{
    QMap<int,QString>::ConstIterator it = m_presets.begin();
    int i = qMin(it != m_presets.end() ? it.key() : 1, 1);

    for (; it != m_presets.end() && it.key() == i; ++it, ++i)
        ;

    return i;
}

int CameraPtzControl::savePreset(int preset, const QString &name)
{
    if (preset < 0)
        preset = nextPresetID();

    QString actualName = name;
    if (m_presets.value(preset) != actualName)
        actualName = validatePresetName(name);

    QUrl url;
    url.addEncodedQueryItem("command", "save");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));
    url.addQueryItem(QLatin1String("name"), actualName);

    QNetworkReply *reply = sendCommand(url);
    connect(reply, SIGNAL(finished()), SLOT(savePresetResult()));

    return preset;
}

void CameraPtzControl::updatePreset(int preset)
{
    QUrl url;
    url.addEncodedQueryItem("command", "sync");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));
    url.addQueryItem(QLatin1String("name"), m_presets.value(preset));

    QNetworkReply *reply = sendCommand(url);
    connect(reply, SIGNAL(finished()), SLOT(savePresetResult()));
}

void CameraPtzControl::savePresetResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QXmlStreamReader xml;
    QString error;

    if (!parseResponse(reply, xml, error))
    {
        qWarning() << "PTZ preset save failed:" << error;
        return;
    }

    int presetId = -1;
    QString presetName;

    if (!parsePresetResponse(xml, presetId, presetName) || presetName.isEmpty())
    {
        qWarning() << "PTZ preset save succeeded, but was not a valid response";
        return;
    }

    qDebug() << "PTZ preset save succeeded for preset" << presetId << ":" << presetName;

    /* It might be a good idea to create this before it succeeds, and handle failure instead.
     * That depends on how the UI ends up working. */
    m_presets.insert(presetId, presetName);
    emit infoUpdated();

    m_currentPreset = presetId;
    emit currentPresetChanged(m_currentPreset);
}

void CameraPtzControl::renamePreset(int preset, const QString &name)
{
    if (!m_presets.contains(preset) || m_presets[preset] == name)
        return;

    QString actualName = validatePresetName(name);

    QUrl url;
    url.addEncodedQueryItem("command", "rename");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));
    url.addQueryItem(QLatin1String("name"), actualName);

    sendCommand(url);

    m_presets[preset] = actualName;
    emit infoUpdated();

    if (preset == m_currentPreset)
        emit currentPresetChanged(m_currentPreset);
}

void CameraPtzControl::clearPreset(int preset)
{
    QUrl url;
    url.addEncodedQueryItem("command", "clear");
    url.addEncodedQueryItem("preset", QByteArray::number(preset));

    QNetworkReply *reply = sendCommand(url);
    connect(reply, SIGNAL(finished()), SLOT(clearPresetResult()));
}

void CameraPtzControl::clearPresetResult()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    QXmlStreamReader xml;
    QString error;

    if (!parseResponse(reply, xml, error))
    {
        qWarning() << "PTZ preset clear failed:" << error;
        return;
    }

    int presetId = -1;
    QString presetName;

    if (!parsePresetResponse(xml, presetId, presetName))
    {
        qWarning() << "PTZ preset clear succeeded, but was not a valid response";
        return;
    }

    if (m_currentPreset == presetId)
    {
        m_currentPreset = -1;
        emit currentPresetChanged(m_currentPreset);
    }

    m_presets.remove(presetId);
    emit infoUpdated();
}

void CameraPtzControl::cancel(QNetworkReply *command)
{
    if (command && m_pendingCommands.removeOne(command))
    {
        command->disconnect(this);
        command->abort();
        command->deleteLater();

        if (m_pendingCommands.isEmpty())
            emit hasPendingActionsChanged(false);
    }
}

void CameraPtzControl::cancelAll()
{
    foreach (QNetworkReply *reply, m_pendingCommands)
    {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }

    m_pendingCommands.clear();
    emit hasPendingActionsChanged(false);
}

QString CameraPtzControl::validatePresetName(const QString &name) const
{
    QList<QString> names = m_presets.values();
    QString re = name;
    int c = 1;
    while (names.contains(re))
        re = tr("%1 (%2)", "e.g.: Name (2)").arg(name).arg(++c);
    return re;
}
