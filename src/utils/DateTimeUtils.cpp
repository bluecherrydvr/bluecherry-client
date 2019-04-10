/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#include "DateTimeUtils.h"
#include <QDateTime>
#include <QLatin1Char>

QDateTime isoToDateTime(const QString &str, qint16 *tzOffsetMins)
{
    /* If there is a - or a + after the T portion, it indicates timezone, and should be removed */
    int tzpos = -1;
    for (int i = str.size()-1; i && tzpos == -1; --i)
    {
        switch (str[i].toLatin1())
        {
        case '-':
        case '+':
            tzpos = i;
            break;
        case 'T':
        case 'Z':
            tzpos = -2;
            break;
        }
    }

    qint16 offset = 0;
    if (tzpos > 0)
    {
        /* Starting from tzpos, this can be in any of the following formats:
         *   +hh   +hh:mm   +hhmm   + may also be -                         */
        int p = tzpos;
        bool positive = (str[p] == QLatin1Char('+'));
        p++;

        offset = (qint16)str.mid(p, 2).toUShort() * 60;
        p += 2;

        if (p < str.size() && str[p] == QLatin1Char(':'))
            ++p;
        offset += (qint16)str.mid(p, 2).toUShort();

        if (!positive)
            offset = -offset;
    }

    if (tzOffsetMins)
        *tzOffsetMins = offset;

    QDateTime re = QDateTime::fromString(str.mid(0, tzpos), Qt::ISODate);
    re.setTimeSpec(Qt::UTC);
    return re.addSecs(int(-offset)*60);
}
