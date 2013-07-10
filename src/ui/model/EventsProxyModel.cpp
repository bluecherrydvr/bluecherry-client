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

#include "EventsProxyModel.h"

#include "core/EventData.h"
#include "ui/model/EventsModel.h"

EventsProxyModel::EventsProxyModel(QObject *parent) :
        QSortFilterProxyModel(parent), m_column(EventsModel::ServerColumn),
        m_incompletePlace(IncompleteInPlace), m_minimumLevel(EventLevel::Minimum)
{
}

EventsProxyModel::~EventsProxyModel()
{
}

bool EventsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceParent.isValid())
        return true;

    EventData *eventData = sourceModel()->index(sourceRow, 0).data(EventsModel::EventDataPtr).value<EventData *>();
    if (!eventData)
        return false;

    return filterAcceptsRow(eventData);
}

bool EventsProxyModel::filterAcceptsRow(EventData *eventData) const
{
    if (eventData->level() < m_minimumLevel)
        return false;

    if (!m_types.isNull() && (int)eventData->type() >= 0 && !m_types[(int)eventData->type()])
        return false;

    return true;
}

bool EventsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    EventData *leftEvent = left.data(EventsModel::EventDataPtr).value<EventData *>();
    EventData *rightEvent = right.data(EventsModel::EventDataPtr).value<EventData *>();

    if (leftEvent && rightEvent)
        return lessThan(leftEvent, rightEvent);

    return QSortFilterProxyModel::lessThan(left, right);
}

bool EventsProxyModel::lessThan(EventData *left, EventData *right) const
{
    if (m_incompletePlace != IncompleteInPlace)
    {
        if (left->inProgress() && !right->inProgress())
            return m_incompletePlace == IncompleteFirst ? true : false;
        else if (right->inProgress() && !left->inProgress())
            return m_incompletePlace == IncompleteFirst ? false : true;
    }

    switch (m_column)
    {
        case EventsModel::ServerColumn:
            return QString::localeAwareCompare(left->server()->configuration().displayName(), right->server()->configuration().displayName()) < 0;
        case EventsModel::LocationColumn:
            return QString::localeAwareCompare(left->uiLocation(), right->uiLocation()) < 0;
        case EventsModel::TypeColumn:
            return QString::localeAwareCompare(left->uiType(), right->uiType()) < 0;
        case EventsModel::DurationColumn:
            return left->durationInSeconds() < right->durationInSeconds();
        case EventsModel::LevelColumn:
            return left->level() < right->level();
        case EventsModel::DateColumn:
            return left->utcStartDate() < right->utcStartDate();
        default:
            return left < right;
    }
}

void EventsProxyModel::setColumn(int column)
{
    if (m_column == column)
        return;

    m_column = column;
    invalidateFilter();
}

void EventsProxyModel::setIncompletePlace(IncompletePlace incompletePlace)
{
    if (m_incompletePlace == incompletePlace)
        return;

    m_incompletePlace = incompletePlace;
    invalidateFilter();
}

void EventsProxyModel::setMinimumLevel(EventLevel minimumLevel)
{
    if (m_minimumLevel == minimumLevel)
        return;

    m_minimumLevel = minimumLevel;
    invalidateFilter();
}

void EventsProxyModel::setTypes(QBitArray types)
{
    if (m_types == types)
        return;

    m_types = types;
    invalidateFilter();
}