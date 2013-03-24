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

EventTimelineDatePainter::EventTimelineDatePainter(const QDate &startDate, const QDate &endDate, const QDateTime &visibleTimeStart, double pixelsPerSecondRatio)
    : m_startDate(startDate), m_endDate(endDate), m_visibleTimeStart(visibleTimeStart), m_pixelsPerSecondRatio(pixelsPerSecondRatio)
{
}

int EventTimelineDatePainter::paintDates(QPainter &painter, const QRect &rect, int yPos)
{
    int resultYPos = yPos;

    /* Dates across the top; first one is fully qualified (space permitting) */
    painter.save();
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.setBrush(Qt::NoBrush);

    bool first = true;

    QRect lastDrawnDateRect;
    QRect undrawnDateRect;
    QString undrawnDateString;

    // because of UTC/local time differences we need to start with previous date
    for (QDate date = m_startDate.addDays(-1); date <= m_endDate; date = date.addDays(1))
    {
        QDateTime dt = QDateTime(date);
        dt.setTimeSpec(Qt::UTC);

        if (m_visibleTimeStart < dt.addDays(1))
        {
            QString dateStr = date.toString(first ? QLatin1String("ddd, MMM d yyyy") : QLatin1String("ddd, MMM d"));

            QRect dateRect;
            dateRect.setTop(0);
            dateRect.setLeft(qMax(0, qRound(m_pixelsPerSecondRatio * m_visibleTimeStart.secsTo(dt))));
            dateRect.setWidth(painter.fontMetrics().width(dateStr) + 15);
            dateRect.setHeight(painter.fontMetrics().height());

            if (lastDrawnDateRect.intersect(dateRect).isEmpty())
            {
                painter.drawText(dateRect, Qt::AlignLeft | Qt::TextDontClip, dateStr);
                lastDrawnDateRect = dateRect;
                resultYPos = qMax(yPos, dateRect.bottom());

                if (!undrawnDateString.isEmpty() && undrawnDateRect.intersect(lastDrawnDateRect).isEmpty())
                    painter.drawText(undrawnDateRect, Qt::AlignLeft | Qt::TextDontClip, undrawnDateString);

                undrawnDateRect = QRect();
                undrawnDateString.clear();
            }
            else
            {
                undrawnDateRect = dateRect.translated(lastDrawnDateRect.right() - dateRect.left(), 0);
                undrawnDateString = dateStr;
            }

            first = false;
        }
    }
    painter.restore();

    return resultYPos;
}
