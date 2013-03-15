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

    DateTimeRange m_range;
    DateTimeRange m_visibleRange;

    /* Span of seconds between timeStart and timeEnd */
    int m_timeSeconds;
    /* Span of seconds between viewTimeStart and viewTimeEnd */
    int m_viewSeconds;
    /* Seconds of time per primary tick (a x-axis label), derived from the view area
     * and a minimum width and rounded up to a user-friendly duration in updateTimeRange */
    int m_primaryTickSecs;

    void computeVisibleRange();

public:
    explicit VisibleTimeRange();
    
    int viewSeconds() const;
    int primaryTickSecs() const;
    QDateTime viewTimeStart() const;
    QDateTime timeStart() const;
    QDateTime viewTimeEnd() const;
    QDateTime timeEnd() const;

    void setDataRange(const QDateTime &dataStart, const QDateTime &dataEnd);
    void clear();

    int visibleSeconds() const { return m_viewSeconds; }
    int minVisibleSeconds() const { return qMin(m_timeSeconds, 60); }
    int maxVisibleSeconds() const { return m_timeSeconds; }

    double zoomLevel() const;
    void setZoomSeconds(int seconds);

    void setViewStartOffset(int secs);
    int invisibleSeconds() const;

    void computePrimaryTickSecs(int areaWidth, int minTickWidth);
    void computeTimeSeconds();

    void ensureViewTimeSpan();
    void addDate(const QDateTime &date);

signals:
    void invisibleSecondsChanged(int invisibleSeconds);
    void primaryTickSecsChanged(int primaryTickSecs);

};

#endif // VISIBLETIMERANGE_H
