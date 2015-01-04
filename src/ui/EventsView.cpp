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

#include "EventsView.h"
#include "model/EventsModel.h"
#include "model/EventsProxyModel.h"
#include "EventViewWindow.h"
#include "core/EventData.h"
#include "event/EventList.h"
#include <QHeaderView>
#include <QMovie>
#include <QLabel>
#include <QEvent>

EventsView::EventsView(QWidget *parent)
    : QTreeView(parent), loadingIndicator(0), m_eventsModel(0)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);

    viewport()->installEventFilter(this);

    m_eventsProxyModel = new EventsProxyModel(this);
    m_eventsProxyModel->setColumn(EventsModel::DateColumn);
    m_eventsProxyModel->setDynamicSortFilter(true);
    m_eventsProxyModel->sort(0, Qt::DescendingOrder);
}

void EventsView::setModel(EventsModel *model, bool loading)
{
    bool first = !m_eventsModel;

    m_eventsModel = model;
    m_eventsProxyModel->setSourceModel(m_eventsModel);
    QTreeView::setModel(m_eventsProxyModel);

    if (first)
    {
        header()->setResizeMode(QHeaderView::Interactive);
        QFontMetrics fm(font());
        header()->resizeSection(EventsModel::LocationColumn, fm.width(QLatin1Char('X')) * 20);
        header()->resizeSection(EventsModel::DurationColumn,
                                fm.width(QLatin1String("99 minutes, 99 seconds")) + 25);
        header()->resizeSection(EventsModel::LevelColumn, fm.width(QLatin1String("Warning")) + 18);
    }

    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), SLOT(loadingFinished()));
    connect(model, SIGNAL(modelReset()), SLOT(loadingFinished()));

    if (loading)
        loadingStarted();
}


EventList EventsView::selectedEvents() const
{
    EventList result;

    const QModelIndexList &selectedItems = selectionModel()->selectedRows();
    foreach (const QModelIndex &selectedItem, selectedItems)
    {
        EventData *eventData = selectedItem.data(EventsModel::EventDataPtr).value<EventData*>();
        if (eventData)
            result.append(*eventData);
    }

    return result;
}

void EventsView::setIncompletePlace(EventsProxyModel::IncompletePlace incompletePlace)
{
    m_eventsProxyModel->setIncompletePlace(incompletePlace);
}

void EventsView::setMinimumLevel(EventLevel minimumLevel)
{
    m_eventsProxyModel->setMinimumLevel(minimumLevel);
}

void EventsView::setTypes(QBitArray types)
{
    m_eventsProxyModel->setTypes(types);
}

void EventsView::setDay(const QDate &day)
{
    m_eventsProxyModel->setDay(day);
}

void EventsView::setSources(const QMap<DVRServer*, QSet<int> > &sources)
{
    m_eventsProxyModel->setSources(sources);
}

void EventsView::sortEvents(int logicalIndex, Qt::SortOrder sortOrder)
{
    m_eventsProxyModel->setDynamicSortFilter(false);
    m_eventsProxyModel->setColumn(logicalIndex);
    m_eventsProxyModel->sort(0, sortOrder);
    m_eventsProxyModel->setDynamicSortFilter(true);
}

void EventsView::openEvent(const QModelIndex &index)
{
    EventData *event = index.data(EventsModel::EventDataPtr).value<EventData*>();
    if (!event)
        return;

    EventViewWindow::open(*event, 0);
}

void EventsView::loadingStarted()
{
    /* The loading indicator is only displayed when no rows are visible */
    if (model()->rowCount())
        return;

    if (!loadingIndicator)
    {
        loadingIndicator = new QLabel(viewport());
        Q_ASSERT(QMovie::supportedFormats().contains("gif"));
        QMovie *m = new QMovie(QLatin1String(":/images/loading.gif"), "gif", loadingIndicator);
        loadingIndicator->setMovie(m);
        m->start();
    }

    QSize size = loadingIndicator->sizeHint();
    QRect cr = viewport()->contentsRect();

    loadingIndicator->setGeometry((cr.width()-size.width())/2,
                                  size.width()/4, size.width(), size.height());
    loadingIndicator->show();
}

void EventsView::loadingFinished()
{
    delete loadingIndicator;
    loadingIndicator = 0;
}

bool EventsView::eventFilter(QObject *obj, QEvent *ev)
{
    Q_UNUSED(obj);

    Q_ASSERT(obj == viewport());
    if (ev->type() == QEvent::Resize && loadingIndicator)
    {
        QSize size = loadingIndicator->sizeHint();
        QRect cr = viewport()->contentsRect();

        loadingIndicator->setGeometry((cr.width()-size.width())/2,
                                      size.width()/4, size.width(), size.height());
    }

    return false;
}
