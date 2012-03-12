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

    QList<QAction*> fpsActions = bcApp->liveView->fpsActions(bcApp->liveView->globalInterval(),
                                                             bcApp->liveView,
                                                             SLOT(setGlobalIntervalFromAction()));

    foreach (QAction *a, fpsActions)
        a->setParent(m_menu);

    m_menu->addActions(fpsActions);

    [m_button setMenu: m_menu->macMenu()];
    setCocoaView(m_button);

    [m_button release];
    [pool release];

    connect(bcApp->globalRate, SIGNAL(rateUpdated(unsigned)), SLOT(rateUpdated(unsigned)));
    connect(bcApp->liveView, SIGNAL(globalIntervalChanged(int)), SLOT(globalIntervalChanged(int)));
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

void StatusBandwidthWidget::globalIntervalChanged(int interval)
{
    foreach (QAction *a, m_menu->actions())
    {
        if (!a->data().isNull() && a->data().toInt() == interval)
            a->setChecked(true);
        else
            a->setChecked(false);
    }
}
