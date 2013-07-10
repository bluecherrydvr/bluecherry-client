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

#include "EventsModel.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerRepository.h"
#include "camera/DVRCamera.h"
#include "core/EventData.h"
#include "core/ServerRequestManager.h"
#include "event/EventsLoader.h"
#include <QTextDocument>
#include <QColor>
#include <QDebug>
#include <QIcon>
#include <QtConcurrentRun>

EventsModel::EventsModel(DVRServerRepository *serverRepository, QObject *parent)
    : QAbstractItemModel(parent), m_serverRepository(serverRepository), serverEventsLimit(-1)
{
    Q_ASSERT(m_serverRepository);

    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(serverAdded(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverRemoved(DVRServer*)), SLOT(clearServerEvents(DVRServer*)));
    connect(&updateTimer, SIGNAL(timeout()), SLOT(updateServers()));

    applyFilters();

    foreach (DVRServer *s, m_serverRepository->servers())
        serverAdded(s);
}

void EventsModel::serverAdded(DVRServer *server)
{
    connect(server, SIGNAL(loginSuccessful()), SLOT(updateServer()));
    connect(server, SIGNAL(disconnected()), SLOT(clearServerEvents()));
    updateServer(server);
}

int EventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return items.size();
}

int EventsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return LastColumn+1;
}

QModelIndex EventsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || column < 0 || row >= items.size() || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column, items[row]);
}

QModelIndex EventsModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EventData *data = reinterpret_cast<EventData*>(index.internalPointer());
    if (!data)
        return QVariant();

    if (role == EventDataPtr)
    {
        return QVariant::fromValue(data);
    }
    else if (role == Qt::ToolTipRole)
    {
        return tr("%1 (%2)<br>%3 on %4<br>%5").arg(data->uiType(), data->uiLevel(), Qt::escape(data->uiLocation()),
                                                   Qt::escape(data->uiServer()), data->serverStartDate().toString());
    }
    else if (role == Qt::ForegroundRole)
    {
        return data->uiColor(false);
    }

    switch (index.column())
    {
    case ServerColumn:
        if (role == Qt::DisplayRole)
        {
            if (data->server())
                return data->server()->configuration().displayName();
            else
                return QString();
        }
        break;
    case LocationColumn:
        if (role == Qt::DisplayRole)
            return data->uiLocation();
        break;
    case TypeColumn:
        if (role == Qt::DisplayRole)
            return data->uiType();
        else if (role == Qt::DecorationRole)
            return data->hasMedia() ? QIcon(QLatin1String(":/icons/control-000-small.png")) : QVariant();
        break;
    case DurationColumn:
        if (role == Qt::DisplayRole)
            return data->uiDuration();
        else if (role == Qt::EditRole)
            return data->durationInSeconds();
        else if (role == Qt::FontRole && data->inProgress())
        {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;
    case LevelColumn:
        if (role == Qt::DisplayRole)
            return data->uiLevel();
        else if (role == Qt::EditRole)
            return data->level().level;
        break;
    case DateColumn:
        if (role == Qt::DisplayRole)
            return data->serverStartDate().toString();
        else if (role == Qt::EditRole)
            return data->utcStartDate();
        break;
    }

    return QVariant();
}

QVariant EventsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case ServerColumn: return tr("Server");
    case LocationColumn: return tr("Device");
    case TypeColumn: return tr("Event");
    case DurationColumn: return tr("Duration");
    case LevelColumn: return tr("Priority");
    case DateColumn: return tr("Date");
    }

    return QVariant();
}

void EventsModel::clearServerEvents(DVRServer *server)
{
    if (!server && !(server = qobject_cast<DVRServer*>(sender())))
    {
        ServerRequestManager *srm = qobject_cast<ServerRequestManager*>(sender());
        if (srm)
            server = srm->server;
        else
        {
            Q_ASSERT_X(false, "clearServerEvents", "No server and no appropriate sender");
            return;
        }
    }

    /* Group contiguous removed rows together; provides a significant boost in performance */
    int removeFirst = -1;
    for (int i = 0; ; ++i)
    {
        if (i < items.size() && items[i]->server() == server)
        {
            if (removeFirst < 0)
                removeFirst = i;
        }
        else if (removeFirst >= 0)
        {
            beginRemoveRows(QModelIndex(), removeFirst, i-1);
            items.erase(items.begin()+removeFirst, items.begin()+i);
            i = removeFirst;
            removeFirst = -1;
            endRemoveRows();
        }

        if (i == items.size())
            break;
    }

    cachedEvents.remove(server);
}

bool EventsModel::Filter::acceptEvent(const EventData *data) const
{
    if (    (!types.isNull() && (int)data->type() >= 0 && !types[(int)data->type()]) ||
            (!dateBegin.isNull() && data->serverStartDate() < dateBegin) ||
            (!dateEnd.isNull() && data->serverStartDate() > dateEnd))
        return false;

    QHash<DVRServer*, QSet<int> >::ConstIterator it = sources.find(data->server());
    if (!sources.isEmpty() && (it == sources.end() || (!it->isEmpty() && !it->contains(data->locationId()))))
        return false;

    return true;
}

