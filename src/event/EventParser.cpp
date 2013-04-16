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

#include "core/EventData.h"
#include "utils/DateTimeUtils.h"
#include "EventParser.h"
#include <QDebug>
#include <QLatin1String>
#include <QXmlStreamReader>

/* May be threaded; avoid dereferencing the server and so forth */

QList<EventData *> EventParser::parseEvents(DVRServer *server, const QByteArray &input)
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

EventData * EventParser::parseEntry(DVRServer *server, QXmlStreamReader &reader)
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
