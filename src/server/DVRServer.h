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

#ifndef DVRSERVER_H
#define DVRSERVER_H

#include <QObject>
#include <QVariant>
#include <QTimer>
#include "core/ServerRequestManager.h"
#include "core/DVRCamera.h"

class QNetworkRequest;
class QUrl;
class QSslCertificate;

class DVRServer : public QObject
{
    Q_OBJECT

public:
    ServerRequestManager *api;
    const int configId;

    explicit DVRServer(int configId, QObject *parent = 0);

    const QString &displayName() const { return m_displayName; }
    void setDisplayName(const QString &displayName);

    QString hostname() const;
    int serverPort() const;
    int rtspPort() const;
    QString username() const;
    QString password() const;
    bool autoConnect() const;

    QList<DVRCamera> cameras() const { return m_cameras; }
    DVRCamera findCamera(int id) { return DVRCamera::getCamera(this, id); }

    QString statusAlertMessage() const { return m_statusAlertMessage; }

    /* Settings */
    void writeSetting(const char *key, const QVariant &value) { writeSetting(QLatin1String(key), value); }
    void writeSetting(const QString &key, const QVariant &value);

    void clearSetting(const char *key) { clearSetting(QLatin1String(key)); }
    void clearSetting(const QString &key);

    /* SSL */
    bool isKnownCertificate(const QSslCertificate &certificate) const;
    void setKnownCertificate(const QSslCertificate &certificate);

public slots:
    /* Permanently remove from config and delete */
    void removeServer();

    void login();
    void toggleOnline();
    void updateCameras();

signals:
    void changed();
    void devicesReady();

    void serverRemoved(DVRServer *server);

    void cameraAdded(const DVRCamera &camera);
    void cameraRemoved(const DVRCamera &camera);

    void statusAlertMessageChanged(const QString &message);

private slots:
    void updateCamerasReply();
    void updateStatsReply();
    void disconnected();

private:
    QList<DVRCamera> m_cameras;
    QString m_displayName;
    QString m_statusAlertMessage;
    QTimer m_refreshTimer;
    bool devicesLoaded;

    QVariant readSetting(const char *key, const QVariant &def = QVariant()) const { return readSetting(QLatin1String(key), def); }
    QVariant readSetting(const QString &key, const QVariant &def = QVariant()) const;

};

Q_DECLARE_METATYPE(DVRServer*)

#endif // DVRSERVER_H
