#ifndef EVENTFILTER_H
#define EVENTFILTER_H

class EventData;

class EventFilter
{
public:
    EventFilter();
    virtual ~EventFilter();

    virtual bool accept(const EventData &event) const = 0;
};

#endif // EVENTFILTER_H
