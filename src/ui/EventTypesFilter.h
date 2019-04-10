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

#ifndef EVENTTYPESFILTER_H
#define EVENTTYPESFILTER_H

#include <QTreeWidget>
#include <QBitArray>

class EventType;

class EventTypesFilter : public QTreeWidget
{
    Q_OBJECT

public:
    explicit EventTypesFilter(QWidget *parent = 0);

    QBitArray checkedTypes() const;

signals:
    void checkedTypesChanged(const QBitArray &checkedTypes);

protected:
	virtual void changeEvent(QEvent *event);


private slots:
    void checkStateChanged(QTreeWidgetItem *item);

private:
    QTreeWidgetItem *addItem(const QString &name, EventType min, EventType max);
};

#endif // EVENTTYPESFILTER_H
