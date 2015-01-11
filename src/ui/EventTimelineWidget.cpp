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

#include "EventTimelineWidget.h"
#include "EventTimelineDatePainter.h"
#include "model/EventsModel.h"
#include "TimeRangeScrollBar.h"
#include "core/EventData.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include <QPaintEvent>
#include <QPainter>
#include <QVector>
#include <QScrollBar>
#include <QFontMetrics>
#include <QDebug>
#include <qmath.h>

struct RowData
{
    enum Type
    {
        Server,
        Location
    } const type : 8;
    int y;

    RowData(Type t) : type(t), y(0) { }

    LocationData *toLocation();
    ServerData *toServer();
};

struct ServerData : public RowData
{
    DVRServer *server;
    QHash<int,LocationData*> locationsMap;

    ServerData() : RowData(Server)
    {
    }
};

struct LocationData : public RowData
{
    ServerData *serverData;
    QList<EventData*> events;
    int locationId;

    LocationData() : RowData(Location)
    {
    }

    QString uiLocation() const
    {
        return EventData::uiLocation(serverData->server, locationId);
    }
};

inline LocationData *RowData::toLocation()
{
    return (type == Location) ? static_cast<LocationData*>(this) : 0;
}

inline ServerData *RowData::toServer()
{
    return (type == Server) ? static_cast<ServerData*>(this) : 0;
}

EventTimelineWidget::EventTimelineWidget(QWidget *parent)
    : QAbstractItemView(parent), cachedTopPadding(0),
      cachedLeftPadding(-1), rowsMapUpdateStart(-1), mouseRubberBand(0)
{
    setAutoFillBackground(false);

    TimeRangeScrollBar * timeRangeScrollBar = new TimeRangeScrollBar(this);
    connect(&visibleTimeRange, SIGNAL(invisibleSecondsChanged(int)), timeRangeScrollBar, SLOT(setInvisibleSeconds(int)));
    connect(&visibleTimeRange, SIGNAL(primaryTickSecsChanged(int)), timeRangeScrollBar, SLOT(setPrimaryTickSecs(int)));

    setHorizontalScrollBar(timeRangeScrollBar);
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(setViewStartOffset(int)));
}

EventTimelineWidget::~EventTimelineWidget()
{
    clearData();
}

void EventTimelineWidget::clearData()
{
    foreach (ServerData *server, serversMap)
    {
        qDeleteAll(server->locationsMap);
        delete server;
    }

    serversMap.clear();
    rowsMap.clear();
    visibleTimeRange.clear();

    clearLeftPaddingCache();

    layoutRows.clear();
    layoutRowsBottom = 0;

    pendingLayouts = 0;

    verticalScrollBar()->setRange(0, 0);
    viewport()->update();
}

void EventTimelineWidget::setModel(QAbstractItemModel *newModel)
{
    if (newModel == model())
        return;

    if (model())
    {
        model()->disconnect(this);
        clearData();
    }

    connect(newModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(rowsRemoved(QModelIndex,int,int)));
    /* setModel calls reset(), which will set up the new internal state */
    QAbstractItemView::setModel(newModel);
}

QRect EventTimelineWidget::visualRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    EventData *event = rowData(index.row());
    if (!event)
        return QRect();

    ServerData *serverData;
    LocationData *locationData;
    if (!const_cast<EventTimelineWidget*>(this)->findEvent(event, false, &serverData, &locationData, 0))
        return QRect();

    const_cast<EventTimelineWidget*>(this)->ensureLayout();
    QRect itemArea = viewportItemArea();

    QRect re = timeCellRect(event->localStartDate(), event->durationInSeconds());
    re.translate(itemArea.topLeft());
    re.moveTop(itemArea.top() + locationData->y - verticalScrollBar()->value());
    re.setHeight(rowHeight());

    return re;
}

void EventTimelineWidget::setZoomLevel(int level)
{
    visibleTimeRange.setZoomLevel(level);
    scheduleDelayedItemsLayout(DoUpdateTimeRange);
}

void EventTimelineWidget::setViewStartOffset(int secs)
{
    visibleTimeRange.setViewStartOffset(secs);
    viewport()->update();
}

void EventTimelineWidget::updateScrollBars()
{
    ensureLayout();

    int h = viewportItemArea().height();
    verticalScrollBar()->setRange(0, qMax(0, layoutRowsBottom-h));
    verticalScrollBar()->setPageStep(h);
    verticalScrollBar()->setSingleStep(rowHeight());
}

