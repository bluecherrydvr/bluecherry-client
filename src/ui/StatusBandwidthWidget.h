#ifndef STATUSBANDWIDTHWIDGET_H
#define STATUSBANDWIDTHWIDGET_H

#include <QtGlobal>

#ifndef Q_WS_MAC
#include <QToolButton>

class StatusBandwidthWidget : public QToolButton
{
    Q_OBJECT

public:
    explicit StatusBandwidthWidget(QWidget *parent = 0);

private slots:
    void bandwidthModeChanged(int value);
    void rateUpdated(unsigned currentRate);
};

#else /* Q_WS_MAC */

#include <QMacCocoaViewContainer>

class QMenu;
class QAction;
class NSPopUpButton;

class StatusBandwidthWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    explicit StatusBandwidthWidget(QWidget *parent);

private slots:
    void bandwidthModeChanged(int value);
    void rateUpdated(unsigned currentRate);

private:
    NSPopUpButton *m_button;
    QMenu *m_menu;
    QAction *m_titleAction;
};

#endif

#endif // STATUSBANDWIDTHWIDGET_H
