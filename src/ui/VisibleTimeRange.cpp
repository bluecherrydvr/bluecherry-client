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

#include "VisibleTimeRange.h"

VisibleTimeRange::VisibleTimeRange()
    : timeSeconds(0), viewSeconds(0), primaryTickSecs(0)
{
}

void VisibleTimeRange::clear()
{
    timeStart = QDateTime();
    timeEnd = QDateTime();
    viewTimeStart = QDateTime();
    viewTimeEnd = QDateTime();
    dataTimeStart = QDateTime();
    dataTimeEnd = QDateTime();
    timeSeconds = 0;
    viewSeconds = 0;
    primaryTickSecs = 0;
}

double VisibleTimeRange::zoomLevel() const
{
    /* Zoom level of 0 indicates that the entire span of time (visibleTimeRange.timeStart to visibleTimeRange.timeEnd) is
     * visible; visibleTimeRange.timeStart/visibleTimeRange.viewTimeStart and visibleTimeRange.timeEnd/visibleTimeRange.viewTimeEnd are equal. From there, the
     * span (by number of seconds) is scaled up to 100, with 100 indicating maximum zoom,
     * which we define as 1 minute for simplicity. In other words, we scale visibleTimeRange.viewSeconds
     * between 60 and visibleTimeRange.timeSeconds and use the inverse. */
    if (viewSeconds == timeSeconds)
        return 0;
    return 100-((double(viewSeconds-minZoomSeconds())/double(timeSeconds-minZoomSeconds()))*100);
}
