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

#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <QString>
#include <QDateTime>
#include <QColor>
#include "camera/DVRCamera.h"

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

class EventData
{
    QDateTime m_utcStartDate;
    DVRServer *m_server;
    qint64 m_eventId;
    qint64 m_mediaId;
    int m_durationInSeconds;
    int m_locationId;
    EventLevel m_level;
    EventType m_type;
    qint16 m_dateTzOffsetMins; /* Offset in minutes for the server's timezone as of this event */

public:
    EventData(DVRServer *s = 0)
        : m_server(s), m_eventId(-1), m_mediaId(-1), m_durationInSeconds(0), m_locationId(-1), m_dateTzOffsetMins(0)
    {
    }

    bool operator==(const EventData &o)
    {
        return (o.m_server == m_server && o.m_eventId == m_eventId);
    }

    QDateTime utcStartDate() const { return m_utcStartDate; }
    QDateTime utcEndDate() const;
    QDateTime serverStartDate() const;
    QDateTime serverEndDate() const;
    void setUtcStartDate(const QDateTime utcStartDate);

    int durationInSeconds() const { return m_durationInSeconds; }
    bool hasDuration() const;
    void setDurationInSeconds(int durationInSeconds);

    bool inProgress() const;
    void setInProgress();

    DVRServer * server() const { return m_server; }

    int locationId() const { return m_locationId; }
    void setLocationId(int locationId);

    EventLevel level() const { return m_level; }
    void setLevel(EventLevel level);

    EventType type() const { return m_type; }
    void setType(EventType type);

    qint64 eventId() const { return m_eventId; }
    void setEventId(qint64 eventId);

    qint64 mediaId() const { return m_mediaId; }
    void setMediaId(qint64 mediaId);

    qint16 dateTzOffsetMins() const { return m_dateTzOffsetMins; }
    void setDateTzOffsetMins(qint16 dateTzOffsetMins);

    void setLocation(const QString &location);

    bool isSystem() const { return locationId() < 0; }
    bool isCamera() const { return locationId() >= 0; }
    bool hasMedia() const { return m_mediaId >= 0; }

    QColor uiColor(bool graphical = true) const { return level().uiColor(graphical); }
    QString uiLevel() const { return level().uiString(); }
    QString uiType() const { return type().uiString(); }
    QString uiDuration() const;
    QString uiServer() const;
    QString uiLocation() const { return uiLocation(server(), locationId()); }

    DVRCamera * locationCamera() const;

    static QString uiLocation(DVRServer *server, int locationId);

    QString baseFileName() const;

    static QList<EventData*> parseEvents(DVRServer *server, const QByteArray &input);
};

#endif // EVENTDATA_H
