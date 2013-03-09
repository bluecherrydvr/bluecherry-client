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

#include <QDateTime>

struct VisibleTimeRange
{
    /* Total span of time represented by the timeline, rounded from dataTime */
    QDateTime timeStart, timeEnd;
    /* Span of time represented in data; end is inclusive of the duration */
    QDateTime dataTimeStart, dataTimeEnd;
    /* Span of time shown in the viewport */
    QDateTime viewTimeStart, viewTimeEnd;
    /* Span of seconds between timeStart and timeEnd */
    int timeSeconds;
    /* Span of seconds between viewTimeStart and viewTimeEnd */
    int viewSeconds;
    /* Seconds of time per primary tick (a x-axis label), derived from the view area
     * and a minimum width and rounded up to a user-friendly duration in updateTimeRange */
    int primaryTickSecs;

public:
    explicit VisibleTimeRange();

    void clear();

};

#endif // VISIBLETIMERANGE_H
