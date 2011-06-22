#include "RangeMap.h"
#include <QtAlgorithms>
#include <QDebug>

#undef RANGEMAP_DEBUG

RangeMap::RangeMap()
{
}

inline bool RangeMap::rangeStartLess(const RangeMap::Range &a, const RangeMap::Range &b)
{
    return a.start < b.start;
}

bool RangeMap::contains(unsigned position, unsigned size) const
{
    if (ranges.isEmpty())
        return false;

    Range range = { position, position+size-1 };
    QList<Range>::ConstIterator it = qLowerBound(ranges.begin(), ranges.end(), range, rangeStartLess);
    if (it == ranges.end() || it->start > position)
        --it;

    /* Position is 1, size is 1; last pos required is 1 */
    Q_ASSERT(it->start <= position);
    if (it != ranges.end() && it->end >= range.end)
        return true;

    return false;
}

bool RangeMap::nextMissingRange(unsigned startPosition, unsigned totalSize, unsigned &position, unsigned &size)
{
    if (ranges.isEmpty())
    {
        position = startPosition;
        size = totalSize;
        return true;
    }

    /* Find the range inclusive of or less than startPosition */
    Range r = { startPosition, startPosition };
    QList<Range>::ConstIterator it = qLowerBound(ranges.begin(), ranges.end(), r, rangeStartLess);
    if (it == ranges.end() || it->start > startPosition)
        --it;

    Q_ASSERT(it->start <= startPosition);

    position = qMax(startPosition, it->end+1);
    size = qMin((it+1 == ranges.end()) ? (totalSize - position) : ((it+1)->start - position - 1),
                totalSize - position);

    Q_ASSERT(size == 0 || (position+size) == totalSize || (position+size)+1 == (it+1)->start);

    return size != 0;
}

void RangeMap::insert(unsigned position, unsigned size)
{
    Range range = { position, position+size-1 };

    /* Item with a position LESS THAN OR EQUAL TO the BEGINNING of the inserted range */
    QList<Range>::Iterator lower = qLowerBound(ranges.begin(), ranges.end(), range, rangeStartLess);
    if (!ranges.isEmpty() && (lower == ranges.end() || lower->start > position))
        lower--;
    /* Item with a position GREATER THAN the END of the inserted range */
    Range r2 = { range.end+1, range.end+1 };
    QList<Range>::Iterator upper = qUpperBound(lower, ranges.end(), r2, rangeStartLess);

    Q_ASSERT(lower == ranges.end() || lower->start <= position);

    /* Intersection at the beginning */
    if (lower != ranges.end() && position <= lower->end+1)
    {
        /* The beginning of the new range intersects the range in 'lower'.
         * Extend this range, and merge ranges after it as needed. */

        Q_ASSERT(lower->start <= position);
        lower->end = qMax(range.end, (upper-1)->end);

        if ((upper-1) != lower)
            ranges.erase(lower+1, upper);
    }
    else if (lower != ranges.end() && range.end <= (upper-1)->end)
    {
        /* The end of the new range intersects the range in 'upper'.
         * Extend this range. Merging is unnecessary, because any case
         * where that would be possible is included in the prior one. */

        upper--;
        upper->start = position;
    }
    else
    {
        /* Create a new range item */
        ranges.insert(upper, range);
    }

#ifdef RANGEMAP_DEBUG
    qDebug() << "Rangemap insert" << position << size << ":" << *this;
#endif
    debugTestConsistency();
}

#ifndef QT_NO_DEBUG
void RangeMap::debugTestConsistency()
{
    Range last = { 0, 0 };
    for (QList<Range>::Iterator it = ranges.begin(); it != ranges.end(); ++it)
    {
        const Range &current = *it;
        const bool isFirst = it == ranges.begin();

        /* End is after start; these are both inclusive, so they may be equal (in the case
         * of a range of one). */
        Q_ASSERT(current.start <= current.end);

        if (!isFirst)
        {
            /* Sequentially ordered */
            Q_ASSERT(current.start > last.end);
            /* There is a gap of at least one between ranges (otherwise, they would be the same range) */
            Q_ASSERT(current.start - last.end > 1);
        }

        last = current;
    }
}
#endif
