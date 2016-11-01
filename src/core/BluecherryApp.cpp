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

#include "BluecherryApp.h"
#include "LiveViewManager.h"
#include "audio/AudioPlayer.h"
#include "core/UpdateChecker.h"
#include "ui/MainWindow.h"
#include "event/EventDownloadManager.h"
#include "event/ThumbnailManager.h"
#include "network/MediaDownloadManager.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerRepository.h"
#include "video/mplayer/MplVideoPlayerFactory.h"
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHostAddress>
#include <QFile>
#include <QSslError>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAbstractButton>
#include <QDirIterator>
#include <QDebug>

#ifdef Q_OS_LINUX
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#endif

#include "bluecherry-config.h"

BluecherryApp *bcApp = 0;

BluecherryApp::BluecherryApp()
    : nam(new QNetworkAccessManager(this)), liveView(new LiveViewManager(this)),
      audioPlayer(new AudioPlayer(this)),
      globalRate(new TransferRateCalculator(this)), m_updateChecker(0),
      m_livePaused(false), m_inPauseQuery(false),
      m_screensaverInhibited(false), m_screensaveValue(0)
{
    Q_ASSERT(!bcApp);
    bcApp = this;

    m_serverRepository = new DVRServerRepository(this);

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(aboutToQuit()));

    appIcon.addFile(QLatin1String(":/icons/icon16.png"));
    appIcon.addFile(QLatin1String(":/icons/icon32.png"));
    appIcon.addFile(QLatin1String(":/icons/icon64.png"));
    appIcon.addFile(QLatin1String(":/icons/bluecherry-client.png"));
    qApp->setWindowIcon(appIcon);

    connect(nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));

    /* Don't use the system CAs to verify certificates */
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setCaCertificates(QList<QSslCertificate>());
#if QT_VERSION >= 0x040800
    /* SNI breaks connections (before sslError, even) when the hostname does
     * not match the server. */
    sslConfig.setSslOption(QSsl::SslOptionDisableServerNameIndication, true);
#endif
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    clearTempFiles();
    loadServers();
    if (shouldAddLocalServer())
        addLocalServer();
    autoConnectServers();

    sendSettingsChanged();

    m_updateChecker = new UpdateChecker(nam, this);
    connect(m_updateChecker, SIGNAL(newVersionAvailable(Version)), this, SLOT(newVersionAvailable(Version)));
    QSettings settings;

    if (!settings.value(QLatin1String("ui/disableUpdateNotifications"), false).toBool())
    {
        startUpdateChecker();
    }

    m_thumbnailManager = new ThumbnailManager(this);

    m_mediaDownloadManager = new MediaDownloadManager(this);
    m_mediaDownloadManager->setCookieJar(nam->cookieJar());

    m_eventDownloadManager = new EventDownloadManager(this);
    connect(m_serverRepository, SIGNAL(serverRemoved(DVRServer*)), m_eventDownloadManager, SLOT(serverRemoved(DVRServer*)));

    registerVideoPlayerFactory();

    connect(qApp, SIGNAL(commitDataRequest(QSessionManager&)), this, SLOT(commitDataRequest(QSessionManager&)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
}

BluecherryApp::~BluecherryApp()
{
    unregisterVideoPlayerFactory();
}

void BluecherryApp::clearTempFiles()
{
    QStringList nf("bc_vbuf_*.mkv");
    nf << "bc_tmb_*";
    QDirIterator it(QDir::tempPath(), nf, QDir::Files);

    while(it.hasNext())
    {
        //qDebug() << it.next();
        QFile::remove(it.next());
    }

}

void BluecherryApp::startUpdateChecker()
{
    if (m_updateChecker)
        m_updateChecker->start(60 * 60 * 24 * 1000);
}

void BluecherryApp::stopUpdateChecker()
{
    if (m_updateChecker)
        m_updateChecker->stop();
}

void BluecherryApp::registerVideoPlayerFactory()
{
    m_videoPlayerFactory.reset(new MplVideoPlayerFactory());
}

void BluecherryApp::unregisterVideoPlayerFactory()
{
    m_videoPlayerFactory.reset();
}

QStringList BluecherryApp::absolutePaths(const QStringList& paths)
{
    QStringList result;

    foreach (const QString &path, paths)
        if (path.startsWith(QLatin1String("./")))
            result.append(QString::fromAscii("%1/%2").arg(QApplication::applicationDirPath()).arg(path));
        else
            result.append(path);

    return result;
}

QStringList BluecherryApp::mplayerVideoOutputs()
{
    QStringList result;

    result << QString("default");

#ifdef Q_OS_WIN
    result << QString("directx");
    result << QString("directx:noaccel");
    result << QString("direct3d");
    result << QString("gl_nosw");
    result << QString("sdl");
    result << QString("gl");
#endif

#ifdef Q_OS_LINUX
    result << QString("xv");
    result << QString("x11");
    result << QString("gl_nosw");
    result << QString("sdl");
    result << QString("gl");
    result << QString("vdpau");
#endif
    return result;
}

void BluecherryApp::commitDataRequest(QSessionManager &sessionManager)
{
    Q_UNUSED(sessionManager);

    saveSettings();
}

void BluecherryApp::saveSettings()
{
    m_serverRepository->storeServers();
}

void BluecherryApp::newVersionAvailable(const Version &newVersion)
{
    QMessageBox messageBox(mainWindow);
    messageBox.setWindowTitle(tr("Bluecherry"));
    messageBox.setText(tr("A new Bluecherry client update is available"));
    messageBox.setInformativeText(tr("Would you like to download version %2 now?").arg(newVersion.toString()));
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Discard);
    messageBox.button(QMessageBox::Ok)->setText(tr("Download"));
    messageBox.button(QMessageBox::Cancel)->setText(tr("Remind Me Later"));
    messageBox.button(QMessageBox::Discard)->setText(tr("Do Not Show Again"));

    int result = messageBox.exec();
    if (result == QMessageBox::Ok)
    {
        qDebug() << Q_FUNC_INFO << "Upgrading";
        QDesktopServices::openUrl(QString::fromLatin1("http://www.bluecherrydvr.com/downloads/?v=%1").arg(QApplication::applicationVersion()));
    }

    if (result == QMessageBox::Discard)
    {
        stopUpdateChecker();

        QSettings settings;

        settings.setValue(QLatin1String("ui/disableUpdateNotifications"), true);
    }
}

