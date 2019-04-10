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

#ifndef EVENTSCURSOR_H
#define EVENTSCURSOR_H

#include <QObject>

class EventData;

class EventsCursor : public QObject
{
    Q_OBJECT

public:
    explicit EventsCursor(QObject *parent = 0);
    virtual ~EventsCursor();

    virtual EventData * current() const = 0;
    virtual bool hasNext() = 0;
    virtual bool hasPrevious() = 0;

public slots:
    virtual void moveToNext() = 0;
    virtual void moveToPrevious() = 0;

signals:
    void eventSwitched(EventData *event);

    void nextAvailabilityChanged(bool hasNext);
    void previousAvailabilityChanged(bool hasPrevious);

protected slots:
    void emitAvailabilitySignals();
};

#endif // EVENTSCURSOR_H
