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

#ifndef EVENTTIMELINEWIDGET_H
#define EVENTTIMELINEWIDGET_H

#include "VisibleTimeRange.h"
#include <QAbstractItemView>
#include <QDateTime>

class DVRServer;
class QRubberBand;

struct RowData;
struct ServerData;
struct LocationData;
class EventData;

class EventTimelineWidget : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit EventTimelineWidget(QWidget *parent = 0);
    ~EventTimelineWidget();

    /* Zoom in the number of seconds of time visible on screen */
    int zoomSeconds() const { return visibleTimeRange.zoomSeconds(); }
    int minZoomSeconds() const { return visibleTimeRange.minZoomSeconds(); }
    int maxZoomSeconds() const { return visibleTimeRange.maxZoomSeconds(); }

    /* Zoom level in the range of 0-100, 0 showing everything with no scroll */
    double zoomLevel() const { return visibleTimeRange.zoomLevel(); }

    virtual QSize sizeHint() const;
    virtual QRect visualRect(const QModelIndex &index) const;
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint);
    virtual QModelIndex indexAt(const QPoint &point) const;
    virtual void setModel(QAbstractItemModel *model);

    virtual void reset();

    /* Internal, do not touch. */
    virtual void doItemsLayout();

public slots:
    void setZoomLevel(double level);
    void setZoomSeconds(int seconds);

signals:
    void zoomSecondsChanged(int zoomSeconds);
    void zoomRangeChanged(int min, int max);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool viewportEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    virtual bool isIndexHidden(const QModelIndex &index) const;

    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;

    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    /* layoutChanged, rowsMoved */

private slots:
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void setViewStartOffset(int secs);

private:
    QHash<DVRServer*,ServerData*> serversMap;
    QHash<EventData*,int> rowsMap;

    VisibleTimeRange visibleTimeRange;

    int cachedTopPadding;
    mutable int cachedLeftPadding;

    /* Cached layout information */
    int rowsMapUpdateStart;
    enum LayoutFlag
    {
        DoRowsLayout = 1,
        DoUpdateRowsMap,
        DoUpdateTimeRange, /* Update cached time range information, assuming that dataTimeStart is accurate */
        DoUpdateTimeRangeFromData, /* Update cached time range information from underlying data */
    };
    Q_DECLARE_FLAGS(LayoutFlags, LayoutFlag)
    LayoutFlags pendingLayouts;
    void scheduleDelayedItemsLayout(LayoutFlags flags);
    void ensureLayout();

    /* Update the rowsMap starting with the item at row 'start' and continuing to the end */
    void updateRowsMap(int start = 0);
    void updateRowsMapDelayed(int start = 0);
    /* Call when the time range in the underlying data may have changed. If fromData is true, dataTimeStart
     * and dataTimeEnd will be updated. Must be called regardless, to update various other cached data. */
    void updateTimeRange(bool fromData = true);

    /* Scroll-invariant inner y-position to row data, suitable for vertical painting and hit tests.
     * First row will be at position 0, which is drawn just below the top padding when scrolled up. */
    QMap<int, RowData*> layoutRows;
    int layoutRowsBottom;

    void doRowsLayout();
    int layoutHeightForRow(const QMap<int,RowData*>::ConstIterator &iterator) const;

    /* Mouse events */
    QPoint mouseClickPos;
    QRubberBand *mouseRubberBand;

    int leftPadding() const;
    int topPadding() const { return cachedTopPadding; }
    int rowHeight() const { return 20; }
    int cellMinimum() const { return 8; }

    void clearLeftPadding();

    EventData *rowData(int row) const;
    bool findEvent(EventData *event, bool create, ServerData **server, LocationData **location, int *position);

    void addModelRows(int first, int last = -1);
    void clearData();
    /* Ensure that the view time is within the boundaries of the data, changing it (scrolling or zooming) if necessary */
    void ensureViewTimeSpan();
    /* Update the scroll bar position, which is necessary when viewSeconds has changed */
    void updateScrollBars();

    EventData *eventAt(const QPoint &point) const;

    /* Area of the viewport containing items */
    QRect viewportItemArea() const;
    /* X-coordinate offset from the edge of the viewport item area for the given time */
    int timeXOffset(const QDateTime &time) const;
    QRect timeCellRect(const QDateTime &start, int duration) const;

    void paintRow(QPainter *p, QRect rect, LocationData *locationData);
};

inline void EventTimelineWidget::ensureLayout()
{
    if (pendingLayouts != 0)
        executeDelayedItemsLayout();
}

#endif // EVENTTIMELINEWIDGET_H
