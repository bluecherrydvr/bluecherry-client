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

#include "EventData.h"
#include "core/DVRServer.h"
#include "core/DVRCamera.h"
#include "utils/FileUtils.h"
#include <QApplication>
#include <QXmlStreamReader>
#include <QDebug>

QString EventLevel::uiString() const
{
    switch (level)
    {
    case Info: return QApplication::translate("EventLevel", "Info");
    case Warning: return QApplication::translate("EventLevel", "Warning");
    case Alarm: return QApplication::translate("EventLevel", "Alarm");
    case Critical: return QApplication::translate("EventLevel", "Critical");
    default: return QApplication::translate("EventLevel", "Unknown");
    }
}

QColor EventLevel::uiColor(bool graphical) const
{
    switch (level)
    {
    case Info: return QColor(122, 122, 122);
    case Warning: return graphical ? QColor(62, 107, 199) : QColor(Qt::black);
    case Alarm: return QColor(204, 120, 10);
    case Critical: return QColor(175, 0, 0);
    default: return QColor(Qt::black);
    }
}

EventLevel &EventLevel::operator=(const QString &str)
{
    if (str == QLatin1String("info"))
        level = Info;
    else if (str == QLatin1String("warn"))
        level = Warning;
    else if (str == QLatin1String("alrm") || str == QLatin1String("alarm"))
        level = Alarm;
    else if (str == QLatin1String("critical"))
        level = Critical;
    else
        level = Info;

    return *this;
}

QString EventType::uiString() const
{
    switch (type)
    {
    case CameraMotion: return QApplication::translate("EventType", "Motion");
    case CameraContinuous: return QApplication::translate("EventType", "Continuous");
    case CameraNotFound: return QApplication::translate("EventType", "Not Found");
    case CameraVideoLost: return QApplication::translate("EventType", "Video Lost");
    case CameraAudioLost: return QApplication::translate("EventType", "Audio Lost");
    case SystemDiskSpace: return QApplication::translate("EventType", "Disk Space");
    case SystemCrash: return QApplication::translate("EventType", "Crash");
    case SystemBoot: return QApplication::translate("EventType", "Startup");
    case SystemShutdown: return QApplication::translate("EventType", "Shutdown");
    case SystemReboot: return QApplication::translate("EventType", "Reboot");
    case SystemPowerOutage: return QApplication::translate("EventType", "Power Lost");
    case UnknownType:
    default:
        return QApplication::translate("EventType", "Unknown");
    }
}

EventType &EventType::operator=(const QString &str)
{
    if (str == QLatin1String("motion"))
        type = CameraMotion;
    else if (str == QLatin1String("continuous"))
        type = CameraContinuous;
    else if (str == QLatin1String("not found"))
        type = CameraNotFound;
    else if (str == QLatin1String("video signal loss"))
        type = CameraVideoLost;
    else if (str == QLatin1String("audio signal loss"))
        type = CameraAudioLost;
    else if (str == QLatin1String("disk-space"))
        type = SystemDiskSpace;
    else if (str == QLatin1String("crash"))
        type = SystemCrash;
    else if (str == QLatin1String("boot"))
        type = SystemBoot;
    else if (str == QLatin1String("shutdown"))
        type = SystemShutdown;
    else if (str == QLatin1String("reboot"))
        type = SystemReboot;
    else if (str == QLatin1String("power-outage"))
        type = SystemPowerOutage;
    else
        type = UnknownType;

    return *this;
}

QDateTime EventData::utcEndDate() const
{
    return m_utcStartDate.addSecs(qMax(0, m_durationInSeconds));
}

QDateTime EventData::serverStartDate() const
{
    Q_ASSERT(m_utcStartDate.timeSpec() == Qt::UTC);

    int dateTzOffsetSeconds = int(dateTzOffsetMins()) * 60;
    QDateTime result = m_utcStartDate.addSecs(dateTzOffsetSeconds);
    result.setUtcOffset(dateTzOffsetSeconds);
    return result;
}

QDateTime EventData::serverEndDate() const
{
    if (!hasDuration())
        return serverStartDate();

    int dateTzOffsetSeconds = int(dateTzOffsetMins()) * 60;
    QDateTime result = utcEndDate().addSecs(dateTzOffsetSeconds);
    result.setUtcOffset(dateTzOffsetSeconds);
    return result;
}

