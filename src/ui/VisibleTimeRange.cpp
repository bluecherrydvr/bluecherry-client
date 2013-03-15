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
    : m_primaryTickSecs(0)
{
}

int VisibleTimeRange::primaryTickSecs() const
{
    return m_primaryTickSecs;
}

QDateTime VisibleTimeRange::viewTimeStart() const
{
    return m_visibleRange.start();
}

QDateTime VisibleTimeRange::timeStart() const
{
    return m_range.start();
}

QDateTime VisibleTimeRange::viewTimeEnd() const
{
    return m_range.end();
}

QDateTime VisibleTimeRange::timeEnd() const
{
    return m_range.end();
}

void VisibleTimeRange::setDataRange(const QDateTime &dataStart, const QDateTime& dataEnd)
{
    m_range = DateTimeRange(dataStart, dataEnd);
    computeVisibleRange();
    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::clear()
{
    m_range = DateTimeRange();
    m_visibleRange = DateTimeRange();
    m_primaryTickSecs = 0;

    emit invisibleSecondsChanged(invisibleSeconds());
    emit primaryTickSecsChanged(m_primaryTickSecs);
}

double VisibleTimeRange::zoomLevel() const
{
    /* Zoom level of 0 indicates that the entire span of time (visibleTimeRange.timeStart to visibleTimeRange.timeEnd) is
     * visible; visibleTimeRange.timeStart/visibleTimeRange.viewTimeStart and visibleTimeRange.timeEnd/visibleTimeRange.viewTimeEnd are equal. From there, the
     * span (by number of seconds) is scaled up to 100, with 100 indicating maximum zoom,
     * which we define as 1 minute for simplicity. In other words, we scale visibleTimeRange.viewSeconds
     * between 60 and visibleTimeRange.timeSeconds and use the inverse. */
    if (visibleSeconds() == maxVisibleSeconds())
        return 0;
    return 100 - ((double(visibleSeconds() - minVisibleSeconds()) / double(maxVisibleSeconds() - minVisibleSeconds())) * 100);
}

void VisibleTimeRange::setZoomSeconds(int seconds)
{
    seconds = qBound(minVisibleSeconds(), seconds, maxVisibleSeconds());
    if (visibleSeconds() == seconds)
        return;

    QDateTime visibleStart = m_visibleRange.start();
    QDateTime visibleEnd = visibleStart.addSecs(seconds);

    if (visibleEnd > m_range.end())
    {
        visibleEnd = m_range.end();
        visibleStart = visibleEnd.addSecs(-seconds);
        if (visibleStart < m_range.start())
            visibleStart = m_range.start();
    }

    m_visibleRange = DateTimeRange(visibleStart, visibleEnd);
    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::setViewStartOffset(int secs)
{
    secs = qBound(0, secs, maxVisibleSeconds() - visibleSeconds());

    QDateTime visibleStart = m_range.start().addSecs(secs);
    QDateTime visibleEnd = visibleStart.addSecs(visibleSeconds());

    m_visibleRange = DateTimeRange(visibleStart, visibleEnd);
}

int VisibleTimeRange::invisibleSeconds() const
{
    return qMax(maxVisibleSeconds() - visibleSeconds(), 0);
}

void VisibleTimeRange::computePrimaryTickSecs(int areaWidth, int minTickWidth)
{
    int minTickSecs = qMax(int(visibleSeconds() / (double(areaWidth) / minTickWidth)), 1);

    if (minTickSecs <= 30)
        m_primaryTickSecs = 30;
    else if (minTickSecs <= 60)
        m_primaryTickSecs = 60;
    else if (minTickSecs <= 300)
        m_primaryTickSecs = 300;
    else if (minTickSecs <= 600)
        m_primaryTickSecs = 600;
    else if (minTickSecs <= 3600)
        m_primaryTickSecs = 3600;
    else if (minTickSecs <= 7200)
        m_primaryTickSecs = 7200;
    else if (minTickSecs <= 21600)
        m_primaryTickSecs = 21600;
    else if (minTickSecs <= 43200)
        m_primaryTickSecs = 43200;
    else if (minTickSecs <= 86400)
        m_primaryTickSecs = 86400;
    else
        m_primaryTickSecs = 604800;

    emit primaryTickSecsChanged(m_primaryTickSecs);
}

void VisibleTimeRange::computeVisibleRange()
{
    if (!m_visibleRange.isNull())
        m_visibleRange = m_visibleRange.boundedBy(m_range);
    else
        m_visibleRange = m_range;

    emit invisibleSecondsChanged(invisibleSeconds());
}

int VisibleTimeRange::visibleSeconds() const
{
    return m_visibleRange.lengthInSeconds();
}

int VisibleTimeRange::minVisibleSeconds() const
{
    return qMin(maxVisibleSeconds(), 60);
}

int VisibleTimeRange::maxVisibleSeconds() const
{
    return m_range.lengthInSeconds();
}

void VisibleTimeRange::ensureViewTimeSpan()
{
    QDateTime visibleStart = m_visibleRange.start();
    QDateTime visibleEnd = m_visibleRange.end();

    if (visibleStart.isNull())
        visibleStart = m_range.start();
    if (visibleEnd.isNull())
        visibleEnd = m_range.end();

    if (visibleStart < m_range.start())
    {
        visibleEnd = visibleEnd.addSecs(visibleStart.secsTo(m_range.start()));
        visibleStart = m_range.start();
    }

    if (visibleEnd > m_range.end())
    {
        visibleStart = qMax(m_range.start(), visibleStart.addSecs(visibleEnd.secsTo(m_range.end())));
        visibleEnd = m_range.end();
    }

    m_visibleRange = DateTimeRange(visibleStart, visibleEnd);

    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::addDate(const QDateTime& date)
{
    m_range.extendTo(date);
}
