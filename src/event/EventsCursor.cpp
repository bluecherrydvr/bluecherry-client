#include "EventsCursor.h"

EventsCursor::EventsCursor(QObject *parent)
    : QObject(parent)
{
}

EventsCursor::~EventsCursor()
{
}

void EventsCursor::emitAvailabilitySignals()
{
    emit previousAvailabilityChanged(hasPrevious());
    emit nextAvailabilityChanged(hasNext());
}