void BluecherryApp::aboutToQuit()
{
    setScreensaverInhibited(false);
    delete m_thumbnailManager;
}

void BluecherryApp::sendSettingsChanged()
{
    emit settingsChanged();

    QSettings settings;
    /* If always is disabled while another condition would keep the screensaver off right now,
     * this will incorrectly turn off inhibition. Detecting this case is complicated. */
    setScreensaverInhibited(settings.value(QLatin1String("ui/disableScreensaver/always"), true).toBool());

    if (settings.value(QLatin1String("ui/disableUpdateNotifications"), false).toBool())
        stopUpdateChecker();
}

void BluecherryApp::setLanguageController(const QSharedPointer<LanguageController> &controller)
{
    m_languageController = controller;
}

QNetworkAccessManager *BluecherryApp::createNam()
{
    QNetworkAccessManager *n = new QNetworkAccessManager(this);
    connect(n, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    return n;
}

void BluecherryApp::loadServers()
{
    m_serverRepository->loadServers();
}

bool BluecherryApp::shouldAddLocalServer() const
{
    if (m_serverRepository->serverCount() > 0)
        return false;

    return QFile::exists(QLatin1String("/etc/bluecherry.conf"));
}

void BluecherryApp::addLocalServer()
{
#ifdef Q_OS_LINUX
    DVRServer *s = m_serverRepository->createServer(tr("Local"));
    s->configuration().setHostname(QHostAddress(QHostAddress::LocalHost).toString());
    /* This must match the default username and password for the server */
    s->configuration().setUsername(QLatin1String("Admin"));
    s->configuration().setPassword(QLatin1String("bluecherry"));
    s->configuration().setPort(7001);
    s->configuration().setAutoConnect(true);
#endif
}

void BluecherryApp::autoConnectServers()
{
    foreach (DVRServer *server, m_serverRepository->servers())
        if (server->configuration().autoConnect() && !server->configuration().hostname().isEmpty() && !server->configuration().username().isEmpty())
            server->login();
}

void BluecherryApp::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_ASSERT(QThread::currentThread() == thread());

    foreach (const QSslError &err, errors)
    {
        switch (err.error())
        {
        case QSslError::CertificateNotYetValid:
        case QSslError::CertificateExpired:
            qDebug() << "BluecherryApp: Ignoring expired SSL certificate";
            break;
        case QSslError::SelfSignedCertificate:
        case QSslError::SelfSignedCertificateInChain:
        case QSslError::HostNameMismatch:
            break;
        default:
            qWarning() << "SSL Error:" << err.errorString() <<(int) err.error();
            return;
        }
    }

    QSslCertificate cert = reply->sslConfiguration().peerCertificate();
    if (cert.isNull())
    {
        qWarning() << "SSL Error: No certificate from peer";
        return;
    }

    /* Figure out which server is associated with this request */
    DVRServer *server = reply->property("DVRServer").value<DVRServer*>();
    if (!server)
    {
        server = qobject_cast<DVRServer*>(reply->request().originatingObject());
        if (!server)
        {
            QUrl requestUrl = reply->request().url();
            foreach (DVRServer *s, m_serverRepository->servers())
            {
                QUrl serverUrl = s->url();
                if (QString::compare(serverUrl.host(), requestUrl.host(), Qt::CaseInsensitive) == 0
                    && serverUrl.port() == requestUrl.port()
                    && QString::compare(serverUrl.scheme(), requestUrl.scheme(), Qt::CaseInsensitive) == 0)
                {
                    server = s;
                    break;
                }
            }

            if (!server)
            {
                qWarning() << "BluecherryApp: Unable to determine server associated with request for" << requestUrl;
                return;
            }
        }
    }

    if (!server->isKnownCertificate(cert))
    {
        qDebug("BluecherryApp: Prompting user to accept different SSL certificate");
        emit sslConfirmRequired(server, errors, reply->sslConfiguration());
        /* If the user accepted, this should now be a known certificate */
        if (!server->isKnownCertificate(reply->sslConfiguration().peerCertificate()))
        {
            server->setError(tr("Unrecognized SSL certificate"));
            return;
        }
    }

    reply->ignoreSslErrors();
}

