#include "EventsView.h"
#include "EventsModel.h"
#include <QHeaderView>

EventsView::EventsView(QWidget *parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setUniformRowHeights(true);
}

void EventsView::setModel(EventsModel *model)
{
    QTreeView::setModel(model);
    header()->setResizeMode(QHeaderView::Interactive);
}

EventsModel *EventsView::eventsModel() const
{
    Q_ASSERT(!model() || qobject_cast<EventsModel*>(model()));
    return static_cast<EventsModel*>(model());
}
