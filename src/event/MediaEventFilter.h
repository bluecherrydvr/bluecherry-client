#ifndef MEDIAEVENTFILTER_H
#define MEDIAEVENTFILTER_H

#include "EventFilter.h"

class MediaEventFilter : public EventFilter
{
public:
    MediaEventFilter();
    virtual ~MediaEventFilter();

    virtual bool accept(const EventData &event) const;
};

#endif // MEDIAEVENTFILTER_H
