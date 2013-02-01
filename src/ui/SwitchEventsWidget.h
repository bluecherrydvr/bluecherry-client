#ifndef SWITCHEVENTSWIDGET_H
#define SWITCHEVENTSWIDGET_H

#include <QWidget>

class QPushButton;
class EventsCursor;

class SwitchEventsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SwitchEventsWidget(QWidget *parent = 0);
    virtual ~SwitchEventsWidget();

    void setEventsCursor(const QSharedPointer<EventsCursor> &eventsCursor);
    EventsCursor * eventsCursor() const { return m_eventsCursor.data(); }

private slots:
    void updateButtonsState();

private:
    QSharedPointer<EventsCursor> m_eventsCursor;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
};

#endif // SWITCHEVENTSWIDGET_H
