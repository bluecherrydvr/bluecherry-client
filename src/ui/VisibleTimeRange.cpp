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
#include <QtAlgorithms>

VisibleTimeRange::VisibleTimeRange()
    : m_primaryTickSecs(0)
{
    m_tickSizes << 30 << 60 << 300 << 600 << 3600 << 7200 << 21600 << 43200 << 86400 << 604800;
}

int VisibleTimeRange::primaryTickSecs() const
{
    return m_primaryTickSecs;
}

void VisibleTimeRange::setDateTimeRange(const DateTimeRange &dateTimeRange)
{
    m_range = dateTimeRange;
    boundVisibleRange(m_range);
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

void VisibleTimeRange::setZoomLevel(int zoomLevel)
{
    if (zoomLevel != 0)
    {
        int maxMinVisibleSecondsDifference = maxVisibleSeconds() - minVisibleSeconds();
        int visibleSeconds = double(maxMinVisibleSecondsDifference) * (1.0 - (double)zoomLevel/100.0) + minVisibleSeconds();
        m_visibleRange = m_visibleRange.withLengthInSeconds(visibleSeconds).moveInto(m_range);
    }
    else
        m_visibleRange = m_range;
    
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

void VisibleTimeRange::computePrimaryTickSecs(int prefferedTickCount)
{
    int minTickSecs = qMax(visibleSeconds() / prefferedTickCount, 1);
    QList<int>::const_iterator tickIterator = qLowerBound(m_tickSizes, minTickSecs);
    if (tickIterator == m_tickSizes.constEnd())
        m_primaryTickSecs = *m_tickSizes.end();
    else
        m_primaryTickSecs = *tickIterator;

    emit primaryTickSecsChanged(m_primaryTickSecs);
}

void VisibleTimeRange::boundVisibleRange(const DateTimeRange &boundBy)
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