void EventData::setUtcStartDate(const QDateTime utcStartDate)
{
    m_utcStartDate = utcStartDate;
}

bool EventData::hasDuration() const
{
    return durationInSeconds() > 0;
}

void EventData::setDurationInSeconds(int durationInSeconds)
{
    m_durationInSeconds = durationInSeconds;
}

bool EventData::inProgress() const
{
    return durationInSeconds() < 0;
}

void EventData::setInProgress()
{
    setDurationInSeconds(-1);
}

void EventData::setLocationId(int locationId)
{
    m_locationId = locationId;
}

void EventData::setLevel(EventLevel level)
{
    m_level = level;
}

void EventData::setType(EventType type)
{
    m_type = type;
}

void EventData::setEventId(qint64 eventId)
{
    m_eventId = eventId;
}

void EventData::setMediaId(qint64 mediaId)
{
    m_mediaId = mediaId;
}

void EventData::setDateTzOffsetMins(qint16 dateTzOffsetMins)
{
    m_dateTzOffsetMins = dateTzOffsetMins;
}

void EventData::setLocation(const QString &location)
{
    if (location.startsWith(QLatin1String("camera-")))
    {
        bool ok = false;
        m_locationId = location.mid(7).toUInt(&ok);

        if (!ok)
        {
            qWarning() << "Invalid event location" << location;
            m_locationId = -1;
        }
    }
    else if (location != QLatin1String("system"))
    {
        qWarning() << "Invalid event location" << location;
        m_locationId = -1;
    }
    else
        m_locationId = -1;
}

QString EventData::uiServer() const
{
    return server()->displayName();
}

DVRCamera EventData::locationCamera(DVRServer *server, int locationId)
{
    if (locationId >= 0)
        return DVRCamera::getCamera(server, locationId);
    return DVRCamera();
}

QString EventData::uiLocation(DVRServer *server, int locationId)
{
    const DVRCamera &camera = locationCamera(server, locationId);
    if (camera)
        return camera.displayName();
    else if (locationId < 0)
        return QApplication::translate("EventData", "System");
    else
        return QString::fromLatin1("camera-%1").arg(locationId);
}

QString EventData::baseFileName() const
{
    QString fileName = QString::fromLatin1("%1.%2.%3")
        .arg(uiServer())
        .arg(uiLocation())
        .arg(utcStartDate().toString(QLatin1String("yyyy-MM-dd hh-mm-ss")));

    return sanitizeFilename(fileName);
}

/* XXX not properly translatable right now */
static inline void durationWord(QString &s, int n, const char *w)
{
    if (!s.isEmpty())
        s.append(QLatin1String(", "));
    s.append(QString::number(n));
    s.append(QLatin1Char(' '));
    s.append(QLatin1String(w));
    if (n != 1)
        s.append(QLatin1Char('s'));
}

QString EventData::uiDuration() const
{
    if (inProgress())
        return QApplication::translate("EventData", "In progress");

    QString re;
    int d = qMax(1, durationInSeconds()), count = 0;

    if (d >= (60*60*24))
    {
        durationWord(re, d / (60*60*24), "day");
        d %= (60*60*24);
        ++count;
    }
    if (d >= (60*60))
    {
        durationWord(re, d / (60*60), "hour");
        d %= (60*60);
        if (++count == 2)
            return re;
    }
    if (d >= 60)
    {
        durationWord(re, d / 60, "minute");
        d %= 60;
        if (++count == 2)
            return re;
    }
    if (d || re.isEmpty())
        durationWord(re, d, "second");

    return re;
}

/* May be threaded; avoid dereferencing the server and so forth */

static EventData *parseEntry(DVRServer *server, QXmlStreamReader &reader);

QList<EventData*> EventData::parseEvents(DVRServer *server, const QByteArray &input)
{
    QXmlStreamReader reader(input);
    QList<EventData*> re;

    if (!reader.hasError() && reader.readNextStartElement())
    {
        if (reader.name() == QLatin1String("feed"))
        {
            while (reader.readNext() != QXmlStreamReader::Invalid)
            {
                if (reader.tokenType() == QXmlStreamReader::EndDocument)
                    break;
                if (reader.tokenType() != QXmlStreamReader::StartElement)
                    continue;

                if (reader.name() == QLatin1String("entry"))
                {
                    EventData *ev = parseEntry(server, reader);
                    if (ev)
                        re.append(ev);
                }
            }
        }
        else
            reader.raiseError(QLatin1String("Invalid feed format"));
    }

    if (reader.hasError())
    {
        qWarning() << "EventData::parseEvents error:" << reader.errorString();
    }

    return re;
}

