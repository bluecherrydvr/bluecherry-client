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

#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <QList>

class DVRServer;
class EventData;
class QXmlStreamReader;

class EventParser
{
public:
    static QList<QSharedPointer<EventData> > parseEvents(DVRServer *server, const QByteArray &input);

private:
    static EventData * parseEntry(DVRServer *server, QXmlStreamReader &reader);

};

#endif // EVENTPARSER_H
