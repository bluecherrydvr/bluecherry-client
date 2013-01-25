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

#include "RangeMap.h"
#include <QtAlgorithms>
#include <QDebug>

RangeMap::RangeMap()
{
}

inline bool RangeMap::rangeStartLess(const Range &a, const Range &b)
{
    return a.start() < b.start();
}

bool RangeMap::contains(const Range &range)
{
    if (ranges.isEmpty())
        return false;

    QList<Range>::Iterator it = findContainingRange(range.start());
    return it != ranges.end() && it->includes(range);
}

QList<Range>::Iterator RangeMap::findContainingRange(unsigned value)
{
    if (ranges.begin()->start() > value)
        return ranges.end();
    if ((ranges.end() - 1)->end() < value)
        return ranges.end();

    QList<Range>::Iterator from = ranges.begin();
    QList<Range>::Iterator to = ranges.end();

    while (true)
    {
        int half = (to - from) / 2;
        QList<Range>::Iterator i = from + half;
        if (i->includes(value))
            return i;
        if (from == to)
            return ranges.end();
        if (i->start() > value)
            to = i - 1;
        else if (i->end() < value)
            from = i + 1;
    }

    Q_ASSERT(false);
    return ranges.end();
}

QList<Range>::Iterator RangeMap::findContainingOrPrecedingRange(unsigned value)
{
    if (ranges.isEmpty())
        return ranges.end();
    if (ranges.begin()->start() > value)
        return ranges.end();
    if ((ranges.end() - 1)->start() <= value)
        return ranges.end() - 1;

    QList<Range>::Iterator from = ranges.begin();
    QList<Range>::Iterator to = ranges.end();

    while (true)
    {
        int half = (to - from) / 2;
        QList<Range>::Iterator i = from + half;
        if (i->includes(value))
            return i;
        if (from == to)
            return from;
        if (i->start() > value)
            to = i - 1;
        else if (i->end() < value)
        {
            QList<Range>::Iterator next = i + 1;
            if (next == ranges.end() || next->start() > value) {
                return i;
            }
            from = i + 1;
        }
    }

    Q_ASSERT(false);
    return ranges.end();
}

unsigned RangeMap::firstNotInRange(QList<Range>::Iterator rangeIterator)
{
    if (rangeIterator == ranges.end())
        return 0;
    else
        return rangeIterator->end() + 1;
}

unsigned RangeMap::lastNotInRange(QList<Range>::Iterator rangeIterator)
{
    if (rangeIterator == ranges.end())
        return UINT_MAX;
    else if (rangeIterator->start() == 0)
        return 0;
    else
        return rangeIterator->start() - 1;
}

Range RangeMap::nextMissingRange(const Range &search)
{
    if (ranges.isEmpty())
        return search;

    QList<Range>::Iterator precedingRange = findContainingOrPrecedingRange(search.start());
    QList<Range>::Iterator followingRange = nextRange(precedingRange);

    return Range::fromStartEnd(
        qMax(search.start(), firstNotInRange(precedingRange)),
        qMin(search.end(), lastNotInRange(followingRange)));
}

QList<Range>::Iterator RangeMap::nextRange(QList<Range>::Iterator range)
{
    if (range == ranges.end()) // for no-range next is first
        return ranges.begin();
    else
        return range + 1;
}

QList<Range>::Iterator RangeMap::findFirstMergingRange(unsigned value)
{
    QList<Range>::Iterator precedingRange = findContainingOrPrecedingRange(value);
    if (precedingRange == ranges.end())
        return precedingRange;
    if (precedingRange->includes(value))
        return precedingRange;

    if (precedingRange->end() + 1 != value)
        return precedingRange + 1;
    else
        return precedingRange;
}

QList<Range>::Iterator RangeMap::findLastMergingRange(unsigned value)
{
    QList<Range>::Iterator precedingRange = findContainingOrPrecedingRange(value);
    QList<Range>::Iterator followingRange = nextRange(precedingRange);
    if (followingRange == ranges.end())
        return precedingRange;

    if (followingRange->start() - 1 == value)
        return followingRange;
    else
        return precedingRange;
}

void RangeMap::insert(const Range &range)
{
    if (!range.isValid())
        return;

    if (ranges.isEmpty())
    {
        ranges.append(range);
        return;
    }

    QList<Range>::Iterator firstMergingRange = findFirstMergingRange(range.start());
    QList<Range>::Iterator lastMergingRange = findLastMergingRange(range.end());

    if (firstMergingRange > lastMergingRange)
    {
        ranges.insert(firstMergingRange, range);
        return;
    }

    if (firstMergingRange == ranges.end())
    {
        ranges.prepend(range);
        return;
    }

    Range finalRange = Range::fromStartEnd(
                qMin(range.start(), firstMergingRange->start()),
                qMax(range.end(), lastMergingRange->end()));
    ranges.erase(firstMergingRange, lastMergingRange);
    *lastMergingRange = finalRange;
}

#ifndef QT_NO_DEBUG
void RangeMap::debugTestConsistency()
{
    Range last = Range::fromValue(0);
    for (QList<Range>::Iterator it = ranges.begin(); it != ranges.end(); ++it)
    {
        const Range &current = *it;
        const bool isFirst = it == ranges.begin();

        /* End is after start; these are both inclusive, so they may be equal (in the case
         * of a range of one). */
        Q_ASSERT(current.start() <= current.end());

        if (!isFirst)
        {
            /* Sequentially ordered */
            Q_ASSERT(current.start() > last.end());
            /* There is a gap of at least one between ranges (otherwise, they would be the same range) */
            Q_ASSERT(current.start() - last.end() > 1);
        }

        last = current;
    }
}
#endif
