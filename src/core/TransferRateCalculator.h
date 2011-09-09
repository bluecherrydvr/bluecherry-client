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

    Q_INVOKABLE void startTimer();
};

#endif // TRANSFERRATECALCULATOR_H
