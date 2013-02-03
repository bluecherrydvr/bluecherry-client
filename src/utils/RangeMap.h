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

#include <QMap>
#include <QDebug>

class RangeMap
{
    friend QDebug operator<<(QDebug d, const RangeMap &r);

public:
    RangeMap();

    void insert(unsigned position, unsigned size);
    void remove(unsigned position, unsigned size);

    bool contains(unsigned position, unsigned size = 1) const;

    /* Return the position and size of the next sequential range that is not included,
     * starting from startPosition (inclusive). Returns true if there is a gap, or
     * false if there are no gaps before the end of the range. In the latter case,
     * position will be set to one past the largest position in the range, and size
     * will be zero. */
    bool nextMissingRange(unsigned startPosition, unsigned totalSize, unsigned &position, unsigned &size);
private:
    struct Range {
        unsigned start; /* First index, inclusive */
        unsigned end; /* Last index, inclusive; size is (end-start+1) */
    };

    static bool rangeStartLess(const Range &a, const Range &b);

#ifndef QT_NO_DEBUG
    void debugTestConsistency();
#endif

    QList<Range> ranges;
};

inline QDebug operator<<(QDebug d, const RangeMap &r)
{
#ifndef QT_NO_DEBUG
    QString text = QLatin1String("Range: ");
    foreach (const RangeMap::Range &n, r.ranges)
        text.append(QString::number(n.start) + QLatin1String(" - ")
                    + QString::number(n.end) + QLatin1String("; "));
    return (d << text);
#else
    Q_UNUSED(r);
    return d;
#endif
}

#endif // RANGEMAP_H
