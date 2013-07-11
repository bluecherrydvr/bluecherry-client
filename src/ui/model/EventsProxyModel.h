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

#ifndef EVENTS_PROXY_MODEL_H
#define EVENTS_PROXY_MODEL_H

#include "core/EventData.h"
#include <QBitArray>
#include <QSortFilterProxyModel>

class EventsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum IncompletePlace
    {
        IncompleteInPlace,
        IncompleteFirst,
        IncompleteLast
    };

    explicit EventsProxyModel(QObject *parent);
    virtual ~EventsProxyModel();

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    void setColumn(int column);
    void setIncompletePlace(IncompletePlace incompletePlace);

    void setMinimumLevel(EventLevel minimumLevel);
    void setTypes(QBitArray types);
    void setDay(const QDate &day);
    void setSources(const QMap<DVRServer*, QSet<int> > &sources);

private:
    int m_column;
    IncompletePlace m_incompletePlace;
    EventLevel m_minimumLevel;
    QBitArray m_types;
    QDate m_day;
    QMap<DVRServer*, QSet<int> > m_sources;

    bool filterAcceptsRow(EventData *eventData) const;
    bool lessThan(EventData *left, EventData *right) const;

};

#endif // EVENTS_PROXY_MODEL_H
