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

#ifndef EVENTSLOADER_H
#define EVENTSLOADER_H

#include <QDateTime>
#include <QObject>

class DVRServer;
class EventData;

class EventsLoader : public QObject
{
    Q_OBJECT

public:
    explicit EventsLoader(DVRServer *server, QObject *parent = 0);
    virtual ~EventsLoader();

    DVRServer * server() const { return m_server.data(); }

    void setLimit(int limit);
    void setStartTime(const QDateTime &startTime);
    void setEndTime(const QDateTime &endTime);

    void loadEvents();

signals:
    void eventsLoaded(DVRServer *server, bool ok, const QList<EventData *> &events);

private slots:
    void serverRequestFinished();
    void eventParseFinished();

private:
    QWeakPointer<DVRServer> m_server;
    int m_limit;
    QDateTime m_startTime;
    QDateTime m_endTime;

};

#endif // EVENTSLOADER_H