void EventTimelineWidget::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    QRect itemArea = viewportItemArea();
    QRect itemRect = visualRect(index);
    itemRect.moveTop(itemRect.top() - itemArea.top());

    EventData *event = rowData(index.row());

    switch (hint)
    {
    case EnsureVisible:
        if (itemRect.y() < 0)
            verticalScrollBar()->setValue(verticalScrollBar()->value() + itemRect.y());
        else if (itemRect.bottom() > itemArea.height())
            verticalScrollBar()->setValue(verticalScrollBar()->value() + (itemRect.bottom() - itemArea.height()));
        break;

    case PositionAtTop:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + itemRect.y());
        break;

    case PositionAtBottom:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + (itemRect.bottom() - itemArea.height()));
        break;

    case PositionAtCenter:
        verticalScrollBar()->setValue((verticalScrollBar()->value() + itemRect.y()) - ((itemArea.height() - itemRect.height())/2));
        break;
    }

    if (!isEventVisible(event))
        horizontalScrollBar()->setValue(visibleTimeRange.range().start().secsTo(event->localStartDate()));
}

bool rowDataLessThan(const RowData *a, const RowData *b)
{
    return a->y < b->y;
}

QList<RowData *>::const_iterator EventTimelineWidget::findLayoutRow(int y) const
{
    RowData compareTo(RowData::Server);
    compareTo.y = y;

    QList<RowData *>::ConstIterator it = qLowerBound(layoutRows.constBegin(), layoutRows.constEnd(), &compareTo, rowDataLessThan);
    if (it == layoutRows.end() || (*it)->y > y)
    {
        Q_ASSERT(it != layoutRows.begin());
        --it;
    }
    
    return it;
}

EventData *EventTimelineWidget::eventAt(const QPoint &point) const
{
    const_cast<EventTimelineWidget*>(this)->ensureLayout();

    QRect itemArea = viewportItemArea();
    int ry = (point.y() - itemArea.top()) + verticalScrollBar()->value();

    if (!itemArea.contains(point) || ry >= layoutRowsBottom || layoutRows.isEmpty())
        return 0;

    QList<RowData *>::ConstIterator it = findLayoutRow(ry);

    if ((*it)->type != RowData::Location)
        return 0;

    LocationData *location = (*it)->toLocation();

    /* This is slow and can likely be improved. */
    for (QList<EventData*>::ConstIterator evit = location->events.begin(); evit != location->events.end(); ++evit)
    {
        QRect eventRect = timeCellRect((*evit)->localStartDate(), (*evit)->durationInSeconds()).translated(itemArea.left(), 0);
        if (point.x() >= eventRect.left() && point.x() <= eventRect.right())
            return *evit;
    }

    return 0;
}

QModelIndex EventTimelineWidget::indexAt(const QPoint &point) const
{
    EventData *event = eventAt(point);
    if (!event)
        return QModelIndex();

    int row = rowsMap[event];
    return model()->index(row, 0);
}

QSize EventTimelineWidget::sizeHint() const
{
    return QSize(500, 300);
}

QModelIndex EventTimelineWidget::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(cursorAction);
    Q_UNUSED(modifiers);
    return QModelIndex();
}

bool EventTimelineWidget::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return false;
}

int EventTimelineWidget::horizontalOffset() const
{
    return 0;
}

int EventTimelineWidget::verticalOffset() const
{
    return 0;
}

void EventTimelineWidget::setSelection(const QRect &irect, QItemSelectionModel::SelectionFlags command)
{
    QItemSelection sel;

    QRect itemArea = viewportItemArea();
    QRect rect = irect.translated(0, (-itemArea.top()) + verticalScrollBar()->value());

    if (layoutRows.isEmpty())
        return;

    /* The y coordinate of rect is now in scroll-invariant inner y, the same as layoutRows */
    QList<RowData *>::ConstIterator it = findLayoutRow(rect.y());

    for (; it != layoutRows.end() && (*it)->y <= rect.bottom(); ++it)
    {
        if ((*it)->type != RowData::Location)
            continue;

        LocationData *location = (*it)->toLocation();

        for (QList<EventData*>::ConstIterator evit = location->events.begin(); evit != location->events.end(); ++evit)
        {
            QRect eventRect = timeCellRect((*evit)->localStartDate(), (*evit)->durationInSeconds()).translated(itemArea.left(), 0);
            if (eventRect.x() >= rect.x())
            {
                if (eventRect.x() > rect.right())
                    break;

                int row = rowsMap[*evit];
                sel.select(model()->index(row, 0), model()->index(row, model()->columnCount()-1));
            }
        }
    }

    if (!sel.isEmpty())
        selectionModel()->select(sel, command);

    viewport()->update();
}

