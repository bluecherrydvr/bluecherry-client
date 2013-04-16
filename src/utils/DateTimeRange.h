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

#ifndef DATETIMERANGE_H
#define DATETIMERANGE_H

#include <QDateTime>

class DateTimeRange
{
    QDateTime m_start;
    QDateTime m_end;

public:
    DateTimeRange(const QDateTime &start, const QDateTime &end);
    DateTimeRange();
    DateTimeRange(const DateTimeRange &copyMe);

    DateTimeRange & operator = (const DateTimeRange &copyMe);
    bool operator == (const DateTimeRange &compareTo) const;

    bool isNull() const;

    QDateTime start() const;
    QDateTime end() const;

    int lengthInSeconds() const;
    bool contains(const QDateTime &dateTime) const;
    bool contains(const DateTimeRange &dateTimeRange) const;

    DateTimeRange boundedBy(const DateTimeRange &range) const;
    DateTimeRange extendWith(const QDateTime &dateTime) const;
    DateTimeRange withLengthInSeconds(int lengthInSeconds) const;
    DateTimeRange moveInto(const DateTimeRange &dateTime) const;
    DateTimeRange moveStart(const QDateTime &start) const;

};

#endif // DATETIMERANGE_H
