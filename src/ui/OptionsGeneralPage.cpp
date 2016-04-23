/*
 * Copyright 2010-2013 Bluecherry
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

#include "OptionsGeneralPage.h"
#include "MainWindow.h"
#include "core/BluecherryApp.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QSystemTrayIcon>

OptionsGeneralPage::OptionsGeneralPage(QWidget *parent)
    : OptionsDialogPage(parent)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    QSettings settings;

	QFormLayout *languagesLayout = new QFormLayout();
	m_languages = new QComboBox();
	fillLanguageComboBox();
	m_languages->setCurrentIndex(m_languages->findData(bcApp->languageController()->currentLanguage()));
	languagesLayout->addRow(new QLabel(tr("Language:")), m_languages);

	layout->addLayout(languagesLayout);

    QFormLayout *mplayervoLayout = new QFormLayout();
    m_mplayervo = new QComboBox();
    fillMplayerVOComboBox();
    m_mplayervo->setCurrentIndex(m_mplayervo->findText(
                                     settings.value(QLatin1String("eventPlayer/mplayer_vo"), QLatin1String("default")).toString()));
    mplayervoLayout->addRow(new QLabel(tr("MPlayer video output driver:")), m_mplayervo);

    layout->addLayout(mplayervoLayout);


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
    m_liveHwAccel->setChecked(!settings.value(QLatin1String("ui/liveview/disableHardwareAcceleration"), true).toBool());
    m_liveHwAccel->setToolTip(tr("Disable hardware acceleration only if you do not see anything in the live view area."));
    layout->addWidget(m_liveHwAccel);

    m_advancedOpengl = new QCheckBox(tr("Use advanced OpenGL features"));
    m_advancedOpengl->setEnabled(m_liveHwAccel->isChecked());
    connect(m_liveHwAccel, SIGNAL(toggled(bool)), m_advancedOpengl, SLOT(setEnabled(bool)));
    m_advancedOpengl->setChecked(!settings.value(QLatin1String("ui/liveview/disableAdvancedOpengl"), false).toBool());
    m_advancedOpengl->setToolTip(tr("Disable advanced OpenGL features if live video doesn't appear correctly"));
    layout->addWidget(m_advancedOpengl);

#if 0// defined(Q_OS_LINUX)
    m_eventPlayerHardwareDecoding = new QCheckBox(tr("Use hardware decoding in event player (vaapi)"));
    m_eventPlayerHardwareDecoding->setChecked(settings.value(QLatin1String("ui/eventplayer/enableHardwareDecoding"), false).toBool());
    m_eventPlayerHardwareDecoding->setToolTip(tr("Disable hardware decoding if you do not see anything in the event player."));
    layout->addWidget(m_eventPlayerHardwareDecoding);
#else
    m_eventPlayerHardwareDecoding = 0;
#endif

    m_deinterlace = new QCheckBox(tr("Automatic deinterlacing"));
    m_deinterlace->setChecked(settings.value(QLatin1String("ui/liveview/autoDeinterlace"), false).toBool());
    layout->addWidget(m_deinterlace);

    m_updateNotifications = new QCheckBox(tr("Disable notifications about available Bluecherry client updates"));
    m_updateNotifications->setChecked(settings.value(QLatin1String("ui/disableUpdateNotifications"), false).toBool());
    layout->addWidget(m_updateNotifications);

    m_thumbnails = new QCheckBox(tr("Show thumbnails for recordings"));
    m_thumbnails->setChecked(settings.value(QLatin1String("ui/enableThumbnails"), true).toBool());
    layout->addWidget(m_thumbnails);

#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_LINUX)
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

void OptionsGeneralPage::fillLanguageComboBox()
{
	QMap<QString, QString> supportedLanguages = bcApp->languageController()->supportedLanguages();
	for (QMap<QString, QString>::const_iterator it = supportedLanguages.constBegin(), end = supportedLanguages.constEnd(); it != end; ++it)
		m_languages->addItem(it.value(), it.key());
}

void OptionsGeneralPage::fillMplayerVOComboBox()
{
    m_mplayervo->addItems(bcApp->mplayerVideoOutputs());
}

void OptionsGeneralPage::saveChanges()
{
    QSettings settings;
    settings.setValue(QLatin1String("eventPlayer/pauseLive"), m_eventsPauseLive->isChecked());
    bcApp->releaseLive();
	settings.setValue(QLatin1String("ui/main/language"), m_languages->itemData(m_languages->currentIndex()));
	bcApp->languageController()->loadLanguage(m_languages->itemData(m_languages->currentIndex()).toString());
    settings.setValue(QLatin1String("eventPlayer/mplayer_vo"), m_mplayervo->itemText(m_mplayervo->currentIndex()));
    settings.setValue(QLatin1String("ui/main/closeToTray"), m_closeToTray->isChecked());
    bcApp->mainWindow->updateTrayIcon();
    settings.setValue(QLatin1String("ui/liveview/disableHardwareAcceleration"), !m_liveHwAccel->isChecked());
    settings.setValue(QLatin1String("ui/liveview/disableAdvancedOpengl"), !m_advancedOpengl->isChecked());
    settings.setValue(QLatin1String("ui/liveview/autoDeinterlace"), m_deinterlace->isChecked());
    settings.setValue(QLatin1String("ui/disableUpdateNotifications"), m_updateNotifications->isChecked());
    settings.setValue(QLatin1String("ui/enableThumbnails"), m_thumbnails->isChecked());
    if (m_eventPlayerHardwareDecoding)
        settings.setValue(QLatin1String("ui/eventplayer/enableHardwareDecoding"), m_eventPlayerHardwareDecoding->isChecked());

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
