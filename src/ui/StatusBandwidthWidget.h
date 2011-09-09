#ifndef STATUSBANDWIDTHWIDGET_H
#define STATUSBANDWIDTHWIDGET_H

#include <QToolButton>

class StatusBandwidthWidget : public QToolButton
{
    Q_OBJECT

public:
    explicit StatusBandwidthWidget(QWidget *parent = 0);

private slots:
    void rateUpdated(unsigned currentRate);
};

#endif // STATUSBANDWIDTHWIDGET_H
