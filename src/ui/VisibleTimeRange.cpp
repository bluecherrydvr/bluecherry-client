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

void VisibleTimeRange::setDataRange(const QDateTime &dataStart, const QDateTime& dataEnd)
{
    dataTimeStart = dataStart;
    dataTimeEnd = dataEnd;
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

void VisibleTimeRange::setZoomSeconds(int seconds)
{
    seconds = qBound(minZoomSeconds(), seconds, maxZoomSeconds());
    if (viewSeconds == seconds)
        return;

    Q_ASSERT(!viewTimeStart.isNull());
    Q_ASSERT(viewTimeStart >= timeStart);

    viewSeconds = seconds;
    viewTimeEnd = viewTimeStart.addSecs(seconds);
    if (viewTimeEnd > timeEnd)
    {
        viewTimeStart = viewTimeStart.addSecs(viewTimeEnd.secsTo(timeEnd));
        viewTimeEnd = timeEnd;
    }

    Q_ASSERT(viewTimeEnd > viewTimeStart);
    Q_ASSERT(viewTimeStart >= timeStart);
    Q_ASSERT(viewTimeEnd <= timeEnd);
    Q_ASSERT(viewTimeStart.secsTo(viewTimeEnd) == viewSeconds);
}

void VisibleTimeRange::setViewStartOffset(int secs)
{
    secs = qBound(0, secs, timeSeconds - viewSeconds);

    viewTimeStart = timeStart.addSecs(secs);
    viewTimeEnd = viewTimeStart.addSecs(viewSeconds);

    Q_ASSERT(viewSeconds <= timeSeconds);
    Q_ASSERT(viewSeconds >= minZoomSeconds());
    Q_ASSERT(viewTimeEnd <= timeEnd);
}

int VisibleTimeRange::invisibleSeconds() const
{
    return qMax(timeSeconds - viewSeconds, 0);
}

void VisibleTimeRange::computeViewSeconds()
{
    /* Approximate viewSeconds for the tick calculations */
    if (viewTimeStart.isNull() || viewTimeEnd.isNull())
        viewSeconds = dataTimeStart.secsTo(dataTimeEnd);
    else
        viewSeconds = qMin(viewSeconds, dataTimeStart.secsTo(dataTimeEnd));
}

void VisibleTimeRange::computePrimaryTickSecs(int areaWidth, int minTickWidth)
{
    int minTickSecs = qMax(int(viewSeconds / (double(areaWidth) / minTickWidth)), 1);

    if (minTickSecs <= 30)
        primaryTickSecs = 30;
    else if (minTickSecs <= 60)
        primaryTickSecs = 60;
    else if (minTickSecs <= 300)
        primaryTickSecs = 300;
    else if (minTickSecs <= 600)
        primaryTickSecs = 600;
    else if (minTickSecs <= 3600)
        primaryTickSecs = 3600;
    else if (minTickSecs <= 7200)
        primaryTickSecs = 7200;
    else if (minTickSecs <= 21600)
        primaryTickSecs = 21600;
    else if (minTickSecs <= 43200)
        primaryTickSecs = 43200;
    else if (minTickSecs <= 86400)
        primaryTickSecs = 86400;
    else
        primaryTickSecs = 604800;
}

void VisibleTimeRange::computeTimeStart()
{
    timeStart = dataTimeStart.addSecs(-int(dataTimeStart.toTime_t() % primaryTickSecs));
}

void VisibleTimeRange::computeTimeEnd()
{
    timeEnd = dataTimeEnd.addSecs(primaryTickSecs - int(dataTimeEnd.toTime_t() % primaryTickSecs));
}

void VisibleTimeRange::computeTimeSeconds()
{
    timeSeconds = timeStart.secsTo(timeEnd);
}

void VisibleTimeRange::ensureViewTimeSpan()
{
    if (viewTimeStart.isNull())
        viewTimeStart = timeStart;
    if (viewTimeEnd.isNull())
        viewTimeEnd = timeEnd;

    if (viewTimeStart < timeStart)
    {
        viewTimeEnd = viewTimeEnd.addSecs(viewTimeStart.secsTo(timeStart));
        viewTimeStart = timeStart;
    }

    if (viewTimeEnd > timeEnd)
    {
        viewTimeStart = qMax(timeStart, viewTimeStart.addSecs(viewTimeEnd.secsTo(timeEnd)));
        viewTimeEnd = timeEnd;
    }

    Q_ASSERT(viewTimeStart >= timeStart);
    Q_ASSERT(viewTimeEnd <= timeEnd);

    viewSeconds = viewTimeStart.secsTo(viewTimeEnd);
}
