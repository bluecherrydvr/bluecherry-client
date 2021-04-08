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

#include "DVRServer.h"
#include "DVRServerConfiguration.h"
#include "camera/DVRCamera.h"
#include "camera/DVRCameraSettingsReader.h"
#include "camera/DVRCameraSettingsWriter.h"
#include "camera/DVRCameraXMLReader.h"
#include "core/ServerRequestManager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QXmlStreamReader>
#include <QSslCertificate>
#include <QDebug>
#include <QSettings>

bool DVRServer::lessThan(DVRServer *left, DVRServer *right)
{
    Q_ASSERT(left);
    Q_ASSERT(right);

    return QString::localeAwareCompare(left->configuration().displayName(), right->configuration().displayName()) < 0;
}

DVRServer::DVRServer(int id, QObject *parent)
    : QObject(parent), m_configuration(id), m_devicesLoaded(false)
{
    m_api = new ServerRequestManager(this);

    connect(&m_configuration, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(m_api, SIGNAL(loginSuccessful()), SLOT(updateCameras()));
    connect(m_api, SIGNAL(disconnected()), SLOT(disconnectedSlot()));

    connect(m_api, SIGNAL(loginRequestStarted()), this, SIGNAL(loginRequestStarted()));
    connect(m_api, SIGNAL(loginSuccessful()), this, SLOT(loginSuccessfulSlot()));
    connect(m_api, SIGNAL(serverError(QString)), this, SIGNAL(serverError(QString)));
    connect(m_api, SIGNAL(loginError(QString)), this, SIGNAL(loginError(QString)));
    connect(m_api, SIGNAL(statusChanged(int)), this, SIGNAL(statusChanged(int)));
    connect(m_api, SIGNAL(onlineChanged(bool)), this, SIGNAL(onlineChanged(bool)));

    connect(&m_refreshTimer, SIGNAL(timeout()), SLOT(updateCameras()));
}

DVRServer::~DVRServer()
{
    qDeleteAll(m_allCameras);
}

void DVRServer::removeServer()
{
    qDebug("Deleting DVR server %d", m_configuration.id());

    emit serverRemoved(this);

    QSettings settings;
    settings.remove(QString::fromLatin1("servers/%1").arg(m_configuration.id()));

    deleteLater();
}

void DVRServer::login()
{
    m_api->login(m_configuration.username(), m_configuration.password());
}

void DVRServer::toggleOnline()
{
    if (isOnline())
        m_api->logout();
    else
        login();
}

void DVRServer::updateCameras()
{
    if (!isOnline())
    {
        m_refreshTimer.stop();
        return;
    }

    if (!m_refreshTimer.isActive())
        m_refreshTimer.start(60000);

    qDebug() << "DVRServer: Requesting cameras list";
    QNetworkReply *reply = m_api->sendRequest(QUrl(QLatin1String("/ajax/devices.php?XML=1")));
    connect(reply, SIGNAL(finished()), SLOT(updateCamerasReply()));

    reply = m_api->sendRequest(QUrl(QLatin1String("/ajax/stats.php")));
    connect(reply, SIGNAL(finished()), SLOT(updateStatsReply()));
}

void DVRServer::updateCamerasReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    qDebug() << "DVRServer: Received cameras list reply";

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        /* TODO: Handle this well */
        qWarning() << "DVRServer: Error from updating cameras:" << reply->errorString();
        return;
    }

    QByteArray data = reply->readAll();
    QXmlStreamReader xml(data);

    QSet<int> idSet;
    bool hasDevicesElement = false;
    bool wasEmpty = m_visibleCameras.isEmpty();

    while (xml.readNextStartElement())
    {
        if (xml.name() == QLatin1String("devices"))
        {
            hasDevicesElement = true;

            while (xml.readNext() != QXmlStreamReader::Invalid)
            {
                if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("devices"))
                    break;
                else if (xml.tokenType() != QXmlStreamReader::StartElement)
                    continue;

                if (xml.name() == QLatin1String("device"))
                {
                    bool ok = false;
                    QString idv = xml.attributes().value(QLatin1String("id")).toString();
                    if (idv.isNull())
                        continue;
                    int deviceId = (int)idv.toUInt(&ok);
                    if (!ok)
                    {
                        xml.raiseError(QLatin1String("Invalid device ID"));
                        continue;
                    }

                    idSet.insert(deviceId);
                    DVRCamera *camera = getCamera(deviceId);

                    if (camera)
                    {
                        camera->setOnline(true);

                        DVRCameraXMLReader xmlReader;
                        if (!xmlReader.readCamera(camera, xml))
                        {
                            if (!xml.hasError())
                                xml.raiseError(QLatin1String("Device parsing failed"));
                            continue;
                        }
                        else
                        {
                            DVRCameraSettingsWriter settingsWriter;
                            settingsWriter.writeCamera(camera);
                        }
                    }

                    if (!m_visibleCameras.contains(camera))
                    {
                        emit cameraAboutToBeAdded(camera);
                        m_visibleCameras.append(camera);
                        emit cameraAdded(camera);
                    }
                }
            }
            break;
        }
        else
            xml.skipCurrentElement();
    }

    if (!hasDevicesElement)
        xml.raiseError(QLatin1String("Invalid format: no devices element"));

    if (xml.hasError())
    {
        qWarning() << "DVRServer: Error while parsing camera list:" << xml.errorString();
        return;
    }

    for (int i = 0; i < m_visibleCameras.size(); ++i)
    {
        if (!idSet.contains(m_visibleCameras[i]->data().id()))
        {
            DVRCamera *c = m_visibleCameras[i];
            emit cameraAboutToBeRemoved(c);
            m_visibleCameras.removeAt(i);
            m_camerasMap.remove(c->data().id());
            qDebug("DVRServer: camera %d removed", c->data().id());
            emit cameraRemoved(c);
            --i;

            delete c;
        }
    }

    if (!m_devicesLoaded || (wasEmpty && !m_visibleCameras.isEmpty()))
    {
        m_devicesLoaded = true;
        emit devicesReady();
    }
}

