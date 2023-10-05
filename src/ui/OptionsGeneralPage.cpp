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
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QDebug>
#include "core/VaapiHWAccel.h"

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

    QFormLayout *mpvvoLayout = new QFormLayout();
    m_mpvvo = new QComboBox();
    fillMpvVOComboBox();
    m_mpvvo->setCurrentIndex(m_mpvvo->findText(
                                     settings.value(QLatin1String("eventPlayer/mpv_vo"), QLatin1String("default")).toString()));
    mpvvoLayout->addRow(new QLabel(tr("MPV video output driver:")), m_mpvvo);

    layout->addLayout(mpvvoLayout);


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

#if defined(Q_OS_LINUX)
    m_vaapiDecodingAcceleration = new QCheckBox(tr("Use hardware acceleration for decoding liveview streams (VAAPI)"));
    m_vaapiDecodingAcceleration->setChecked(settings.value(QLatin1String("ui/liveview/enableVAAPIdecoding"), false).toBool());
    layout->addWidget(m_vaapiDecodingAcceleration);

    if (!bcApp->vaapi->isAvailable())
        m_vaapiDecodingAcceleration->setEnabled(false);
#else
    m_vaapiDecodingAcceleration = 0;
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

    m_session = new QCheckBox(tr("Restore previous session on startup"));
    m_session->setChecked(settings.value(QLatin1String("ui/saveSession"), false).toBool());
    layout->addWidget(m_session);

    m_fullScreen = new QCheckBox(tr("Startup in fullscreen"));
    m_fullScreen->setChecked(settings.value(QLatin1String("ui/startupFullscreen"), false).toBool());
    layout->addWidget(m_fullScreen);

    m_hidetoolbar = new QCheckBox(tr("Hide toolbar in fullscreen"));
    m_hidetoolbar->setChecked(settings.value(QLatin1String("ui/hideFullscreenToolbar"), false).toBool());
    layout->addWidget(m_hidetoolbar);

    m_startup = new QCheckBox(tr("Run on startup"));
    m_startup->setChecked(settings.value(QLatin1String("ui/startup"), false).toBool());
    layout->addWidget(m_startup);

    connect(m_startup, SIGNAL(toggled(bool)), SLOT(updateStartup(bool)));

/*
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
*/
    layout->addStretch();
}
/*
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
*/
void OptionsGeneralPage::fillLanguageComboBox()
{
	QMap<QString, QString> supportedLanguages = bcApp->languageController()->supportedLanguages();
	for (QMap<QString, QString>::const_iterator it = supportedLanguages.constBegin(), end = supportedLanguages.constEnd(); it != end; ++it)
		m_languages->addItem(it.value(), it.key());
}

void OptionsGeneralPage::fillMpvVOComboBox()
{
    m_mpvvo->addItems(bcApp->mpvVideoOutputs());
}

