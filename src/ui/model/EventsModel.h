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

    bool isLoading() const { return !updatingServers.isEmpty(); }

    void setFilterDates(const QDateTime &begin, const QDateTime &end);
    void setEventLimit(int limit) { serverEventsLimit = limit; }

    QString filterDescription() const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

public slots:
    void setFilterSource(DVRCamera *camera);
    void setFilterSource(DVRServer *server);
    void setFilterSources(const QMap<DVRServer*,QList<int> > &sources);

    void setFilterTypes(const QBitArray &typemap);
    void setFilterLevel(EventLevel minimum);

    void setFilterBeginDate(const QDateTime &begin) { setFilterDates(begin, m_filter.dateEnd); }
    void setFilterEndDate(const QDateTime &end) { setFilterDates(m_filter.dateBegin, end); }
    void setFilterDay(const QDateTime &date);

    void clearFilters();

    /* Request the most recent events from the given server, the DVRServer* source, or the
     * DVRServer represented by the ServerRequestManager* source */
    void updateServer(DVRServer *server = 0);
    void updateServers();

    void setUpdateInterval(int ms);
    void stopUpdates() { setUpdateInterval(-1); }

    void setIncompleteEventsFirst(bool enabled);

signals:
    void filtersChanged();
    void loadingStarted();
    void loadingFinished();

private slots:
    void serverAdded(DVRServer *server);
    void eventsLoaded(bool ok, const QList<EventData *> &events);

    void clearServerEvents(DVRServer *server = 0);

private:
    DVRServerRepository *m_serverRepository;
    QList<EventData*> items;
    QHash<DVRServer*,QList<EventData*> > cachedEvents;
    QSet<DVRServer*> updatingServers;
    QTimer updateTimer;

    /* Filters */
    struct Filter
    {
        QHash<DVRServer*, QSet<int> > sources;
        QDateTime dateBegin, dateEnd;
        QBitArray types;
        EventLevel level;

        bool operator()(const EventData *d) const;
    } m_filter;

    /* Sorting */
    int serverEventsLimit;
    int sortColumn;
    Qt::SortOrder sortOrder;
    bool incompleteEventsFirst;

    void resort() { sort(sortColumn, sortOrder); }

    void applyFilters(bool fromCache = true);
    bool testFilter(EventData *data);

    void createTestData();
};

#endif // EVENTSMODEL_H