void BluecherryApp::pauseLive()
{
    if (m_livePaused)
        return;
    m_livePaused = true;
    if (!m_inPauseQuery)
        emit livePausedChanged(m_livePaused);
}

void BluecherryApp::releaseLive()
{
    m_livePaused = false;
    m_inPauseQuery = true;
    emit queryLivePaused();
    m_inPauseQuery = false;

    if (!m_livePaused)
        emit livePausedChanged(false);
}

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

void BluecherryApp::setScreensaverInhibited(bool inhibit)
{
    if (m_screensaverInhibited == inhibit)
        return;

    m_screensaverInhibited = inhibit;
    if (m_screensaverInhibited)
    {
#ifdef Q_OS_WIN
        m_screensaveValue = 0;
        SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &m_screensaveValue, 0);
        if (m_screensaveValue != 0)
            SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0);
#elif defined(Q_OS_MAC)
        m_screensaveTimer = new QTimer(this);
        connect(m_screensaveTimer, SIGNAL(timeout()), SLOT(resetSystemActivity()));
        m_screensaveTimer->start(30000);
        resetSystemActivity();
#elif defined(Q_OS_LINUX)
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"), QLatin1String("/ScreenSaver"),
                                                          QLatin1String("org.freedesktop.ScreenSaver"), QLatin1String("Inhibit"));
        message << QLatin1String("bluecherry");
        message << QLatin1String("");
        QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);
        if (reply.isValid())
            m_screensaveValue = reply.value();
#else
        qCritical("Screensaver inhibit is not supported on this platform");
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, m_screensaveValue, NULL, 0);
#elif defined(Q_OS_MAC)
        delete m_screensaveTimer;
        m_screensaveTimer = 0;
#elif defined(Q_OS_LINUX)
        if (m_screensaveValue)
        {
            QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"), QLatin1String("/ScreenSaver"),
                                                 QLatin1String("org.freedesktop.ScreenSaver"), QLatin1String("UnInhibit"));
            message << m_screensaveValue;
            QDBusConnection::sessionBus().send(message);
        }
#endif
    }
}

#ifdef Q_OS_MAC
void resetSystemActivity(); // PlatformOSX.mm
#endif

void BluecherryApp::resetSystemActivity()
{
#ifdef Q_OS_MAC
    ::resetSystemActivity();
#endif
}

void BluecherryApp::updateStartup(bool on)
{
#if defined(Q_OS_LINUX)

    QString path;
    QDir dir;

    path = QDir::homePath() + QDir::separator() + QString(".config/autostart");
    dir.setPath(path);

    if (!dir.exists(path) || !dir.mkpath(path))
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

    qDebug() << "Failed to update startup file!\n";

#elif defined(Q_OS_WIN)

    QString autorun = QString("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings settings(autorun, QSettings::NativeFormat);

    if (on)
    {
        QDir dir;
        QString path = dir.absolutePath() + QDir::separator() + QString("BluecherryClient.exe");
        path = QDir::toNativeSeparators(path);

        settings.setValue("bluecherry-client", path);
    }
    else
        settings.remove("bluecherry-client");

#elif defined(Q_OS_MAC)

#endif
}