QRegion EventTimelineWidget::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_UNUSED(selection);
    return QRegion();
}

EventData *EventTimelineWidget::rowData(int row) const
{
    QModelIndex idx = model()->index(row, 0);
    return idx.data(EventsModel::EventDataPtr).value<EventData*>();
}

bool EventTimelineWidget::findEvent(EventData *event, bool create, ServerData **server,
                                    LocationData **location, int *position)
{
    if (server)
        *server = 0;
    if (location)
        *location = 0;
    if (position)
        *position = -1;

    /* Find associated server */
    QHash<DVRServer*,ServerData*>::ConstIterator it = serversMap.find(event->server());
    if (it == serversMap.end())
    {
        if (!create)
            return false;

        ServerData *serverData = new ServerData;
        serverData->server = event->server();
        it = serversMap.insert(serverData->server, serverData);

        scheduleDelayedItemsLayout(DoRowsLayout);
        clearLeftPaddingCache();
    }

    ServerData *serverData = *it;
    if (server)
        *server = serverData;

    /* Find associated location (within the server) */
    QHash<int,LocationData*>::ConstIterator lit = serverData->locationsMap.find(event->locationId());
    if (lit == serverData->locationsMap.end())
    {
        if (!create)
            return false;

        LocationData *locationData = new LocationData;
        locationData->locationId = event->locationId();
        locationData->serverData = serverData;
        lit = serverData->locationsMap.insert(locationData->locationId, locationData);

        scheduleDelayedItemsLayout(DoRowsLayout);
        clearLeftPaddingCache();
    }

    LocationData *locationData = *lit;
    if (location)
        *location = locationData;

    if (position && create)
    {
        /* Find the position where this event belongs */
        int p = 0;
        for (int n = locationData->events.size(); p < n; ++p)
        {
            if (event->localStartDate() < locationData->events[p]->localStartDate())
                break;
        }
        *position = p;
    }
    else if (position)
        *position = locationData->events.indexOf(event);

    return true;
}

QDateTime EventTimelineWidget::earliestDate()
{
    QDateTime result;

    for (QHash<EventData *, int>::Iterator it = rowsMap.begin(); it != rowsMap.end(); ++it)
    {
        QDateTime date = it.key()->localStartDate();
        if (result.isNull() || date < result)
            result = date;
    }

    return result;
}

QDateTime EventTimelineWidget::latestDate()
{
    QDateTime result;

    for (QHash<EventData *, int>::Iterator it = rowsMap.begin(); it != rowsMap.end(); ++it)
    {
        QDateTime date = it.key()->localEndDate();
        if (result.isNull() || date > result)
            result = date;
    }

    return result;
}

void EventTimelineWidget::updateTimeRange(bool fromData)
{
    if (fromData)
        visibleTimeRange.setDateTimeRange(DateTimeRange(earliestDate(), latestDate()));

    /* Determine the minimum width for the primary tick (the tick with a label),
     * which is then used to determine its interval. */
    QFontMetrics fm(font());
    int minTickWidth = qMax(fm.width(tr("22:22"))+6, 16);

    /* Using the minimum tick width, find the minimum number of seconds per tick,
     * and round up to an even and user-friendly duration */
    int areaWidth = viewportItemArea().width();
    
    visibleTimeRange.computePrimaryTickSecs(areaWidth / minTickWidth);

    updateScrollBars();
    viewport()->update();

    scheduleDelayedItemsLayout(DoUpdateTimeRange);
}

void EventTimelineWidget::updateRowsMap(int row)
{
    for (int n = model()->rowCount(); row < n; ++row)
    {
        EventData *data = rowData(row);
        if (!data)
            continue;

        rowsMap.insert(data, row);
    }

#ifndef QT_NO_DEBUG
    Q_ASSERT(rowsMap.size() == model()->rowCount());
    int count = 0;
    foreach (ServerData *sd, serversMap)
    {
        foreach (LocationData *ld, sd->locationsMap)
        {
            count += ld->events.size();
        }
    }
    Q_ASSERT(count == rowsMap.size());
#endif
}

