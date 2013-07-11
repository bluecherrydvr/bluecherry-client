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

#ifndef EVENTSMODEL_H
#define EVENTSMODEL_H

#include "core/EventData.h"
#include <QAbstractItemModel>
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QColor>
#include <QBitArray>
#include <QHash>
#include <QTimer>

class DVRServer;
class DVRServerRepository;
class DVRCamera;

Q_DECLARE_METATYPE(EventData*)

class EventsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum
    {
        EventDataPtr = Qt::UserRole
    };

    enum
    {
        ServerColumn = 0,
        LocationColumn,
        TypeColumn,
        DurationColumn,
        LevelColumn,
        DateColumn,

        LastColumn = DateColumn
    };

    explicit EventsModel(DVRServerRepository *serverRepository, QObject *parent = 0);

    bool isLoading() const { return !m_updatingServers.isEmpty(); }

    void setFilterDates(const QDateTime &begin, const QDateTime &end);
    void setEventLimit(int limit) { m_serverEventsLimit = limit; }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    void setFilterDay(const QDateTime &date);
    void setServerEvents(DVRServer *server, const QList<EventData *> &events);
    void clearServerEvents(DVRServer *server);

    /* Request the most recent events from the given server, the DVRServer* source, or the
     * DVRServer represented by the ServerRequestManager* source */
    void updateServer(DVRServer *server);
    void updateServers();

    void setUpdateInterval(int ms);

signals:
    void loadingStarted();
    void loadingFinished();

private slots:
    void serverAdded(DVRServer *server);
    void eventsLoaded(bool ok, const QList<EventData *> &events);

private:
    DVRServerRepository *m_serverRepository;

    QList<EventData *> m_items;
    QMap<DVRServer *, QPair<int, int> > m_serverEventsBoundaries;
    QMap<DVRServer *, int> m_serverEventsCount;
    QSet<DVRServer *> m_updatingServers;
    QTimer m_updateTimer;
    QDateTime m_dateBegin;
    QDateTime m_dateEnd;
    int m_serverEventsLimit;

    void computeBoundaries();

};

#endif // EVENTSMODEL_H

