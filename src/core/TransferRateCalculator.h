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

#ifndef TRANSFERRATECALCULATOR_H
#define TRANSFERRATECALCULATOR_H

#include <QObject>
#include <QBasicTimer>
#include <QAtomicInt>

class TransferRateCalculator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(unsigned currentRate READ currentRate NOTIFY rateUpdated)

public:
    TransferRateCalculator(QObject *parent = 0);

    /* Current transfer rate in bytes per second */
    unsigned currentRate();

public slots:
    /* Record that x bytes have been added since the last sample. This
     * function is threadsafe, if you can guarantee that the instance will
     * not be destroyed. */
    void addSampleValue(unsigned bytes);

signals:
    void rateUpdated(unsigned currentRate);

protected:
    virtual void timerEvent(QTimerEvent *ev);

private:
    static const int interval = 750;
    static const int sampleCount = (3*(1000/interval));

    QBasicTimer m_timer;
    QAtomicInt m_nextSample;
    unsigned m_samples[sampleCount];
    qint8 m_nSample;

    void startTimer();

};

#endif // TRANSFERRATECALCULATOR_H
