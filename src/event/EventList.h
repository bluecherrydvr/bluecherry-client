#ifndef EVENTLIST_H
#define EVENTLIST_H

#include "core/EventData.h"
#include <QList>

class EventFilter;

class EventList : public QList<EventData>
{
public:
    EventList filter(const EventFilter &eventFilter) const;
    QSet<DVRCamera> cameras() const;
};

#endif // EVENTLIST_H
