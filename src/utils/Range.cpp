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

#include "Range.h"
#include <QtGlobal>

Range Range::invalid()
{
    Range result;
    result.m_start = 1;
    result.m_end = 0;

    return result;
}

Range Range::fromValue(unsigned value)
{
    Range result;
    result.m_start = value;
    result.m_end = value;

    return result;
}

Range Range::fromStartEnd(unsigned start, unsigned end)
{
    Range result;
    result.m_start = start;
    result.m_end = end;

    return result;
}

Range Range::fromStartSize(unsigned start, unsigned size)
{
    Range result;
    result.m_start = start;
    result.m_end = start + size - 1;

    return result;
}

Range::Range()
    : m_start(1), m_end(0)
{
}

unsigned Range::size() const
{
    if (isValid())
        return m_end - m_start + 1;
    else
        return 0;
}

bool Range::isValid() const
{
    return m_end >= m_start;
}

bool Range::includes(unsigned value) const
{
    return (value >= m_start) && (value <= m_end);
}

bool Range::includes(const Range &otherRange) const
{
    return (otherRange.start() >= m_start) && (otherRange.end() <= m_end);
}
