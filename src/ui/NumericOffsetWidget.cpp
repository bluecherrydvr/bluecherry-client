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

#include "NumericOffsetWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

NumericOffsetWidget::NumericOffsetWidget(QWidget *parent)
    : QWidget(parent), m_value(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void NumericOffsetWidget::setValue(int value)
{
    if (m_value == value)
        return;

    m_value = value;
    emit valueChanged(value);
    update();
}

QSize NumericOffsetWidget::textAreaSize() const
{
    QFontMetrics fm(font());
    return fm.boundingRect(QLatin1String("-999")).size() + QSize(6, 0);
}

QSize NumericOffsetWidget::sizeHint() const
{
    const int iconSize = 16;
    QSize textSize = textAreaSize();

    return QSize(textSize.width() + (iconSize*2), qMax(iconSize, textSize.height()));
}

void NumericOffsetWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect r = event->rect();

    QSize textSize = textAreaSize();

    int textLeft = (r.width() - textSize.width()) / 2;

    /* No icons yet; draw - and + instead */
    p.drawText(QRect(textLeft - 16, 0, 16, r.height()), Qt::AlignVCenter | Qt::AlignRight, QLatin1String("-"));
    p.drawText(QRect(textLeft + textSize.width(), 0, 16, r.height()), Qt::AlignVCenter | Qt::AlignLeft, QLatin1String("+"));

    /* Text */
    p.drawText(QRect(textLeft, 0, textSize.width(), r.height()), Qt::AlignCenter, QString::number(value()));
}

void NumericOffsetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    int x = event->pos().x();
    int mid = width()/2;

    if (qAbs(x - mid) < 10)
        clear();
    else if (x < mid)
        decrease();
    else
        increase();

    event->accept();
}
