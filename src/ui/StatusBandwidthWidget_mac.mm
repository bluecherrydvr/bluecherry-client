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

StatusBandwidthWidget::StatusBandwidthWidget(QWidget *parent)
    : QMacCocoaViewContainer(0, parent)
{
    /* Work around Qt 4.8.0 bug (causing the widget to not appear) */
    setAttribute(Qt::WA_NativeWindow);

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    m_button = [[NSPopUpButton alloc] init];
    [m_button setShowsBorderOnlyWhileMouseInside: YES];
    [m_button setBezelStyle: NSTexturedRoundedBezelStyle];
    [m_button setPullsDown: YES];
    [[m_button cell] setControlSize: NSSmallControlSize];
    [[m_button cell] setFont: [NSFont controlContentFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];

    m_menu = new QMenu(this);

    m_titleAction = m_menu->addAction(QString());
    m_titleAction->setIcon(QIcon(QLatin1String(":/icons/system-monitor.png")));

    QList<QAction*> fpsActions = bcApp->liveView->bandwidthActions(bcApp->liveView->bandwidthMode(),
                                                                   bcApp->liveView,
                                                                   SLOT(setBandwidthModeFromAction()));

    foreach (QAction *a, fpsActions)
        a->setParent(m_menu);

    m_menu->addActions(fpsActions);

    [m_button setMenu: m_menu->macMenu()];
    setCocoaView(m_button);

    [m_button release];
    [pool release];

    connect(bcApp->globalRate, SIGNAL(rateUpdated(unsigned)), SLOT(rateUpdated(unsigned)));
    connect(bcApp->liveView, SIGNAL(bandwidthModeChanged(int)), SLOT(bandwidthModeChanged(int)));
    rateUpdated(bcApp->globalRate->currentRate());
}

void StatusBandwidthWidget::rateUpdated(unsigned currentRate)
{
    m_titleAction->setText(byteSizeString(currentRate, BytesPerSecond));

    /* Required; otherwise, Qt will add the first item back into the menu after
     * it changes. Cocoa uses this first item as the button. */
    [m_button setMenu: m_menu->macMenu()];

    [m_button sizeToFit];
    NSRect rect = [m_button frame];
    rect.size.width -= 26;
    [m_button setFrameSize: rect.size];
    setFixedSize(rect.size.width + 2, rect.size.height);
}

void StatusBandwidthWidget::bandwidthModeChanged(int value)
{
    foreach (QAction *a, m_menu->actions())
    {
        a->setChecked(value == a->data().toInt());
    }
}
