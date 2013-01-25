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

#include "ExpandingTextEdit.h"

ExpandingTextEdit::ExpandingTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(document(), SIGNAL(contentsChanged()), SLOT(documentChanged()));
}

QSize ExpandingTextEdit::sizeHint() const
{
    QSize sz = document()->size().toSize();
    QRect cr = contentsRect();
    sz += QSize(cr.x()*2, cr.y()*2);
    return sz;
}

QSize ExpandingTextEdit::minimumSizeHint() const
{
    return QSize();
}

void ExpandingTextEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    document()->setTextWidth(viewport()->width());
    updateGeometry();
}
