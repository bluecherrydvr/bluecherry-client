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
#include <QRect>

class QFontMetrics;
class QPainter;

class EventTimelineDatePainter
{
public:
    explicit EventTimelineDatePainter(QPainter &painter);

    void setStartDate(const QDate &startDate);
    void setEndDate(const QDate &endDate);
    void setVisibleTimeStart(const QDateTime &visibleTimeStart);
    void setPixelsPerSecondRatio(double pixelsPerSecondRatio);

    void paintDates();

private:
    static QLatin1String longDateFormat;
    static QLatin1String shortDateFormat;

    QPainter &m_painter;

    QDate m_startDate;
    QDate m_endDate;
    QDateTime m_visibleTimeStart;
    double m_pixelsPerSecondRatio;

    int m_fontHeight;
    bool m_useLongDateFormat;
    QRect m_lastDrawnDateRect;
    QRect m_undrawnDateRect;
    QString m_undrawnDateString;

    void paintDate(const QDate &date);
    QString dateToString(const QDate &date);
    QRect dateStringToRect(const QDate &date, const QString &dateString);
    bool isRectUnused(const QRect &rect);
    void paintDateString(const QString &dateString, const QRect &dateRect);
    bool shouldPaintPreviousDate();

};

#endif // EVENTTIMELINEDATEPAINTER_H