void EventTimelineWidget::updateRowsMapDelayed(int start)
{
    rowsMapUpdateStart = (rowsMapUpdateStart < 0) ? start : qMin(rowsMapUpdateStart, start);
    scheduleDelayedItemsLayout(DoUpdateRowsMap);
}

inline static bool serverSort(const ServerData *s1, const ServerData *s2)
{
    Q_ASSERT(s1 && s2 && s1->server && s2->server);
    return QString::localeAwareCompare(s1->server->configuration().displayName(), s2->server->configuration().displayName()) < 0;
}

inline static bool locationSort(const LocationData *s1, const LocationData *s2)
{
    Q_ASSERT(s1 && s2);
    return QString::localeAwareCompare(s1->uiLocation(), s2->uiLocation()) < 0;
}

void EventTimelineWidget::scheduleDelayedItemsLayout(LayoutFlags flags)
{
    pendingLayouts |= flags;
    QAbstractItemView::scheduleDelayedItemsLayout();
}

void EventTimelineWidget::doItemsLayout()
{
    LayoutFlags layout = pendingLayouts;
    pendingLayouts = 0;

    if (layout & DoRowsLayout)
        doRowsLayout();

    if (layout & DoUpdateRowsMap)
    {
        updateRowsMap(rowsMapUpdateStart);
        rowsMapUpdateStart = -1;
    }

    if (layout & DoUpdateTimeRangeFromData)
        updateTimeRange(true);
    else if (layout & DoUpdateTimeRange)
        updateTimeRange(false);

    QAbstractItemView::doItemsLayout();
}

void EventTimelineWidget::doRowsLayout()
{
    layoutRows.clear();

    /* Sort servers */
    QList<ServerData*> sortedServers = serversMap.values();
    qSort(sortedServers.begin(), sortedServers.end(), serverSort);

    int y = 0;

    foreach (ServerData *sd, sortedServers)
    {
        sd->y = y;
        layoutRows.insert(y, sd);
        y += rowHeight();

        QList<LocationData*> sortedLocations = sd->locationsMap.values();
        qSort(sortedLocations.begin(), sortedLocations.end(), locationSort);

        foreach (LocationData *ld, sortedLocations)
        {
            ld->y = y;
            layoutRows.insert(y, ld);
            y += rowHeight();
        }
    }

    layoutRowsBottom = y;

    int h = viewportItemArea().height();
    verticalScrollBar()->setRange(0, qMax(0, layoutRowsBottom-h));
    verticalScrollBar()->setPageStep(h);
}

void EventTimelineWidget::addModelRows(int first, int last)
{
    if (last < 0)
        last = model()->rowCount() - 1;

    DateTimeRange dateTimeRange = visibleTimeRange.range();
    for (int i = first; i <= last; ++i)
    {
        EventData *data = rowData(i);
        if (!data)
            continue;

        LocationData *locationData;
        int pos;
        findEvent(data, true, 0, &locationData, &pos);

        locationData->events.insert(pos, data);
        rowsMap.insert(data, i);

        dateTimeRange = dateTimeRange.extendWith(data->localStartDate());
        dateTimeRange = dateTimeRange.extendWith(data->localEndDate());
    }

    visibleTimeRange.setDateTimeRange(dateTimeRange);
    updateRowsMapDelayed(last+1);
    scheduleDelayedItemsLayout(DoUpdateTimeRange);
}

void EventTimelineWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    addModelRows(start, end);
    QAbstractItemView::rowsInserted(parent, start, end);
}

void EventTimelineWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());

    for (int i = start; i <= end; ++i)
    {
        EventData *data = rowData(i);
        if (!data)
            continue;

        ServerData *serverData;
        LocationData *locationData;
        if (!findEvent(data, false, &serverData, &locationData, 0))
            continue;

        bool ok = locationData->events.removeOne(data);
        Q_ASSERT(ok);
        Q_UNUSED(ok);
        rowsMap.remove(data);

        if (locationData->events.isEmpty())
        {
            serverData->locationsMap.remove(locationData->locationId);
            layoutRows.removeAll(locationData);
            delete locationData;

            if (serverData->locationsMap.isEmpty())
            {
                serversMap.remove(serverData->server);
                layoutRows.removeAll(serverData);
                delete serverData;
            }

            scheduleDelayedItemsLayout(DoRowsLayout);
        }
    }

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void EventTimelineWidget::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(end);
    Q_ASSERT(!parent.isValid());

    updateRowsMapDelayed(start);
    scheduleDelayedItemsLayout(DoUpdateTimeRangeFromData);
}

