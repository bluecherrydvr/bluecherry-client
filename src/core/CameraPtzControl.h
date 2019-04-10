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

#ifndef CAMERAPTZCONTROL_H
#define CAMERAPTZCONTROL_H

#include "camera/DVRCamera.h"
#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QUrl>

class QNetworkReply;

class CameraPtzControl : public QObject
{
    Q_OBJECT
    Q_FLAGS(Capability Capabilities)
    Q_FLAGS(Movement Movements)

    Q_PROPERTY(DVRCamera *camera READ camera)
    Q_PROPERTY(int protocol READ protocol NOTIFY infoUpdated)
    Q_PROPERTY(Capabilities capabilities READ capabilities NOTIFY infoUpdated)
    Q_PROPERTY(bool hasPendingActions READ hasPendingActions NOTIFY hasPendingActionsChanged)
    /* May be -1, indicating that we're not currently on a preset (or don't know that we are) */
    Q_PROPERTY(int currentPreset READ currentPreset WRITE moveToPreset NOTIFY currentPresetChanged)
    Q_PROPERTY(QString currentPresetName READ currentPresetName NOTIFY currentPresetChanged)

public:
    enum Capability {
        NoCapabilities = 0,
        CanPan = 1 << 0,
        CanTilt = 1 << 1,
        CanZoom = 1 << 2
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    enum Movement {
        NoMovement = 0,
        MoveNorth = 1 << 0,
        MoveSouth = 1 << 1,
        MoveWest = 1 << 2,
        MoveEast = 1 << 3,
        MoveWide = 1 << 4,
        MoveTele = 1 << 5
    };
    Q_DECLARE_FLAGS(Movements, Movement)

    explicit CameraPtzControl(DVRCamera *camera, QObject *parent = 0);
    virtual ~CameraPtzControl();

    DVRCamera *camera() const { return m_camera.data(); }
    DVRCamera::PtzProtocol protocol() const { return m_protocol; }
    Capabilities capabilities() const { return m_capabilities; }

    bool isReady() const;
    bool hasPendingActions() const { return !m_pendingCommands.isEmpty(); }

    int currentPreset() const { return m_currentPreset; }
    QString currentPresetName() const { return m_presets.value(m_currentPreset); }

    const QMap<int,QString> &presets() const { return m_presets; }
    int nextPresetID() const;

public slots:
    void updateInfo();
    void move(Movements movements, int panSpeed = -1, int tiltSpeed = -1, int duration = -1);
    void moveToPreset(int preset);
    /* preset ID may be -1 to automatically use the next available ID, which is returned */
    int savePreset(int preset, const QString &name);
    void updatePreset(int preset);
    void renamePreset(int preset, const QString &name);
    void clearPreset(int preset);
    void cancel(QNetworkReply *command);
    void cancelAll();

signals:
    void infoUpdated();
    void currentPresetChanged(int currentPreset);
    void hasPendingActionsChanged(bool hasPendingActions);

private slots:
    void finishCommand();
    void queryResult();
    void moveResult();
    void moveToPresetResult();
    void savePresetResult();
    void clearPresetResult();

private:
    QWeakPointer<DVRCamera> m_camera;
    QMap<int,QString> m_presets;
    QList<QNetworkReply*> m_pendingCommands;
    DVRCamera::PtzProtocol m_protocol;
    Capabilities m_capabilities;
    int m_currentPreset;

    QString validatePresetName(const QString &name) const;

    QNetworkReply *sendCommand(const QUrl &partialUrl);
    bool parseResponse(QNetworkReply *reply, QXmlStreamReader &xml, QString &errorMessage);
    bool parsePresetResponse(QXmlStreamReader &xml, int &presetId, QString &presetName);
};

#endif // CAMERAPTZCONTROL_H
