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

#ifndef EVENTTIMELINEDATEPAINTER_H
#define EVENTTIMELINEDATEPAINTER_H

#include <QDate>

class QPainter;
class QRect;

class EventTimelineDatePainter
{
public:
    explicit EventTimelineDatePainter(const QDate &startDate, const QDate &endDate, int leftPadding, const QDateTime &visibleTimeStart, double pixelsPerSecondRatio);

    int paintDates(QPainter &painter, const QRect &rect, int yPos);

private:
    QDate m_startDate;
    QDate m_endDate;
    int m_leftPadding;
    QDateTime m_visibleTimeStart;
    double m_pixelsPerSecondRatio;

};

#endif // EVENTTIMELINEDATEPAINTER_H