static QDateTime isoToDateTime(const QString &str, qint16 *tzOffsetMins = 0)
{
    /* If there is a - or a + after the T portion, it indicates timezone, and should be removed */
    int tzpos = -1;
    for (int i = str.size()-1; i && tzpos == -1; --i)
    {
        switch (str[i].toLatin1())
        {
        case '-':
        case '+':
            tzpos = i;
            break;
        case 'T':
        case 'Z':
            tzpos = -2;
            break;
        }
    }

    qint16 offset = 0;
    if (tzpos > 0)
    {
        /* Starting from tzpos, this can be in any of the following formats:
         *   +hh   +hh:mm   +hhmm   + may also be -                         */
        int p = tzpos;
        bool positive = (str[p] == QLatin1Char('+'));
        p++;

        offset = (qint16)str.mid(p, 2).toUShort() * 60;
        p += 2;

        if (p < str.size() && str[p] == QLatin1Char(':'))
            ++p;
        offset += (qint16)str.mid(p, 2).toUShort();

        if (!positive)
            offset = -offset;
    }

    if (tzOffsetMins)
        *tzOffsetMins = offset;

    QDateTime re = QDateTime::fromString(str.mid(0, tzpos), Qt::ISODate);
    re.setTimeSpec(Qt::UTC);
    return re.addSecs(int(-offset)*60);
}

static EventData *parseEntry(DVRServer *server, QXmlStreamReader &reader)
{
    Q_ASSERT(reader.isStartElement() && reader.name() == QLatin1String("entry"));

    EventData *data = new EventData(server);

    while (reader.readNext() != QXmlStreamReader::Invalid)
    {
        if (reader.tokenType() == QXmlStreamReader::EndElement)
        {
            if (reader.name() == QLatin1String("entry"))
                break;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement)
            continue;

        if (reader.name() == QLatin1String("id"))
        {
            bool ok = false;
            qint64 id = reader.attributes().value(QLatin1String("raw")).toString().toLongLong(&ok);
            if (!ok || id < 0)
            {
                reader.raiseError(QLatin1String("Invalid format for id element"));
                continue;
            }

            data->setEventId(id);
        }
        else if (reader.name() == QLatin1String("published"))
        {
            qint16 dateTzOffsetMins;
            data->setUtcStartDate(isoToDateTime(reader.readElementText(), &dateTzOffsetMins));
            data->setDateTzOffsetMins(dateTzOffsetMins);
        }
        else if (reader.name() == QLatin1String("updated"))
        {
            QString d = reader.readElementText();
            if (d.isEmpty())
                data->setInProgress();
            else
                data->setDurationInSeconds(data->utcStartDate().secsTo(isoToDateTime(d)));
        }
        else if (reader.name() == QLatin1String("content"))
        {
            bool ok = false;
            QXmlStreamAttributes attr = reader.attributes();
            if (attr.hasAttribute(QLatin1String("media_id")))
            {
                data->setMediaId(attr.value(QLatin1String("media_id")).toString().toLongLong(&ok));
                if (!ok)
                    data->setMediaId(-1);
            }
        }
        else if (reader.name() == QLatin1String("category"))
        {
            QXmlStreamAttributes attrib = reader.attributes();
            if (attrib.value(QLatin1String("scheme")) == QLatin1String("http://www.bluecherrydvr.com/atom.html"))
            {
                QStringRef category = attrib.value(QLatin1String("term"));
                QStringList cd = category.toString().split(QLatin1Char('/'));
                if (cd.size() != 3)
                {
                    reader.raiseError(QLatin1String("Invalid format for category element"));
                    continue;
                }

                data->setLocationId(cd[0].toInt());
                data->setLevel(cd[1]);
                data->setType(cd[2]);
            }
        }
        else if (reader.name() == QLatin1String("entry"))
            reader.raiseError(QLatin1String("Unexpected <entry> element"));
    }

    if (!reader.hasError() && (data->eventId() < 0 || !data->utcStartDate().isValid()))
        reader.raiseError(QLatin1String("Missing required elements for entry"));

    return data;
}
