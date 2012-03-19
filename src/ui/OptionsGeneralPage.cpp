#include "OptionsGeneralPage.h"
#include "MainWindow.h"
#include "core/BluecherryApp.h"
#include <QCheckBox>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
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
    m_eventsPauseLive->setVisible(false); // Currently not functional
    layout->addWidget(m_eventsPauseLive);

    m_liveHwAccel = new QCheckBox(tr("Use hardware acceleration (OpenGL)"));
    m_liveHwAccel->setChecked(!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), false).toBool());
    m_liveHwAccel->setToolTip(tr("Disable hardware acceleration only if you do not see anything in the live view area."));
    layout->addWidget(m_liveHwAccel);

    m_advancedOpengl = new QCheckBox(tr("Use advanced OpenGL features"));
    m_advancedOpengl->setEnabled(m_liveHwAccel->isChecked());
    connect(m_liveHwAccel, SIGNAL(toggled(bool)), m_advancedOpengl, SLOT(setEnabled(bool)));
    m_advancedOpengl->setChecked(!settings.value(QLatin1String("ui/liveview/disableAdvancedOpengl"), false).toBool());
    m_advancedOpengl->setToolTip(tr("Disable advanced OpenGL features if live video doesn't appear correctly"));
    layout->addWidget(m_advancedOpengl);

    m_deinterlace = new QCheckBox(tr("Automatic deinterlacing"));
    m_deinterlace->setChecked(settings.value(QLatin1String("ui/liveview/autoDeinterlace"), false).toBool());
    layout->addWidget(m_deinterlace);

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    m_ssFullscreen = new QCheckBox(tr("Viewing live or recorded video in fullscreen"));
    m_ssVideo = new QCheckBox(tr("Playing recorded video"));
    m_ssNever = new QCheckBox(tr("Always prevent the computer from going to sleep"));

    m_ssFullscreen->setChecked(settings.value(QLatin1String("ui/disableScreensaver/onFullscreen"), true).toBool());
    m_ssVideo->setChecked(settings.value(QLatin1String("ui/disableScreensaver/onVideo"), true).toBool());
    m_ssNever->setChecked(settings.value(QLatin1String("ui/disableScreensaver/always"), true).toBool());

    connect(m_ssFullscreen, SIGNAL(toggled(bool)), SLOT(ssUpdateForOthers(bool)));
    connect(m_ssVideo, SIGNAL(toggled(bool)), SLOT(ssUpdateForOthers(bool)));
    connect(m_ssNever, SIGNAL(toggled(bool)), SLOT(ssUpdateForNever()));
    ssUpdateForNever();

    layout->addSpacing(10);

    QGroupBox *screensaverSettings = new QGroupBox(tr("Prevent the computer from going to sleep when..."));
    layout->addWidget(screensaverSettings);

    QBoxLayout *ssLayout = new QVBoxLayout(screensaverSettings);
    ssLayout->addWidget(m_ssNever);
    ssLayout->addWidget(m_ssFullscreen);
    ssLayout->addWidget(m_ssVideo);
#else
    m_ssVideo = m_ssFullscreen = m_ssNever = 0;
#endif

    layout->addStretch();
}

void OptionsGeneralPage::ssUpdateForNever()
{
    if (m_ssNever->isChecked())
    {
        m_ssVideo->setChecked(true);
        m_ssFullscreen->setChecked(true);
    }
}

void OptionsGeneralPage::ssUpdateForOthers(bool checked)
{
    if (!checked && m_ssNever->isChecked())
        m_ssNever->setChecked(false);
}

void OptionsGeneralPage::saveChanges()
{
    QSettings settings;
    settings.setValue(QLatin1String("eventPlayer/pauseLive"), m_eventsPauseLive->isChecked());
    bcApp->releaseLive();
    settings.setValue(QLatin1String("ui/main/closeToTray"), m_closeToTray->isChecked());
    bcApp->mainWindow->updateTrayIcon();
    settings.setValue(QLatin1String("ui/liveview/disableHardwareAcceleration"), !m_liveHwAccel->isChecked());
    settings.setValue(QLatin1String("ui/liveview/disableAdvancedOpengl"), !m_advancedOpengl->isChecked());
    settings.setValue(QLatin1String("ui/liveview/autoDeinterlace"), m_deinterlace->isChecked());

    if (m_ssFullscreen && m_ssVideo && m_ssNever)
    {
        /* If 'always' is set, don't enable the other options (even though they appear enabled in the UI),
         * because they would complicate the logic for maintaining the appropriate screensaver state. */
        settings.setValue(QLatin1String("ui/disableScreensaver/onFullscreen"), m_ssFullscreen->isChecked());
        settings.setValue(QLatin1String("ui/disableScreensaver/onVideo"), m_ssVideo->isChecked());
        settings.setValue(QLatin1String("ui/disableScreensaver/always"), m_ssNever->isChecked());
    }

    bcApp->sendSettingsChanged();
}