void EventsModel::applyFilters(bool fromCache)
{
    if (fromCache)
    {
        beginResetModel();
        items.clear();

        for (QHash<DVRServer*,QList<EventData*> >::Iterator it = cachedEvents.begin(); it != cachedEvents.end(); ++it)
        {
            if (!m_filter.sources.isEmpty() && !m_filter.sources.contains(it.key()))
                continue;

            for (QList<EventData*>::Iterator eit = it->begin(); eit != it->end(); ++eit)
            {
                if (m_filter.acceptEvent(*eit))
                    items.append(*eit);
            }
        }

        endResetModel();
    }
    else
    {
        /* Group contiguous removed rows together; provides a significant boost in performance */
        int removeFirst = -1;
        for (int i = 0; ; ++i)
        {
            if (i < items.size() && !m_filter.acceptEvent(items[i]))
            {
                if (removeFirst < 0)
                    removeFirst = i;
            }
            else if (removeFirst >= 0)
            {
                beginRemoveRows(QModelIndex(), removeFirst, i-1);
                items.erase(items.begin()+removeFirst, items.begin()+i);
                i = removeFirst;
                removeFirst = -1;
                endRemoveRows();
            }

            if (i == items.size())
                break;
        }
    }
}

void EventsModel::setFilterDates(const QDateTime &begin, const QDateTime &end)
{
    m_filter.dateBegin = begin;
    m_filter.dateEnd = end;

    beginResetModel();
    items.clear();
    updateServers();
    endResetModel();
}

void EventsModel::setFilterDay(const QDateTime &date)
{
    setFilterDates(QDateTime(date.date(), QTime(0, 0)), QDateTime(date.date(), QTime(23, 59, 59, 999)));
}

void EventsModel::setFilterSources(const QMap<DVRServer*, QList<int> > &sources)
{
    bool fast = false;

    if (sources.size() <= m_filter.sources.size())
    {
        fast = true;
        /* If the new sources contain any that the old don't, we can't do fast filtering */
        for (QMap<DVRServer*,QList<int> >::ConstIterator nit = sources.begin(); nit != sources.end(); ++nit)
        {
            QHash<DVRServer*, QSet<int> >::Iterator oit = m_filter.sources.find(nit.key());
            if (oit == m_filter.sources.end())
            {
                fast = false;
                break;
            }

            for (QList<int>::ConstIterator it = nit->begin(); it != nit->end(); ++it)
            {
                if (!oit->contains(*it))
                {
                    fast = false;
                    break;
                }
            }

            if (!fast)
                break;
        }
    }
    else if (m_filter.sources.isEmpty())
        fast = true;

    m_filter.sources.clear();
    for (QMap<DVRServer*, QList<int> >::ConstIterator nit = sources.begin(); nit != sources.end(); ++nit)
        m_filter.sources.insert(nit.key(), nit->toSet());

    applyFilters(!fast);
}

void EventsModel::setFilterSource(DVRCamera *camera)
{
    if (!camera)
        return;

    QMap<DVRServer*,QList<int> > sources;
    sources.insert(camera->data().server(), QList<int>() << camera->data().id());
    setFilterSources(sources);
}

void EventsModel::setFilterSource(DVRServer *server)
{
    if (!server)
        return;

    QMap<DVRServer*,QList<int> > sources;
    sources.insert(server, QList<int>());
    setFilterSources(sources);
}

void EventsModel::setFilterTypes(const QBitArray &typemap)
{
    bool fast = true;

    if (!m_filter.types.isNull() && m_filter.types.size() == typemap.size())
    {
        for (int i = 0; i < typemap.size(); ++i)
        {
            if (typemap[i] && !m_filter.types[i])
            {
                fast = false;
                break;
            }
        }
    }

    m_filter.types = typemap;
    applyFilters(!fast);
}

void EventsModel::setUpdateInterval(int ms)
{
    if (ms > 0)
    {
        updateTimer.setInterval(ms);
        updateTimer.start();
    }
    else
        updateTimer.stop();
}

void EventsModel::updateServers()
{
    foreach (DVRServer *s, m_serverRepository->servers())
        updateServer(s);
}

void EventsModel::updateServer(DVRServer *server)
{
    if (!server && !(server = qobject_cast<DVRServer*>(sender())))
    {
        ServerRequestManager *srm = qobject_cast<ServerRequestManager*>(sender());
        if (srm)
            server = srm->server;
        else
            return;
    }

    if (!server->isOnline() || updatingServers.contains(server))
        return;

    updatingServers.insert(server);
    if (updatingServers.size() == 1)
        emit loadingStarted();

    EventsLoader *eventsLoader = new EventsLoader(server);
    eventsLoader->setLimit(serverEventsLimit);
    eventsLoader->setStartTime(m_filter.dateBegin);
    eventsLoader->setEndTime(m_filter.dateEnd);
    connect(eventsLoader, SIGNAL(eventsLoaded(bool,QList<EventData*>)), this, SLOT(eventsLoaded(bool,QList<EventData*>)));
    eventsLoader->loadEvents();
}

void EventsModel::eventsLoaded(bool ok, const QList<EventData *> &events)
{
    EventsLoader *eventsLoader = qobject_cast<EventsLoader *>(sender());
    Q_ASSERT(eventsLoader);

    DVRServer *server = eventsLoader->server();
    if (!server)
        return;

    if (ok)
    {
        QList<EventData*> &cache = cachedEvents[server];
        qDeleteAll(cache);
        cache = events;

        applyFilters();
    }

    if (updatingServers.remove(server) && updatingServers.isEmpty())
        emit loadingFinished();
}
