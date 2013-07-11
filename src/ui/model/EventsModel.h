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

#ifndef EVENTSMODEL_H
#define EVENTSMODEL_H

#include <QAbstractItemModel>

class DVRServer;
class DVRServerRepository;
class EventData;

class EventsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum
    {
        EventDataPtr = Qt::UserRole
    };

    enum
    {
        ServerColumn = 0,
        LocationColumn,
        TypeColumn,
        DurationColumn,
        LevelColumn,
        DateColumn,

        LastColumn = DateColumn
    };

    explicit EventsModel(DVRServerRepository *serverRepository, QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    void setServerEvents(DVRServer *server, const QList<EventData *> &events);
    void clearServerEvents(DVRServer *server);

private slots:
    void serverAdded(DVRServer *server);

private:
    DVRServerRepository *m_serverRepository;

    QList<EventData *> m_items;
    QMap<DVRServer *, QPair<int, int> > m_serverEventsBoundaries;
    QMap<DVRServer *, int> m_serverEventsCount;

    void computeBoundaries();

};

#endif // EVENTSMODEL_H

