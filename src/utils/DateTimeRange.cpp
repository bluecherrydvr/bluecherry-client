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

QDateTime DateTimeRange::start() const
{
    return m_start;
}

QDateTime DateTimeRange::end() const
{
    return m_end;
}