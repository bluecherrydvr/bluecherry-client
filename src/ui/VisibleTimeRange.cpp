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
    : m_timeSeconds(0), m_viewSeconds(0), m_primaryTickSecs(0)
{
}

int VisibleTimeRange::viewSeconds() const
{
    return m_viewSeconds;
}

int VisibleTimeRange::primaryTickSecs() const
{
    return m_primaryTickSecs;
}

QDateTime VisibleTimeRange::viewTimeStart() const
{
    return m_viewTimeStart;
}

QDateTime VisibleTimeRange::timeStart() const
{
    return m_timeStart;
}

QDateTime VisibleTimeRange::dataTimeStart() const
{
    return m_dataTimeStart;
}

QDateTime VisibleTimeRange::viewTimeEnd() const
{
    return m_viewTimeEnd;
}

QDateTime VisibleTimeRange::timeEnd() const
{
    return m_timeEnd;
}

QDateTime VisibleTimeRange::dataTimeEnd() const
{
    return m_dataTimeEnd;
}

void VisibleTimeRange::setDataRange(const QDateTime &dataStart, const QDateTime& dataEnd)
{
    m_dataTimeStart = dataStart;
    m_dataTimeEnd = dataEnd;
}

void VisibleTimeRange::clear()
{
    m_timeStart = QDateTime();
    m_timeEnd = QDateTime();
    m_viewTimeStart = QDateTime();
    m_viewTimeEnd = QDateTime();
    m_dataTimeStart = QDateTime();
    m_dataTimeEnd = QDateTime();
    m_timeSeconds = 0;
    m_viewSeconds = 0;
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
    if (m_viewSeconds == m_timeSeconds)
        return 0;
    return 100 - ((double(m_viewSeconds - minZoomSeconds()) / double(m_timeSeconds - minZoomSeconds())) * 100);
}

void VisibleTimeRange::setZoomSeconds(int seconds)
{
    seconds = qBound(minZoomSeconds(), seconds, maxZoomSeconds());
    if (m_viewSeconds == seconds)
        return;

    m_viewSeconds = seconds;
    m_viewTimeEnd = m_viewTimeStart.addSecs(seconds);
    if (m_viewTimeEnd > m_timeEnd)
    {
        m_viewTimeStart = m_viewTimeStart.addSecs(m_viewTimeEnd.secsTo(m_timeEnd));
        m_viewTimeEnd = m_timeEnd;
    }

    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::setViewStartOffset(int secs)
{
    secs = qBound(0, secs, m_timeSeconds - m_viewSeconds);

    m_viewTimeStart = m_timeStart.addSecs(secs);
    m_viewTimeEnd = m_viewTimeStart.addSecs(m_viewSeconds);
}

int VisibleTimeRange::invisibleSeconds() const
{
    return qMax(m_timeSeconds - m_viewSeconds, 0);
}

void VisibleTimeRange::computeViewSeconds()
{
    /* Approximate viewSeconds for the tick calculations */
    if (m_viewTimeStart.isNull() || m_viewTimeEnd.isNull())
        m_viewSeconds = m_dataTimeStart.secsTo(m_dataTimeEnd);
    else
        m_viewSeconds = qMin(m_viewSeconds, m_dataTimeStart.secsTo(m_dataTimeEnd));

    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::computePrimaryTickSecs(int areaWidth, int minTickWidth)
{
    int minTickSecs = qMax(int(m_viewSeconds / (double(areaWidth) / minTickWidth)), 1);

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

void VisibleTimeRange::computeTimeStart()
{
    m_timeStart = m_dataTimeStart.addSecs(-int(m_dataTimeStart.toTime_t() % m_primaryTickSecs));
}

void VisibleTimeRange::computeTimeEnd()
{
    m_timeEnd = m_dataTimeEnd.addSecs(m_primaryTickSecs - int(m_dataTimeEnd.toTime_t() % m_primaryTickSecs));
}

void VisibleTimeRange::computeTimeSeconds()
{
    m_timeSeconds = m_timeStart.secsTo(m_timeEnd);
    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::ensureViewTimeSpan()
{
    if (m_viewTimeStart.isNull())
        m_viewTimeStart = m_timeStart;
    if (m_viewTimeEnd.isNull())
        m_viewTimeEnd = m_timeEnd;

    if (m_viewTimeStart < m_timeStart)
    {
        m_viewTimeEnd = m_viewTimeEnd.addSecs(m_viewTimeStart.secsTo(m_timeStart));
        m_viewTimeStart = m_timeStart;
    }

    if (m_viewTimeEnd > m_timeEnd)
    {
        m_viewTimeStart = qMax(m_timeStart, m_viewTimeStart.addSecs(m_viewTimeEnd.secsTo(m_timeEnd)));
        m_viewTimeEnd = m_timeEnd;
    }

    m_viewSeconds = m_viewTimeStart.secsTo(viewTimeEnd());

    emit invisibleSecondsChanged(invisibleSeconds());
}

void VisibleTimeRange::addDate(const QDateTime& date)
{
    if (m_dataTimeStart.isNull() || date < m_dataTimeStart)
        m_dataTimeStart = date;
    if (m_dataTimeEnd.isNull() || date > m_dataTimeEnd)
        m_dataTimeEnd = date;
}
