#include "CameraEventFilter.h"
#include "core/EventData.h"

CameraEventFilter::CameraEventFilter()
{
}

CameraEventFilter::~CameraEventFilter()
{
}

bool CameraEventFilter::accept(const EventData &event) const
{
    return event.isCamera();
}
