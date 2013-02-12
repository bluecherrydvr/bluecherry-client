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

#ifndef RANGE_H
#define RANGE_H

class Range
{
public:
    static Range invalid();
    static Range fromValue(unsigned value);
    static Range fromStartEnd(unsigned start, unsigned end);
    static Range fromStartSize(unsigned start, unsigned size);

    Range();

    unsigned start() const { return m_start; }
    unsigned end() const { return m_end; }
    unsigned size() const;

    bool isValid() const;
    bool includes(unsigned value) const;
    bool includes(const Range &otherRange) const;

private:

    unsigned m_start; // inclusive
    unsigned m_end; // inclusive
};

#endif // RANGE_H
