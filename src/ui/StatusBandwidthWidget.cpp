#include "StatusBandwidthWidget.h"
#include "core/BluecherryApp.h"
#include "utils/StringUtils.h"
#include <QMenu>

StatusBandwidthWidget::StatusBandwidthWidget(QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setIcon(QIcon(QLatin1String(":/icons/system-monitor.png")));
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    connect(bcApp->globalRate, SIGNAL(rateUpdated(unsigned)), SLOT(rateUpdated(unsigned)));
    rateUpdated(bcApp->globalRate->currentRate());
}

void StatusBandwidthWidget::rateUpdated(unsigned currentRate)
{
    setText(byteSizeString(currentRate, BytesPerSecond));
}
