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
    m_closeToTray->setVisible(QSystemTrayIcon::isSystemTrayAvailable());
    layout->addWidget(m_closeToTray);

    m_eventsPauseLive = new QCheckBox(tr("Pause live feeds while buffering event video"));
    m_eventsPauseLive->setChecked(settings.value(QLatin1String("eventPlayer/pauseLive"), false).toBool());
    m_eventsPauseLive->setToolTip(tr("Pausing live feeds can speed up video downloads over "
                                     "slow internet connections"));
    layout->addWidget(m_eventsPauseLive);

    layout->addStretch();
}

void OptionsGeneralPage::saveChanges()
{
    QSettings settings;
    settings.setValue(QLatin1String("eventPlayer/pauseLive"), m_eventsPauseLive->isChecked());
    settings.setValue(QLatin1String("ui/main/closeToTray"), m_closeToTray->isChecked());
    bcApp->mainWindow->updateTrayIcon();
}
