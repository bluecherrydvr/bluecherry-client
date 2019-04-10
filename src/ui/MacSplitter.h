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

#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>

class MacSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit MacSplitter(Qt::Orientation o, QWidget *parent = 0);

protected:
    virtual QSplitterHandle *createHandle();

};

#endif // MACSPLITTER_H
