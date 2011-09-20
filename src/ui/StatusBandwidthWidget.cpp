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
    QList<QAction*> fpsActions = bcApp->liveView->fpsActions(bcApp->liveView->globalInterval(),
                                                             bcApp->liveView,
                                                             SLOT(setGlobalIntervalFromAction()));

    foreach (QAction *a, fpsActions)
        a->setParent(menu);

    menu->addActions(fpsActions);
    setMenu(menu);

    connect(this, SIGNAL(pressed()), SLOT(showMenu()));

    connect(bcApp->globalRate, SIGNAL(rateUpdated(unsigned)), SLOT(rateUpdated(unsigned)));
    connect(bcApp->liveView, SIGNAL(globalIntervalChanged(int)), SLOT(globalIntervalChanged(int)));
    rateUpdated(bcApp->globalRate->currentRate());
}

void StatusBandwidthWidget::rateUpdated(unsigned currentRate)
{
    setText(byteSizeString(currentRate, BytesPerSecond));
}

void StatusBandwidthWidget::globalIntervalChanged(int interval)
{
    foreach (QAction *a, menu()->actions())
    {
        if (!a->data().isNull() && a->data().toInt() == interval)
            a->setChecked(true);
        else
            a->setChecked(false);
    }
}
