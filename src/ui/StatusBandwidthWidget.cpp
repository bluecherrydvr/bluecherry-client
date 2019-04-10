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