void EventTimelineWidget::reset()
{
    clearData();
    addModelRows(0);
}

void EventTimelineWidget::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int firstRow = topLeft.row(), lastRow = bottomRight.row();

    for (int row = firstRow; row <= lastRow; ++row)
    {
        EventData *data = rowData(row);
        Q_ASSERT(rowsMap[data] == row);

        /* Try to find this event to handle (relatively quickly) the common case when
         * location/server do not change. */
        ServerData *server = 0;
        if (findEvent(data, false, &server, 0, 0) || !server)
            continue;

        /* Brute-force search of all locations in this server to find the old one and move it.
         * Server cannot change. */
        foreach (LocationData *location, server->locationsMap)
        {
            int pos = location->events.indexOf(data);
            if (pos < 0)
                continue;

            location->events.removeAt(pos);
            if (location->events.isEmpty())
            {
                server->locationsMap.remove(location->locationId);
                delete location;
                scheduleDelayedItemsLayout(DoRowsLayout);
            }

            break;
        }

        LocationData *location;
        int pos;
        findEvent(data, true, &server, &location, &pos);
        location->events.insert(pos, data);
    }

    scheduleDelayedItemsLayout(DoUpdateTimeRangeFromData);
    QAbstractItemView::dataChanged(topLeft, bottomRight);
    viewport()->update();
}

QRect EventTimelineWidget::viewportItemArea() const
{
    return viewport()->rect().adjusted(leftPadding(), topPadding(), 0, 0);
}

int EventTimelineWidget::timeXOffset(const QDateTime &time) const
{
    double range = qMax(visibleTimeRange.visibleSeconds(), 1);
    int width = viewportItemArea().width();
    return qMax(0, qRound((visibleTimeRange.visibleRange().start().secsTo(time) / range) * width));
}

double EventTimelineWidget::pixelsPerSeconds(int seconds) const
{
    int range = qMax(visibleTimeRange.visibleSeconds(), 1);
    int width = viewportItemArea().width();
    double pixelsPerSecond = (double) width / range;
    return pixelsPerSecond * seconds;
}

QRect EventTimelineWidget::timeCellRect(const QDateTime &start, int duration, int top, int height) const
{
    Q_ASSERT(visibleTimeRange.range().contains(start));

    QRect r;
    r.setTop(top);
    r.setHeight(height);
    r.setLeft(timeXOffset(start));
    r.setRight(timeXOffset(start.addSecs(duration)));
    return r;
}

void EventTimelineWidget::resizeEvent(QResizeEvent *event)
{
    updateTimeRange(false);
    QAbstractItemView::resizeEvent(event);
}

bool EventTimelineWidget::viewportEvent(QEvent *event)
{
    bool re = QAbstractItemView::viewportEvent(event);

    if (event->type() == QEvent::Polish || event->type() == QEvent::FontChange)
    {
        /* Top padding for the X-axis label text */
        QFont f = font();
        QFontMetrics fm(font());

        m_rowHeight = fm.height() + 6;
        int height = fm.height();

        f.setBold(true);
        fm = QFontMetrics(f);
        height += fm.height();

        cachedTopPadding = height;

        clearLeftPaddingCache();
        scheduleDelayedItemsLayout(DoUpdateTimeRange);
    }

    return re;
}

void EventTimelineWidget::paintDays(QPainter &p)
{
    EventTimelineDatePainter datePainter(p);
    datePainter.setStartDate(visibleTimeRange.visibleRange().start().date().addDays(-1));
    datePainter.setEndDate(visibleTimeRange.visibleRange().end().date());
//    datePainter.setVisibleTimeStart(visibleTimeRange.visibleRange().start().addSecs(utcOffset()));
    datePainter.setVisibleTimeStart(visibleTimeRange.visibleRange().start());
    datePainter.setPixelsPerSecondRatio(pixelsPerSeconds(1));

    QFont font = p.font();
    font.setBold(true);
    p.setFont(font);
    p.setBrush(Qt::NoBrush);

    datePainter.paintDates();
}

