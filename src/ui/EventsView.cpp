#include "EventsView.h"
#include "EventsModel.h"
#include "EventViewWindow.h"
#include <QHeaderView>
#include <QMovie>
#include <QLabel>
#include <QEvent>

EventsView::EventsView(QWidget *parent)
    : QTreeView(parent), loadingIndicator(0)
{
    setRootIsDecorated(false);
    setUniformRowHeights(true);

    viewport()->installEventFilter(this);
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

    connect(model, SIGNAL(loadingStarted()), SLOT(loadingStarted()));
    connect(model, SIGNAL(loadingFinished()), SLOT(loadingFinished()));
    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), SLOT(loadingFinished()));
    connect(model, SIGNAL(modelReset()), SLOT(loadingFinished()));

    if (model->isLoading())
        loadingStarted();
}

EventsModel *EventsView::eventsModel() const
{
    Q_ASSERT(!model() || qobject_cast<EventsModel*>(model()));
    return static_cast<EventsModel*>(model());
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
    if (!eventsModel()->isLoading() || eventsModel()->rowCount())
    {
        delete loadingIndicator;
        loadingIndicator = 0;
    }
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
