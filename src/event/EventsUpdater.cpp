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

#include "EventsUpdater.h"
#include "server/DVRServer.h"
#include "server/DVRServerRepository.h"
#include "event/EventsLoader.h"

EventsUpdater::EventsUpdater(DVRServerRepository *serverRepository, QObject *parent) :
        QObject(parent), m_serverRepository(serverRepository), m_limit(-1)
{
    Q_ASSERT(m_serverRepository);

    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(serverAdded(DVRServer*)));
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(updateServers()));

    foreach (DVRServer *s, m_serverRepository->servers())
        serverAdded(s);
}

EventsUpdater::~EventsUpdater()
{
}

bool EventsUpdater::isUpdating() const
{
    return !m_updatingServers.isEmpty();
}

void EventsUpdater::serverAdded(DVRServer *server)
{
    //connect(server, SIGNAL(loginSuccessful(DVRServer*)), SLOT(updateServer(DVRServer*)));
    //updateServer(server);
}

void EventsUpdater::setUpdateInterval(int miliseconds)
{
    if (miliseconds > 0)
    {
        m_updateTimer.setInterval(miliseconds);
        m_updateTimer.start();
    }
    else
        m_updateTimer.stop();
}

void EventsUpdater::setLimit(int limit)
{
    m_limit = limit;
}

void EventsUpdater::setDay(const QDate &date)
{
    m_startTime = QDateTime(date, QTime(0, 0));
    m_endTime = QDateTime(date, QTime(23, 59, 59, 999));

    //updateServers();
}

void EventsUpdater::updateServers()
{
    foreach (DVRServer *s, m_serverRepository->servers())
        updateServer(s);
}

void EventsUpdater::updateServer(DVRServer *server)
{
    if (!server->isOnline() || m_updatingServers.contains(server))
        return;

    m_updatingServers.insert(server);
    if (m_updatingServers.size() == 1)
        emit loadingStarted();

    EventsLoader *eventsLoader = new EventsLoader(server);
    connect(eventsLoader, SIGNAL(eventsLoaded(DVRServer*,bool,QList<EventData*>)),
            this, SLOT(eventsLoaded(DVRServer*,bool,QList<EventData*>)));

    eventsLoader->setLimit(m_limit);
    eventsLoader->setStartTime(m_startTime);
    eventsLoader->setEndTime(m_endTime);
    eventsLoader->loadEvents();
}

void EventsUpdater::eventsLoaded(DVRServer *server, bool ok, const QList<EventData *> &events)
{
    if (!server)
        return;

    if (ok)
        emit serverEventsAvailable(server, events);

    if (m_updatingServers.remove(server) && m_updatingServers.isEmpty())
        emit loadingFinished();
}