void DVRServer::updateStatsReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    QString message;

    if (reply->error() != QNetworkReply::NoError)
    {
        message = tr("Status request error: %1").arg(reply->errorString());
    }
    else
    {
        QByteArray data = reply->readAll();
        QXmlStreamReader xml(data);

        bool hadMessageElement = false;
        while (xml.readNextStartElement())
        {
            if (xml.name() == QLatin1String("stats"))
            {
                while (xml.readNext() != QXmlStreamReader::Invalid)
                {
                    if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("stats"))
                        break;
                    else if (xml.tokenType() != QXmlStreamReader::StartElement)
                        continue;

                    if (xml.name() == QLatin1String("message"))
                    {
                        hadMessageElement = true;
                        QString text = xml.readElementText();
                        if (!text.isEmpty())
                            message = text;
                    }
                    else if (xml.name() == QLatin1String("bc-server-running") &&
                             xml.readElementText().trimmed() == QLatin1String("down"))
                        message = tr("Server process stopped");
                }
            }
        }

        if (!hadMessageElement)
            message = tr("Status request error: invalid server response");
    }

    if (message != m_statusAlertMessage) {
        m_statusAlertMessage = message;
        emit statusAlertMessageChanged(message);
    }
}

void DVRServer::disconnectedSlot()
{
    while (!m_visibleCameras.isEmpty())
    {
        DVRCamera *c = m_visibleCameras.first();
        emit cameraAboutToBeRemoved(c);
        m_visibleCameras.takeFirst();
        c->setOnline(false);
        emit cameraRemoved(c);
    }

    m_devicesLoaded = false;
    m_statusAlertMessage.clear();
    emit statusAlertMessageChanged(QString());

    emit disconnected(this);
}

void DVRServer::loginSuccessfulSlot()
{
    emit loginSuccessful(this);
}

bool DVRServer::isKnownCertificate(const QSslCertificate &certificate) const
{
    if (m_configuration.sslDigest().isEmpty())
    {
        /* If we don't know a certificate yet, we treat the first one we see as
         * correct. This is insecure, obviously, but it's a much nicer way to behave
         * for what we're doing here. */
        const_cast<DVRServer*>(this)->setKnownCertificate(certificate);
        return true;
    }

    return (certificate.digest(QCryptographicHash::Sha1) == m_configuration.sslDigest());
}

void DVRServer::setKnownCertificate(const QSslCertificate &certificate)
{
    m_configuration.setSslDigest(certificate.digest(QCryptographicHash::Sha1));
}

bool DVRServer::isOnline() const
{
    return m_api->isOnline();
}

bool DVRServer::isLoginPending() const
{
    return m_api->isLoginPending();
}

DVRServerConfiguration & DVRServer::configuration()
{
    return m_configuration;
}

QUrl DVRServer::url() const
{
    return m_api->serverUrl();
}

int DVRServer::serverPort() const
{
    return m_configuration.port();
}

int DVRServer::rtspPort() const
{
    return m_configuration.port() + 1;
}

void DVRServer::setError(const QString &error)
{
    m_api->setError(error);
}

QString DVRServer::errorMessage() const
{
    return m_api->errorMessage();
}

DVRServer::Status DVRServer::status() const
{
    return m_api->status();
}

void DVRServer::switchSubstream(int device_id, bool substream_enabled)
{
    m_api->switchSubstream(device_id, substream_enabled);
}

QNetworkReply * DVRServer::sendRequest(const QUrl &relativeUrl)
{
    return m_api->sendRequest(relativeUrl);
}

DVRCamera * DVRServer::getCamera(int cameraId)
{
    if (cameraId < 0)
        return 0;

    if (!m_camerasMap.contains(cameraId))
    {
        DVRCameraSettingsReader settingsReader;
        DVRCamera *camera = settingsReader.readCamera(cameraId, this);
        m_allCameras.append(camera);
        m_camerasMap.insert(cameraId, camera);
    }

    return m_camerasMap.value(cameraId);
}
