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

#include "StringUtils.h"
#include <QApplication>

QString byteSizeString(quint64 bytes, ByteSizeFormat format)
{
    double v = bytes / double(1024);
    int n = 1;

    for (; v >= 1024; n++)
        v /= 1024;

    QString suffix;
    if (format == Bytes)
    {
        switch (n)
        {
        case 1: suffix = QApplication::translate("byteSizeString", "KB"); break;
        case 2: suffix = QApplication::translate("byteSizeString", "MB"); break;
        case 3: suffix = QApplication::translate("byteSizeString", "GB"); break;
        }
    }
    else if (format == BytesPerSecond)
    {
        switch (n)
        {
        case 1: suffix = QApplication::translate("byteSizeString", "KB/s"); break;
        case 2: suffix = QApplication::translate("byteSizeString", "MB/s"); break;
        case 3: suffix = QApplication::translate("byteSizeString", "GB/s"); break;
        }
    }

    return QString::number(v, 'f', 0) + QLatin1Char(' ') + suffix;
}

QString withSuffix(const QString &string, const QString &suffix)
{
    if (string.endsWith(suffix))
        return string;
    else
        return string + suffix;
}
