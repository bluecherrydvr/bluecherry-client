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

#ifndef EVENTSVIEW_H
#define EVENTSVIEW_H

#include <QTreeView>

struct EventData;
class EventList;
class EventsModel;
class QLabel;

class EventsView : public QTreeView
{
    Q_OBJECT

public:
    explicit EventsView(QWidget *parent = 0);

    EventsModel *eventsModel() const;

    void setModel(EventsModel *model);

    EventList selectedEvents() const;

private slots:
    void openEvent(const QModelIndex &index);
    void loadingStarted();
    void loadingFinished();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *ev);

private:
    QLabel *loadingIndicator;

    using QTreeView::setModel;
};

#endif // EVENTSVIEW_H
