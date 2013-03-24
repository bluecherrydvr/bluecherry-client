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

#include "EventTimelineDatePainter.h"
#include "utils/DateTimeRange.h"
#include <QDateTime>
#include <QPainter>

QLatin1String EventTimelineDatePainter::longDateFormat("ddd, MMM d yyyy");
QLatin1String EventTimelineDatePainter::shortDateFormat("ddd, MMM d");

EventTimelineDatePainter::EventTimelineDatePainter(const QDate &startDate, const QDate &endDate, const QDateTime &visibleTimeStart, double pixelsPerSecondRatio)
    : m_startDate(startDate), m_endDate(endDate), m_visibleTimeStart(visibleTimeStart), m_pixelsPerSecondRatio(pixelsPerSecondRatio)
{
}

int EventTimelineDatePainter::paintDates(QPainter &painter, const QRect &rect, int yPos)
{
    int resultYPos = yPos;

    m_useLongDateFormat = true;

    for (QDate date = m_startDate; date <= m_endDate; date = date.addDays(1))
        resultYPos = qMax(resultYPos, paintDate(painter, date));

    return resultYPos;
}

int EventTimelineDatePainter::paintDate(QPainter &painter, const QDate &date)
{
    QDateTime dt = QDateTime(date, QTime(), Qt::UTC);

    if (m_visibleTimeStart >= dt.addDays(1))
        return -1;

    QString dateStr = dateToString(date);
    QRect dateRect = dateStringToRect(date, dateStr, painter.fontMetrics());

    int result = -1;
    if (m_lastDrawnDateRect.intersect(dateRect).isEmpty())
    {
        painter.drawText(dateRect, Qt::AlignLeft | Qt::TextDontClip, dateStr);
        m_lastDrawnDateRect = dateRect;
        result = qMax(result, dateRect.bottom());

        if (!m_undrawnDateString.isEmpty() && m_undrawnDateRect.intersect(m_lastDrawnDateRect).isEmpty())
            painter.drawText(m_undrawnDateRect, Qt::AlignLeft | Qt::TextDontClip, m_undrawnDateString);

        m_undrawnDateRect = QRect();
        m_undrawnDateString.clear();
    }
    else
    {
        m_undrawnDateRect = dateRect.translated(m_lastDrawnDateRect.right() - dateRect.left(), 0);
        m_undrawnDateString = dateStr;
    }

    m_useLongDateFormat = false;

    return result;
}

QString EventTimelineDatePainter::dateToString(const QDate &date)
{
    if (m_useLongDateFormat)
        return date.toString(longDateFormat);
    else
        return date.toString(shortDateFormat);
}

QRect EventTimelineDatePainter::dateStringToRect(const QDate &date, const QString &dateString, const QFontMetrics &fontMetrics)
{
    QRect result;

    result.setTop(0);
    result.setLeft(qMax(0, qRound(m_pixelsPerSecondRatio * m_visibleTimeStart.secsTo(QDateTime(date, QTime(), Qt::UTC)))));
    result.setWidth(fontMetrics.width(dateString) + 15);
    result.setHeight(fontMetrics.height());

    return result;
}
