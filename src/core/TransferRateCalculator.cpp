#include "TransferRateCalculator.h"
#include <cstring>

TransferRateCalculator::TransferRateCalculator(QObject *parent)
    : QObject(parent), m_nextSample(0), m_nSample(0)
{
    memset(m_samples, 0, sizeof(m_samples));
}

void TransferRateCalculator::addSampleValue(unsigned bytes)
{
    Q_ASSERT(bytes < 0x80000000);
    m_nextSample.fetchAndAddAcquire((int)bytes);
    if (!m_timer.isActive())
        metaObject()->invokeMethod(this, "startTimer", Qt::AutoConnection);
}

void TransferRateCalculator::startTimer()
{
    m_timer.start(interval, this);
}

unsigned TransferRateCalculator::currentRate()
{
    unsigned r = 0;
    for (int i = 0; i < sampleCount; ++i)
        r += m_samples[i];
    return r/sampleCount;
}

void TransferRateCalculator::timerEvent(QTimerEvent *)
{
    m_samples[m_nSample] = m_nextSample;
    m_nextSample = 0;
    if (++m_nSample == sampleCount)
        m_nSample = 0;

    unsigned r = currentRate();
    if (!r)
        m_timer.stop();
    emit rateUpdated(r);
}
