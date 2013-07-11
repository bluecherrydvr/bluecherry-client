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
#include "server/DVRServerRepository.h"
#include "core/ServerRequestManager.h"
#include "event/EventsLoader.h"
#include <QDebug>
#include <QIcon>
#include <QTextDocument>

EventsModel::EventsModel(DVRServerRepository *serverRepository, QObject *parent)
    : QAbstractItemModel(parent), m_serverRepository(serverRepository), m_serverEventsLimit(-1)
{
    Q_ASSERT(m_serverRepository);

    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(serverAdded(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverAboutToBeRemoved(DVRServer*)), SLOT(clearServerEvents(DVRServer*)));
    connect(&m_updateTimer, SIGNAL(timeout()), SLOT(updateServers()));

    foreach (DVRServer *s, m_serverRepository->servers())
        serverAdded(s);
}

void EventsModel::serverAdded(DVRServer *server)
{
    connect(server, SIGNAL(loginSuccessful()), SLOT(updateServer()));
    connect(server, SIGNAL(disconnected(DVRServer*)), SLOT(clearServerEvents(DVRServer*)));
    updateServer(server);
}

int EventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

int EventsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return LastColumn+1;
}

QModelIndex EventsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || column < 0 || row >= m_items.size() || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column, m_items[row]);
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

void EventsModel::computeBoundaries()
{
    int begin = 0;
    m_serverEventsBoundaries.clear();
    foreach (DVRServer *server, m_serverRepository->servers())
    {
        int count = !m_serverEventsCount.contains(server) ? 0 : m_serverEventsCount.value(server);
        m_serverEventsBoundaries.insert(server, qMakePair(begin, begin + count - 1));
        begin += count;
    }
}

void EventsModel::setFilterDates(const QDateTime &begin, const QDateTime &end)
{
    m_dateBegin = begin;
    m_dateEnd = end;

    beginResetModel();
    m_items.clear();
    updateServers();
    endResetModel();
}

void EventsModel::setFilterDay(const QDateTime &date)
{
    setFilterDates(QDateTime(date.date(), QTime(0, 0)), QDateTime(date.date(), QTime(23, 59, 59, 999)));
}

void EventsModel::setUpdateInterval(int ms)
{
    if (ms > 0)
    {
        m_updateTimer.setInterval(ms);
        m_updateTimer.start();
    }
    else
        m_updateTimer.stop();
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

    if (!server->isOnline() || m_updatingServers.contains(server))
        return;

    m_updatingServers.insert(server);
    if (m_updatingServers.size() == 1)
        emit loadingStarted();

    EventsLoader *eventsLoader = new EventsLoader(server);
    eventsLoader->setLimit(m_serverEventsLimit);
    eventsLoader->setStartTime(m_dateBegin);
    eventsLoader->setEndTime(m_dateEnd);
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
        setServerEvents(server, events);

    if (m_updatingServers.remove(server) && m_updatingServers.isEmpty())
        emit loadingFinished();
}

void EventsModel::setServerEvents(DVRServer *server, const QList<EventData *> &events)
{
    clearServerEvents(server);
    computeBoundaries();

    int insertedRowBegin = m_serverEventsBoundaries.value(server).first;
    int insertedRowEnd = insertedRowBegin + events.count() - 1;

    if (insertedRowEnd >= insertedRowBegin)
    {
        beginInsertRows(QModelIndex(), insertedRowBegin, insertedRowEnd);
        m_items = m_items.mid(0, insertedRowBegin) + events + m_items.mid(insertedRowBegin);
        endInsertRows();
    }

    m_serverEventsCount.insert(server, events.count());
}

void EventsModel::clearServerEvents(DVRServer *server)
{
    computeBoundaries();

    int removedRowBegin = m_serverEventsBoundaries.value(server).first;
    int removedRowEnd = m_serverEventsBoundaries.value(server).second;

    if (removedRowEnd >= removedRowBegin)
    {
        beginRemoveRows(QModelIndex(), removedRowBegin, removedRowEnd);
        QList<EventData *> removedEvents = m_items.mid(removedRowBegin, removedRowEnd - removedRowBegin + 1);
        qDeleteAll(removedEvents);
        m_items = m_items.mid(0, removedRowBegin) + m_items.mid(removedRowEnd + 1);
        endRemoveRows();
    }

    m_serverEventsCount.remove(server);
}
