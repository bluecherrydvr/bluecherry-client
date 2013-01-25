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

#include "EventCommentsWidget.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QDateTime>

EventCommentsWidget::EventCommentsWidget(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
}

QSize EventCommentsWidget::minimumSizeHint() const
{
    return QSize();
}

void EventCommentsWidget::appendComment(const QString &author, const QDateTime &date, const QString &text)
{
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::End);
    if (!cursor.atBlockStart())
        cursor.insertBlock();

    cursor.insertHtml(QString::fromLatin1("<table width='100%' cellspacing=0 style='margin-bottom:6px;'><tr>"
                                          "<td style='font-weight:bold'>%1</td><td align='right'>%2</td></tr>"
                                          "</table><br>%3<hr>").arg(Qt::escape(author), date.toString(),
                                                                    Qt::escape(text)));
}
