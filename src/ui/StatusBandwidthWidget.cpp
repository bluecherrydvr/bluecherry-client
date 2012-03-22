#include "StatusBandwidthWidget.h"
#include "core/BluecherryApp.h"
#include "core/LiveViewManager.h"
#include "utils/StringUtils.h"
#include <QMenu>
#include <QStyle>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QToolButton>

StatusBandwidthWidget::StatusBandwidthWidget(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setIcon(QIcon(QLatin1String(":/icons/system-monitor.png")));
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    QMenu *menu = new QMenu(this);
    QList<QAction*> fpsActions = bcApp->liveView->bandwidthActions(bcApp->liveView->bandwidthMode(),
                                                                   bcApp->liveView,
                                                                   SLOT(setBandwidthModeFromAction()));

    foreach (QAction *a, fpsActions)
        a->setParent(menu);

    menu->addActions(fpsActions);
    setMenu(menu);

    connect(this, SIGNAL(pressed()), SLOT(showMenu()));

    connect(bcApp->globalRate, SIGNAL(rateUpdated(unsigned)), SLOT(rateUpdated(unsigned)));
    connect(bcApp->liveView, SIGNAL(bandwidthModeChanged(int)), SLOT(bandwidthModeChanged(int)));
    rateUpdated(bcApp->globalRate->currentRate());
}

void StatusBandwidthWidget::rateUpdated(unsigned currentRate)
{
    setText(byteSizeString(currentRate, BytesPerSecond));
}

void StatusBandwidthWidget::bandwidthModeChanged(int value)
{
    foreach (QAction *a, menu()->actions())
    {
        a->setChecked(value == a->data().toInt());
    }
}
