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
    bool first = !this->model();
    QTreeView::setModel(model);

    if (first)
    {
        header()->setResizeMode(QHeaderView::Interactive);
        QFontMetrics fm(font());
        header()->resizeSection(EventsModel::LocationColumn, fm.width(QLatin1Char('X')) * 20);
        header()->resizeSection(EventsModel::DurationColumn,
                                fm.width(QLatin1String("99 minutes, 99 seconds")) + 25);
        header()->resizeSection(EventsModel::LevelColumn, fm.width(QLatin1String("Warning")) + 18);
    }
}

EventsModel *EventsView::eventsModel() const
{
    Q_ASSERT(!model() || qobject_cast<EventsModel*>(model()));
    return static_cast<EventsModel*>(model());
}