int EventTimelineWidget::utcOffset() const
{
    if (rowsMap.isEmpty())
        return 0;

    return rowsMap.begin().key()->serverDateTzOffsetMins() * 60;
}

void EventTimelineWidget::paintEvent(QPaintEvent *event)
{
    ensureLayout();

    QPainter p(viewport());
    p.eraseRect(event->rect());

    QAbstractItemModel *model = this->model();
    if (!model || rowsMap.isEmpty())
        return;

    // we dont have to draw anything now
    if (viewportItemArea().width() <= 0)
        return;

    QRect r = viewport()->rect();

    p.save();
    p.translate(leftPadding(), 0);
    paintDays(p);
    p.restore();

    p.save();
    p.translate(QPoint(leftPadding(), p.fontMetrics().height()));
    paintTickLines(p, r);
    p.restore();

    p.save();
    p.translate(QPoint(0, topPadding()));
    paintLegend(p);
    p.restore();

    p.save();
    p.translate(QPoint(leftPadding(), topPadding()));
    p.drawLine(0, 0, r.width(), 0);
    p.drawLine(0, 0, 0, r.height());
    p.setClipRect(0, 1, r.width(), r.height());
    paintChart(p, r.width());
    p.restore();
}

QDateTime EventTimelineWidget::firstTickDateTime() const
{
    QDateTime firstTickDateTime = visibleTimeRange.visibleRange().start();
//    QDateTime firstTickDateTime = visibleTimeRange.visibleRange().start().addSecs(utcOffset());
    int preAreaSecs = visibleTimeRange.visibleRange().start().toTime_t() % visibleTimeRange.primaryTickSecs();
    if (preAreaSecs)
        return firstTickDateTime.addSecs(visibleTimeRange.primaryTickSecs() - preAreaSecs);
    else
        return firstTickDateTime;
}

int EventTimelineWidget::secondsFromVisibleStart(const QDateTime& serverTime) const
{
    return visibleTimeRange.visibleRange().start().secsTo(serverTime);
//    return visibleTimeRange.visibleRange().start().addSecs(utcOffset()).secsTo(serverTime);
}

void EventTimelineWidget::paintTickLines(QPainter &p, const QRect &rect)
{
    Q_ASSERT(visibleTimeRange.primaryTickSecs());

    int tickLineCount = qCeil(double(visibleTimeRange.visibleSeconds()) / visibleTimeRange.primaryTickSecs());
    QVector<QLine> tickLines;
    tickLines.reserve(tickLineCount);

    QDateTime tickDateTime = firstTickDateTime();
    int tickDateTimeOffset = qRound(pixelsPerSeconds(secondsFromVisibleStart(tickDateTime)));

    double tickPosition = tickDateTimeOffset;
    double tickWidth = pixelsPerSeconds(visibleTimeRange.primaryTickSecs());

    int lineTop = p.fontMetrics().height() + 1;
    while (true)
    {
        tickLines.append(QLine(qRound(tickPosition), lineTop, qRound(tickPosition), rect.bottom()));

        QString text = tickDateTime.toString(tr("h:mm"));
        QRectF textRect(tickPosition - tickWidth / 2.0, 0, tickWidth, rect.height());
        p.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, text);

        if (textRect.right() >= rect.right())
            break;

        tickPosition += tickWidth;
        tickDateTime = tickDateTime.addSecs(visibleTimeRange.primaryTickSecs());
    }

    p.save();
    p.setPen(QColor(205, 205, 205));
    p.drawLines(tickLines);
    p.restore();
}

void EventTimelineWidget::paintLegend(QPainter& p)
{
    QRect textRect(16, 0, leftPadding(), rowHeight());
    QFont serverFont = p.font();
    serverFont.setBold(true);

    QList<RowData *>::ConstIterator it = findLayoutRow(verticalScrollBar()->value());
    for (; it != layoutRows.end(); ++it)
    {
        int ry = (*it)->y - verticalScrollBar()->value();
        textRect.moveTop(ry);

        if ((*it)->type == RowData::Server)
        {
            p.save();
            p.setFont(serverFont);
            p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, (*it)->toServer()->server->configuration().displayName());
            p.restore();
        }
        else
            p.drawText(textRect.adjusted(16, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
                       (*it)->toLocation()->uiLocation());
    }
}

