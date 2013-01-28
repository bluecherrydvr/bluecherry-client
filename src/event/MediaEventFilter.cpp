#include "MediaEventFilter.h"
#include "core/EventData.h"

MediaEventFilter::MediaEventFilter()
{
}

MediaEventFilter::~MediaEventFilter()
{
}

bool MediaEventFilter::accept(const EventData &event) const
{
    return event.hasMedia();
}
