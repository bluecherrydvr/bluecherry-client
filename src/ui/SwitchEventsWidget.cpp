#include "SwitchEventsWidget.h"
#include "event/EventsCursor.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>

SwitchEventsWidget::SwitchEventsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    // layout->setMargin(0);

    m_previousButton = new QPushButton(tr("Previous"));
    m_nextButton = new QPushButton(tr("Next"));

    layout->addWidget(m_previousButton);
    layout->addWidget(m_nextButton);

    updateButtonsState();
}

SwitchEventsWidget::~SwitchEventsWidget()
{
}

void SwitchEventsWidget::setEventsCursor(const QSharedPointer<EventsCursor> &eventsCursor)
{
    EventsCursor *oldCursor = m_eventsCursor.data();
    if (m_eventsCursor.data())
    {
        disconnect(m_previousButton, 0, oldCursor, 0);
        disconnect(m_nextButton, 0, oldCursor, 0);
        disconnect(oldCursor, 0, this, 0);
    }

    m_eventsCursor = eventsCursor;

    if (m_eventsCursor)
    {
        connect(m_previousButton, SIGNAL(clicked()), m_eventsCursor.data(), SLOT(moveToPrevious()));
        connect(m_nextButton, SIGNAL(clicked()), m_eventsCursor.data(), SLOT(moveToNext()));
        connect(eventsCursor.data(), SIGNAL(eventSwitched(EventData*)), this, SLOT(updateButtonsState()));
    }

    updateButtonsState();
}

void SwitchEventsWidget::updateButtonsState()
{
    if (!m_eventsCursor)
    {
        m_previousButton->setEnabled(false);
        m_nextButton->setEnabled(false);
    }
    else
    {
        m_previousButton->setEnabled(m_eventsCursor.data()->hasPrevious());
        m_nextButton->setEnabled(m_eventsCursor.data()->hasNext());
    }
}
