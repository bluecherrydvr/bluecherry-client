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
    DateTimeRange m_roundedRange;

    /* Span of time shown in the viewport */
    QDateTime m_viewTimeStart, m_viewTimeEnd;
    /* Span of seconds between timeStart and timeEnd */
    int m_timeSeconds;
    /* Span of seconds between viewTimeStart and viewTimeEnd */
    int m_viewSeconds;
    /* Seconds of time per primary tick (a x-axis label), derived from the view area
     * and a minimum width and rounded up to a user-friendly duration in updateTimeRange */
    int m_primaryTickSecs;

public:
    explicit VisibleTimeRange();
    
    int viewSeconds() const;
    int primaryTickSecs() const;
    QDateTime viewTimeStart() const;
    QDateTime timeStart() const;
    QDateTime dataTimeStart() const;
    QDateTime viewTimeEnd() const;
    QDateTime timeEnd() const;
    QDateTime dataTimeEnd() const;

    void setDataRange(const QDateTime &dataStart, const QDateTime &dataEnd);
    void clear();

    /* Zoom in the number of seconds of time visible on screen */
    int zoomSeconds() const { return m_viewSeconds; }
    int minZoomSeconds() const { return qMin(m_timeSeconds, 60); }
    int maxZoomSeconds() const { return m_timeSeconds; }

    double zoomLevel() const;
    void setZoomSeconds(int seconds);

    void setViewStartOffset(int secs);
    int invisibleSeconds() const;

    void computeViewSeconds();
    void computePrimaryTickSecs(int areaWidth, int minTickWidth);
    void computeRoundedRange();
    void computeTimeSeconds();

    void ensureViewTimeSpan();
    void addDate(const QDateTime &date);

signals:
    void invisibleSecondsChanged(int invisibleSeconds);
    void primaryTickSecsChanged(int primaryTickSecs);

};

#endif // VISIBLETIMERANGE_H
