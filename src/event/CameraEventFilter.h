#ifndef CAMERAEVENTFILTER_H
#define CAMERAEVENTFILTER_H

#include "EventFilter.h"

class CameraEventFilter : public EventFilter
{
public:
    CameraEventFilter();
    virtual ~CameraEventFilter();

    virtual bool accept(const EventData &event) const;
};

#endif // CAMERAEVENTFILTER_H
