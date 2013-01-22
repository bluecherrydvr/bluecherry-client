#ifndef RANGE_H
#define RANGE_H

class Range
{
public:
    static Range invalid();
    static Range fromValue(unsigned value);
    static Range fromStartEnd(unsigned start, unsigned end);
    static Range fromStartSize(unsigned start, unsigned size);

    Range();

    unsigned start() const { return m_start; }
    unsigned end() const { return m_end; }
    unsigned size() const;

    bool isValid() const;
    bool includes(unsigned value) const;
    bool includes(const Range &otherRange) const;

private:

    unsigned m_start; // inclusive
    unsigned m_end; // inclusive
};

#endif // RANGE_H
