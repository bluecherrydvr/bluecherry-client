#ifndef EVENTSVIEW_H
#define EVENTSVIEW_H

#include <QTreeView>

class EventsModel;

class EventsView : public QTreeView
{
    Q_OBJECT

public:
    explicit EventsView(QWidget *parent = 0);

    EventsModel *eventsModel() const;

    void setModel(EventsModel *model);

private slots:
    void openEvent(const QModelIndex &index);

private:
    using QTreeView::setModel;
};

#endif // EVENTSVIEW_H
