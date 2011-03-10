#ifndef CAMERAPTZCONTROL_H
#define CAMERAPTZCONTROL_H

#include <QObject>
#include "DVRCamera.h"

class QNetworkReply;

class CameraPtzControl : public QObject
{
    Q_OBJECT
    Q_ENUMS(PtzProtocol)
    Q_FLAGS(Capability Capabilities)
    Q_FLAGS(Movement Movements)

    Q_PROPERTY(DVRCamera camera READ camera)
    Q_PROPERTY(PtzProtocol protocol READ protocol NOTIFY infoUpdated)
    Q_PROPERTY(Capabilities capabilities READ capabilities NOTIFY infoUpdated)
    Q_PROPERTY(Movements pendingMovements READ pendingMovements NOTIFY pendingMovementsChanged)

public:
    enum PtzProtocol {
        UnknownProtocol = -1,
        NoPtz,
        BasicPtz
    };

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

    explicit CameraPtzControl(const DVRCamera &camera, QObject *parent = 0);

    const DVRCamera &camera() const { return m_camera; }
    PtzProtocol protocol() const { return m_protocol; }
    Capabilities capabilities() const { return m_capabilities; }

    bool isReady() const;
    bool hasPendingActions() const;
    Movements pendingMovements() const;

public slots:
    void move(Movements movements, int panSpeed = -1, int tiltSpeed = -1, int duration = -1);
    void moveToPreset(int preset);
    void savePreset(int preset, const QString &name);
    void clearPreset(int preset);
    void cancel();

signals:
    void infoUpdated();
    void pendingMovementsChanged(Movements pendingMovements);

private slots:
    void queryResult();
    void moveResult();

private:
    DVRCamera m_camera;
    PtzProtocol m_protocol;
    Capabilities m_capabilities;

    void sendQuery();
    bool parseResponse(QNetworkReply *reply, QXmlStreamReader &xml, QString &errorMessage);
};

#endif // CAMERAPTZCONTROL_H
