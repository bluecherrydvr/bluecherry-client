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

#ifndef VISIBLETIMERANGE_H
#define VISIBLETIMERANGE_H

#include "utils/DateTimeRange.h"

class VisibleTimeRange : public QObject
{
    Q_OBJECT

    QList<int> m_tickSizes;

    DateTimeRange m_range;
    DateTimeRange m_visibleRange;

    /* Seconds of time per primary tick (a x-axis label), derived from the view area
     * and a minimum width and rounded up to a user-friendly duration in updateTimeRange */
    int m_primaryTickSecs;

    void boundVisibleRange(const DateTimeRange &boundBy);

public:
    explicit VisibleTimeRange();
    
    int primaryTickSecs() const;

    const DateTimeRange & range() const { return m_range; }
    const DateTimeRange & visibleRange() const { return m_visibleRange; }

    void setDateTimeRange(const DateTimeRange &dateTimeRange);
    void clear();

    int visibleSeconds() const;
    int minVisibleSeconds() const;
    int maxVisibleSeconds() const;

    void setZoomLevel(int zoomLevel);

    void setViewStartOffset(int secs);
    int invisibleSeconds() const;

    void computePrimaryTickSecs(int prefferedTickCount);

signals:
    void invisibleSecondsChanged(int invisibleSeconds);
    void primaryTickSecsChanged(int primaryTickSecs);

};

#endif // VISIBLETIMERANGE_H
