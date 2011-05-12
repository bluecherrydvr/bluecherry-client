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

    QList<Range> ranges;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug d, const RangeMap &r)
{
    QString text = QLatin1String("Range: ");
    foreach (const RangeMap::Range &n, r.ranges)
        text.append(QString::number(n.start) + QLatin1String(" - ")
                    + QString::number(n.end) + QLatin1String("; "));
    return (d << text);
}
#endif

#endif // RANGEMAP_H
