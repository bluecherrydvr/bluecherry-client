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

#ifndef MACSPLITTERHANDLE_H
#define MACSPLITTERHANDLE_H

#include <QSplitterHandle>

class MacSplitterHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    MacSplitterHandle(Qt::Orientation o, QSplitter *parent);
    ~MacSplitterHandle();

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual bool eventFilter(QObject *, QEvent *);

private:
    bool isMouseGrabbed;
};

#endif // MACSPLITTERHANDLE_H
