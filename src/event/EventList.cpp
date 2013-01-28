#include "EventList.h"
#include "core/DVRCamera.h"
#include "event/EventFilter.h"
#include <QSet>

EventList EventList::filter(const EventFilter &eventFilter) const
{
    EventList result;
    foreach (const EventData &event, *this)
        if (eventFilter.accept(event))
            result.append(event);
    return result;
}

QSet<DVRCamera> EventList::cameras() const
{
    QSet<DVRCamera> result;
    foreach (const EventData &cameraEventData, *this)
    {
        DVRCamera camera = cameraEventData.locationCamera();
        if (camera)
            result.insert(camera);
    }
    return result;
}
