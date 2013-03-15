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
    computeVisibleRange();
    emit invisibleSecondsChanged(invisibleSeconds());
}
