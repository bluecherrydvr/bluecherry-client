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

#ifndef DVRSERVERSVIEW_H
#define DVRSERVERSVIEW_H

#include <QTreeView>

class DVRServer;
class DVRServerRepository;

class DVRServersView : public QTreeView
{
    Q_OBJECT

public:
    explicit DVRServersView(DVRServerRepository *serverRepository, QWidget *parent = 0);
    virtual ~DVRServersView();

    DVRServer *currentServer() const;

    void checkOnlyIndex(const QModelIndex &index);

    virtual void setModel(QAbstractItemModel *model);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private slots:
    void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);

private:
    DVRServerRepository *m_serverRepository;

};

#endif // DVRSERVERSVIEW_H
