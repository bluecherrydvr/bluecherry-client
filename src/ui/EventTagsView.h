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

#ifndef EVENTTAGSVIEW_H
#define EVENTTAGSVIEW_H

#include <QListView>

class EventTagsView : public QListView
{
    Q_OBJECT

public:
    explicit EventTagsView(QWidget *parent = 0);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    virtual void setModel(QAbstractItemModel *model);
    virtual void reset();

public slots:
    void removeTag(const QModelIndex &index);

protected:
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    QSize cachedSizeHint;

    void calculateSizeHint();
};

#endif // EVENTTAGSVIEW_H
