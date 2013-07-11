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

#ifndef EVENTS_UPDATER_H
#define EVENTS_UPDATER_H

#include <QDateTime>
#include <QObject>
#include <QSet>
#include <QTimer>

class DVRServer;
class DVRServerRepository;
class EventData;

class EventsUpdater : public QObject
{
    Q_OBJECT

public:
    explicit EventsUpdater(DVRServerRepository *serverRepository, QObject *parent = 0);
    virtual ~EventsUpdater();

    bool isUpdating() const;

public slots:
    void setUpdateInterval(int miliseconds);
    void setLimit(int limit);
    void setDay(const QDate &date);

    void updateServer(DVRServer *server);
    void updateServers();

signals:
    void loadingStarted();
    void loadingFinished();

    void serverEventsAvailable(DVRServer *server, const QList<EventData *> &events);

private slots:
    void serverAdded(DVRServer *server);
    void eventsLoaded(DVRServer *server, bool ok, const QList<EventData *> &events);

private:
    DVRServerRepository *m_serverRepository;
    QSet<DVRServer *> m_updatingServers;

    QTimer m_updateTimer;
    int m_limit;
    QDateTime m_startTime;
    QDateTime m_endTime;

};

#endif // EVENTS_UPDATER_H
