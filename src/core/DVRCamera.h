#ifndef DVRCAMERA_H
#define DVRCAMERA_H

#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QHash>
#include <QXmlStreamReader>
#include <QMetaType>

class DVRServer;
class LiveStream;
class QMimeData;

/* There is one DVRCameraData per server+ID; it is shared among many instances of
 * DVRCamera by reference count. This may be created before we've actually queried
 * the server for cameras (for example, with saved camera layouts in the config).
 * Once real data is available, this object will be updated and dataUpdated will be
 * emitted. The DVRServer holds a reference to every camera that currently exists
 * according to the server. */
class DVRCameraData : public QObject, public QSharedData
{
    Q_OBJECT

    friend class DVRCamera;

public:
    DVRServer * const server;
    const int uniqueID;
    QString displayName;
    QByteArray streamUrl;
    QWeakPointer<LiveStream> liveStream;
    bool isLoaded, isOnline, isDisabled;
    qint8 ptzProtocol;
    qint8 recordingState;

    DVRCameraData(DVRServer *server, int uniqueID);
    virtual ~DVRCameraData();

    void loadSavedSettings();
    void doDataUpdated();

public slots:
    void setRecordingState(int recordingState);

signals:
    void onlineChanged(bool isOnline);
    void dataUpdated();
    void removed();

    void recordingStateChanged(int recordingState);

private:
    static QHash<QPair<int,int>,DVRCameraData*> instances;
};

class DVRCamera
{
    friend class DVRServer;

public:
    enum PtzProtocol {
        NoPtz,
        UnknownProtocol,
        PelcoPtz
    };

    /* Duplicated in LiveFeedItem for QMetaObject. This is not ideal. */
    enum RecordingState {
        NoRecording = 0,
        Continuous,
        MotionInactive,
        MotionActive
    };

    static PtzProtocol parseProtocol(const QString &protocol);

    static DVRCamera getCamera(int serverID, int cameraID);
    static DVRCamera getCamera(DVRServer *server, int cameraID);

    DVRCamera() { }
    DVRCamera(const DVRCamera &o) : d(o.d) { }

    DVRCamera &operator=(const DVRCamera &o)
    {
        d = o.d;
        return *this;
    }

    bool operator==(const DVRCamera &o) const
    {
        return (d.data() == o.d.data());
    }

    bool isValid() const { return d; }
    operator bool() const { return isValid(); }
    operator QObject*() const { return d ? d.data() : 0; }

    DVRServer *server() const { return d ? d->server : 0; }
    int uniqueId() const { return d ? d->uniqueID : -1; }
    QString displayName() const { return d ? d->displayName : QString(); }
    QByteArray streamUrl() const { return d ? d->streamUrl : QByteArray(); }
    bool isOnline() const { return d && d->isOnline && !d->isDisabled; }
    bool isDisabled() const { return d && d->isDisabled; }
    bool canStream() const { return d && !d->streamUrl.isEmpty() && isOnline(); }
    QSharedPointer<LiveStream> liveStream();

    PtzProtocol ptzProtocol() const { return d ? static_cast<PtzProtocol>(d->ptzProtocol) : NoPtz; }
    bool hasPtz() const { return d ? (d->ptzProtocol > 0) : false; }

    RecordingState recordingState() const { return d ? RecordingState(d->recordingState) : NoRecording; }

    bool parseXML(QXmlStreamReader &xml);

    static QList<DVRCamera> fromMimeData(const QMimeData *mimeData);
    static DVRCamera fromQObject(QObject *o);

private:
    QExplicitlySharedDataPointer<DVRCameraData> d;

    DVRCamera(DVRCameraData *dt) : d(dt) { }

    void removed();
    void setOnline(bool on);
};

Q_DECLARE_METATYPE(DVRCamera)

QDataStream &operator<<(QDataStream &s, const DVRCamera &camera);
QDataStream &operator>>(QDataStream &s, DVRCamera &camera);

inline DVRCamera DVRCamera::fromQObject(QObject *o)
{
    return DVRCamera(qobject_cast<DVRCameraData*>(o));
}

#endif // DVRCAMERA_H