void EventTimelineWidget::paintChart(QPainter& p, int width)
{
    QList<RowData *>::ConstIterator it = findLayoutRow(verticalScrollBar()->value());
    for (; it != layoutRows.end(); ++it)
    {
        int ry = (*it)->y - verticalScrollBar()->value();

        if ((*it)->type != RowData::Server)
        {
            QRect rowRect(0, ry, width, rowHeight());
            paintRow(&p, rowRect, (*it)->toLocation());
        }
    }
}

bool EventTimelineWidget::isEventVisible(EventData *data) const
{
    if (data->localEndDate() < visibleTimeRange.visibleRange().start())
        return false;
    if (data->localStartDate() > visibleTimeRange.visibleRange().end())
        return false;

    return true;
}

void EventTimelineWidget::paintRow(QPainter *p, QRect r, LocationData *locationData)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(Qt::NoPen);
    p->setClipRect(r, Qt::IntersectClip);
    p->translate(r.topLeft());

    // TODO: what about finding first and last visible event by binary search?
    foreach (EventData *event, locationData->events)
        if (isEventVisible(event))
            paintEvent(*p, r.height(), event);

    p->restore();
}

void EventTimelineWidget::paintEvent(QPainter &p, int boxHeight, EventData *event)
{
    Q_ASSERT(event);
    Q_ASSERT(rowsMap.contains(event));

    int modelRow = rowsMap[event];

    QRect cellRect = timeCellRect(event->localStartDate(), event->durationInSeconds(), 0, boxHeight);

    p.setBrush(event->uiColor());
    p.drawRoundedRect(cellRect.adjusted(0, 1, 0, -1), 2, 2);

    if (selectionModel()->rowIntersectsSelection(modelRow, QModelIndex()))
    {
        p.setPen(Qt::red);
        p.drawRect(cellRect.adjusted(0, 0, -1, -1));
        p.setPen(Qt::NoPen);
    }
}

void EventTimelineWidget::clearLeftPaddingCache()
{
    cachedLeftPadding = -1;
}

int EventTimelineWidget::leftPadding() const
{
    if (cachedLeftPadding >= 0)
        return cachedLeftPadding;

    QFont locationFont(font());
    QFont serverFont(locationFont);
    serverFont.setBold(true);

    QFontMetrics locfm(locationFont);
    QFontMetrics serverfm(serverFont);

    for (QHash<DVRServer*,ServerData*>::ConstIterator it = serversMap.begin(); it != serversMap.end(); ++it)
    {
        cachedLeftPadding = qMax(cachedLeftPadding, serverfm.width((*it)->server->configuration().displayName()) + 16);

        for (QHash<int,LocationData*>::ConstIterator lit = (*it)->locationsMap.begin();
             lit != (*it)->locationsMap.end(); ++lit)
        {
            cachedLeftPadding = qMax(cachedLeftPadding, locfm.width((*lit)->uiLocation()) + 16);
        }
    }

    cachedLeftPadding += 16;
    return cachedLeftPadding;
}

void EventTimelineWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        QAbstractItemView::mousePressEvent(event);
        return;
    }

    EventData *data = eventAt(event->pos());

    if (data)
    {
        QAbstractItemView::mousePressEvent(event);

        Q_ASSERT(rowsMap.contains(data));
        QModelIndex index = model()->index(rowsMap[data], 0);
        Q_ASSERT(index.isValid());

        selectionModel()->select(index, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
        viewport()->update();
        event->accept();
    }
    else if (viewportItemArea().contains(event->pos()))
    {
        if (!mouseRubberBand)
            mouseRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        mouseClickPos = event->pos();
        mouseRubberBand->setGeometry(QRect(mouseClickPos, QSize()));
        mouseRubberBand->show();
    }
}

void EventTimelineWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseRubberBand)
    {
        Q_ASSERT(!mouseClickPos.isNull());
        mouseRubberBand->setGeometry(QRect(mouseClickPos, event->pos()).normalized().intersect(viewportItemArea()));
    }
}

void EventTimelineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouseRubberBand)
    {
        if (event->modifiers() & Qt::AltModifier)
            setSelection(mouseRubberBand->geometry(), QItemSelectionModel::Deselect);
        else
            setSelection(mouseRubberBand->geometry(), QItemSelectionModel::Select);
        mouseRubberBand->hide();
        mouseRubberBand->deleteLater();
        mouseRubberBand = 0;
    }

    QAbstractItemView::mouseReleaseEvent(event);
}
