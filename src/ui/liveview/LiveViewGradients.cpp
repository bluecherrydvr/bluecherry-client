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

#include "LiveViewGradients.h"
#include <QLinearGradient>
#include <QPainter>
#include <QDebug>

LiveViewGradients::LiveViewGradients()
    : QDeclarativeImageProvider(Pixmap)
{
}

QPixmap LiveViewGradients::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap re = QPixmap(1, (requestedSize.height() > 0) ? requestedSize.height() : 20);
    QLinearGradient g(0, 0, 0, re.height());

    if (id == QLatin1String("header"))
    {
        g.setColorAt(0, QColor(0x4c, 0x4c, 0x4c));
        g.setColorAt(0.4, QColor(0x33, 0x33, 0x33));
        g.setColorAt(0.49, QColor(0x26, 0x26, 0x26));
        g.setColorAt(1, QColor(0x19, 0x19, 0x19));
    }
    else if (id == QLatin1String("header/focused"))
    {
        g.setColorAt(0, QColor(0x62, 0x62, 0x62));
        g.setColorAt(0.4, QColor(0x49, 0x49, 0x49));
        g.setColorAt(0.49, QColor(0x3c, 0x3c, 0x3c));
        g.setColorAt(1, QColor(0x2f, 0x2f, 0x2f));
    }
    else if (id == QLatin1String("ptzHeader"))
    {
        g.setColorAt(0, QColor(0x2d, 0x2d, 0x2d));
        g.setColorAt(1, QColor(0x03, 0x03, 0x03));
    }
    else
        return QPixmap();

    QPainter p(&re);
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawRect(0, 0, 1, re.height());
    p.end();

    *size = re.size();
    return re;
}
