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

#ifndef RANGEMAP_H
#define RANGEMAP_H

#include "utils/Range.h"
#include <QMap>
#include <QDebug>

class RangeMap
{
    friend QDebug operator<<(QDebug d, const RangeMap &r);

public:
    RangeMap();

    void insert(const Range &range);
    bool contains(const Range &range);

    /* Return the first subrange of search that is not included in this RangeMap.
       May return empty range if it is contained. */
    Range nextMissingRange(const Range &search);
private:
    int size() const { return ranges.size(); }

    unsigned firstNotInRange(QList<Range>::Iterator rangeIterator);
    unsigned lastNotInRange(QList<Range>::Iterator rangeIterator);

    QList<Range>::Iterator findContainingRange(unsigned value);
    QList<Range>::Iterator findContainingOrPrecedingRange(unsigned value);
    QList<Range>::Iterator nextRange(QList<Range>::Iterator range);

    QList<Range>::Iterator findFirstMergingRange(unsigned value);
    QList<Range>::Iterator findLastMergingRange(unsigned value);

#ifndef QT_NO_DEBUG
    void debugTestConsistency();
#endif

    QList<Range> ranges;
};

inline QDebug operator<<(QDebug d, const RangeMap &r)
{
#ifndef QT_NO_DEBUG
    QString text = QLatin1String("Range: ");
    foreach (const Range &n, r.ranges)
        text.append(QString::number(n.start()) + QLatin1String(" - ")
                    + QString::number(n.end()) + QLatin1String("; "));
    return (d << text);
#else
    Q_UNUSED(r);
    return d;
#endif
}

#endif // RANGEMAP_H
