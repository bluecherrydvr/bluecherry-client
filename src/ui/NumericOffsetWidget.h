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

#ifndef NUMERICOFFSETWIDGET_H
#define NUMERICOFFSETWIDGET_H

#include <QWidget>

class NumericOffsetWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged STORED true USER true)

public:
    explicit NumericOffsetWidget(QWidget *parent = 0);

    int value() const { return m_value; }

    virtual QSize sizeHint() const;

public slots:
    void setValue(int value);
    void clear() { setValue(0); }
    void increase() { setValue(value()+1); }
    void decrease() { setValue(value()-1); }

signals:
    void valueChanged(int value);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private:
    int m_value;

    QSize textAreaSize() const;
};

#endif // NUMERICOFFSETWIDGET_H
