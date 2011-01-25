#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <QString>
#include <QDateTime>
#include <QColor>
#include "DVRCamera.h"

class DVRServer;

class EventLevel
{
public:
    enum Level
    {
        Info = 0,
        Warning,
        Alarm,
        Critical,
        Minimum = Info
    } level : 8;

    EventLevel() : level(Info) { }
    EventLevel(Level l) : level(l) { }
    EventLevel(const QString &l) { *this = l; }

    QString uiString() const;
    QColor uiColor(bool graphical = true) const;

    EventLevel &operator=(const QString &l);
    operator Level() const { return level; }
};

class EventType
{
public:
    enum Type
    {
        UnknownType = -1,
        CameraMotion,
        MinCamera = CameraMotion,
        CameraContinuous,
        CameraNotFound,
        CameraVideoLost,
        CameraAudioLost,
        MaxCamera = CameraAudioLost,
        SystemDiskSpace,
        MinSystem = SystemDiskSpace,
        SystemCrash,
        SystemBoot,
        SystemShutdown,
        SystemReboot,
        SystemPowerOutage,
        MaxSystem = SystemPowerOutage,
        Max = MaxSystem
    } type : 8;

    EventType() : type(UnknownType) { }
    EventType(Type t) : type(t) { }
    EventType(const QString &str) { *this = str; }

    QString uiString() const;

    EventType &operator=(const QString &str);
    operator int() const { return type; }
    operator Type() const { return type; }
};

struct EventData
{
    QDateTime date; /* UTC */
    DVRServer *server;
    qint64 eventId, mediaId;
    int duration;
    int locationId;
    EventLevel level;
    EventType type;
    qint16 dateTzOffsetMins; /* Offset in minutes for the server's timezone as of this event */

    EventData(DVRServer *s = 0)
        : server(s), eventId(-1), mediaId(-1), duration(0), locationId(-1), dateTzOffsetMins(0)
    {
    }

    bool operator==(const EventData &o)
    {
        return (o.server == server && o.eventId == eventId);
    }

    void setLocation(const QString &location);

    bool isSystem() const { return locationId < 0; }
    bool isCamera() const { return locationId >= 0; }
    QDateTime endDate() const { return date.addSecs(qMax(0, duration)); }
    QDateTime serverLocalDate() const { return date.addSecs(int(dateTzOffsetMins)*60); }
    bool hasMedia() const { return mediaId >= 0; }

    QColor uiColor(bool graphical = true) const { return level.uiColor(graphical); }
    QString uiLevel() const { return level.uiString(); }
    QString uiType() const { return type.uiString(); }
    QString uiDuration() const;
    QString uiServer() const;
    QString uiLocation() const { return uiLocation(server, locationId); }

    DVRCamera locationCamera() const { return locationCamera(server, locationId); }

    static QString uiLocation(DVRServer *server, int locationId);
    static DVRCamera locationCamera(DVRServer *server, int locationId);

    static QList<EventData*> parseEvents(DVRServer *server, const QByteArray &input);
};

#endif // EVENTDATA_H
