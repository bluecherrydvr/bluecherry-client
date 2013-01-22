#include "Range.h"
#include <QtGlobal>

Range Range::invalid()
{
    Range result;
    result.m_start = 1;
    result.m_end = 0;

    return result;
}

Range Range::fromValue(unsigned value)
{
    Range result;
    result.m_start = value;
    result.m_end = value;

    return result;
}

Range Range::fromStartEnd(unsigned start, unsigned end)
{
    Range result;
    result.m_start = start;
    result.m_end = end;

    return result;
}

Range Range::fromStartSize(unsigned start, unsigned size)
{
    Range result;
    result.m_start = start;
    result.m_end = start + size - 1;

    return result;
}

Range::Range()
    : m_start(1), m_end(0)
{
}

unsigned Range::size() const
{
    if (isValid())
        return m_end - m_start + 1;
    else
        return 0;
}

bool Range::isValid() const
{
    return m_end >= m_start;
}

bool Range::includes(unsigned value) const
{
    return (value >= m_start) && (value <= m_end);
}

bool Range::includes(const Range &otherRange) const
{
    return (otherRange.start() >= m_start) && (otherRange.end() <= m_end);
}