void OptionsGeneralPage::saveChanges()
{
    QSettings settings;
    settings.setValue(QLatin1String("eventPlayer/pauseLive"), m_eventsPauseLive->isChecked());
    bcApp->releaseLive();
	settings.setValue(QLatin1String("ui/main/language"), m_languages->itemData(m_languages->currentIndex()));
	bcApp->languageController()->loadLanguage(m_languages->itemData(m_languages->currentIndex()).toString());
    settings.setValue(QLatin1String("eventPlayer/mpv_vo"), m_mpvvo->itemText(m_mpvvo->currentIndex()));
    settings.setValue(QLatin1String("ui/main/closeToTray"), m_closeToTray->isChecked());
    bcApp->mainWindow->updateTrayIcon();
    settings.setValue(QLatin1String("ui/liveview/autoDeinterlace"), m_deinterlace->isChecked());
    settings.setValue(QLatin1String("ui/disableUpdateNotifications"), m_updateNotifications->isChecked());
    settings.setValue(QLatin1String("ui/enableThumbnails"), m_thumbnails->isChecked());
    settings.setValue(QLatin1String("ui/saveSession"), m_session->isChecked());
    settings.setValue(QLatin1String("ui/startupFullscreen"), m_fullScreen->isChecked());
    settings.setValue(QLatin1String("ui/hideFullscreenToolbar"), m_hidetoolbar->isChecked());
    settings.setValue(QLatin1String("ui/startup"), m_startup->isChecked());

    if (m_vaapiDecodingAcceleration)
        settings.setValue(QLatin1String("ui/liveview/enableVAAPIdecoding"), m_vaapiDecodingAcceleration->isChecked());

//    if (m_ssFullscreen && m_ssVideo && m_ssNever)
//    {
//        /* If 'always' is set, don't enable the other options (even though they appear enabled in the UI),
//         * because they would complicate the logic for maintaining the appropriate screensaver state. */
//        settings.setValue(QLatin1String("ui/disableScreensaver/onFullscreen"), m_ssFullscreen->isChecked());
//        settings.setValue(QLatin1String("ui/disableScreensaver/onVideo"), m_ssVideo->isChecked());
//        settings.setValue(QLatin1String("ui/disableScreensaver/always"), m_ssNever->isChecked());
//    }

    bcApp->sendSettingsChanged();
}

void OptionsGeneralPage::updateStartup(bool on)
{
#if defined(Q_OS_LINUX)

    QString path;
    QDir dir;

    path = QDir::homePath() + QDir::separator() + QString(".config/autostart");
    dir.setPath(path);

    if (!dir.exists(path) && !dir.mkpath(path))
        goto updateStartupFailed;

    path.append(QDir::separator()).append("bluecherry-client.desktop");

    if (on)
    {
        if (!QFile::copy(QString("/usr/share/applications/bluecherry-client.desktop"), path))
            goto updateStartupFailed;
    }
    else
    {
        if (QFile::exists(path) && !QFile::remove(path))
            goto updateStartupFailed;
    }

    return;

updateStartupFailed:

    m_startup->setChecked(on ? false : true);
    qDebug() << "Failed to update startup file!\n";

#elif defined(Q_OS_WIN)

    QString autorun = QString("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings settings(autorun, QSettings::NativeFormat);

    if (on)
    {
        QDir dir;
        QString path = dir.absolutePath() + QDir::separator() + QString("bluecherry-client.exe");
        path = QDir::toNativeSeparators(path);

        settings.setValue("bluecherry-client", path);
    }
    else
        settings.remove("bluecherry-client");

#elif defined(Q_OS_MAC)

    QString path;
    QDir dir;

    path = QDir::homePath() + QDir::separator() + QString("Library/LaunchAgents");
    dir.setPath(path);

    if (!dir.exists(path) && !dir.mkpath(path))
        goto updateStartupFailed;

    path.append(QDir::separator()).append("bluecherry-client.plist");

    if (on)
    {
        const char *data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>Label</key>\n"
        "    <string>bluecherry-client</string>\n"
        "    <key>ProgramArguments</key>\n"
        "    <array>\n"
        "    <string>/Applications/Bluecherry Client.app/Contents/MacOS/bluecherry-client</string>\n"
        "    </array>\n"
        "    <key>ProcessType</key>\n"
        "    <string>Interactive</string>\n"
        "    <key>RunAtLoad</key>\n"
        "    <true/>\n"
        "    <key>KeepAlive</key>\n"
        "    <false/>\n"
        "</dict>\n"
        "</plist>\n";

        QFile plist;
        plist.setFileName(path);

        if (!plist.open(QIODevice::WriteOnly | QIODevice::Text))
            goto updateStartupFailed;

        plist.write(data);
        plist.close();
    }
    else
    {
        QFile plist;
        plist.setFileName(path);
        plist.remove();
    }

    return;

updateStartupFailed:

    m_startup->setChecked(on ? false : true);
    qDebug() << "Failed to update startup file!\n";
#endif
}

