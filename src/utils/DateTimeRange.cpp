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

#include "DateTimeRange.h"

DateTimeRange::DateTimeRange(const QDateTime &start, const QDateTime &end)
    : m_start(start), m_end(end)
{
    Q_ASSERT(m_end >= m_start);
}

DateTimeRange::DateTimeRange()
{
}

DateTimeRange::DateTimeRange(const DateTimeRange &copyMe)
{
    m_start = copyMe.m_start;
    m_end = copyMe.m_end;
}

DateTimeRange & DateTimeRange::operator=(const DateTimeRange& copyMe)
{
    m_start = copyMe.m_start;
    m_end = copyMe.m_end;

    return *this;
}

bool DateTimeRange::isNull() const
{
    return m_start.isNull() || m_end.isNull();
}

QDateTime DateTimeRange::start() const
{
    return m_start;
}

QDateTime DateTimeRange::end() const
{
    return m_end;
}

int DateTimeRange::lengthInSeconds() const
{
    if (isNull())
        return -1;

    return m_start.secsTo(m_end);
}

bool DateTimeRange::contains(const QDateTime &dateTime) const
{
    if (isNull())
        return false;

    return dateTime >= m_start && dateTime <= m_end;
}

DateTimeRange DateTimeRange::boundedBy(const DateTimeRange &range) const
{
    if (isNull() || range.isNull())
        return DateTimeRange();

    return DateTimeRange(qMax(m_start, range.m_start), qMin(m_end, range.m_end));
}

DateTimeRange DateTimeRange::extendWith(const QDateTime &dateTime) const
{
    QDateTime start = (m_start.isNull() || dateTime < m_start) ? dateTime : m_start;
    QDateTime end = (m_end.isNull() || dateTime > m_end) ? dateTime : m_end;
    return DateTimeRange(start, end);
}

DateTimeRange DateTimeRange::withLengthInSeconds(int lengthInSeconds) const
{
    if (isNull())
        return DateTimeRange();

    return DateTimeRange(m_start, m_start.addSecs(lengthInSeconds));
}
