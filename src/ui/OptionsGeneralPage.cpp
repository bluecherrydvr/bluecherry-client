#include "OptionsGeneralPage.h"
#include "MainWindow.h"
#include "core/BluecherryApp.h"
#include <QCheckBox>
#include <QBoxLayout>
#include <QGridLayout>
#include <QSettings>
#include <QSystemTrayIcon>

OptionsGeneralPage::OptionsGeneralPage(QWidget *parent)
    : OptionsDialogPage(parent)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    QSettings settings;

    m_closeToTray = new QCheckBox(tr("Close to tray"));
    m_closeToTray->setChecked(settings.value(QLatin1String("ui/main/closeToTray"), false).toBool());
    m_closeToTray->setToolTip(tr("When the main window is closed, minimize to the system tray"));
    layout->addWidget(m_closeToTray);
    m_closeToTray->setVisible(QSystemTrayIcon::isSystemTrayAvailable());

    m_eventsPauseLive = new QCheckBox(tr("Pause live feeds while buffering event video"));
    m_eventsPauseLive->setChecked(settings.value(QLatin1String("eventPlayer/pauseLive"), false).toBool());
    m_eventsPauseLive->setToolTip(tr("Pausing live feeds can speed up video downloads over "
                                     "slow internet connections"));
    layout->addWidget(m_eventsPauseLive);

    m_liveHwAccel = new QCheckBox(tr("Use hardware acceleration (OpenGL)"));
    m_liveHwAccel->setChecked(!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), false).toBool());
    m_liveHwAccel->setToolTip(tr("Disable hardware acceleration only if you do not see anything in the live view area."));
    layout->addWidget(m_liveHwAccel);

    layout->addStretch();
}

void OptionsGeneralPage::saveChanges()
{
    QSettings settings;
    settings.setValue(QLatin1String("eventPlayer/pauseLive"), m_eventsPauseLive->isChecked());
    bcApp->releaseLive();
    settings.setValue(QLatin1String("ui/main/closeToTray"), m_closeToTray->isChecked());
    bcApp->mainWindow->updateTrayIcon();
    settings.setValue(QLatin1String("ui/liveview/disableHardwareAcceleration"), !m_liveHwAccel->isChecked());

    bcApp->sendSettingsChanged();
}
