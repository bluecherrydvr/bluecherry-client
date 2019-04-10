/*
 * Copyright 2010-2019 Bluecherry, LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
