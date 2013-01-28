#include "EventList.h"
#include "event/EventFilter.h"

EventList EventList::filter(const EventFilter &eventFilter) const
{
    EventList result;
    foreach (const EventData &event, *this)
        if (eventFilter.accept(event))
            result.append(event);
    return result;
}
