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
#include "ui/MainWindow.h"
#include "event/EventDownloadManager.h"
#include "network/MediaDownloadManager.h"
#include "server/DVRServer.h"
#include "server/DVRServerRepository.h"
#include "server/DVRServerSettingsReader.h"
#include "server/DVRServerSettingsWriter.h"
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QHostAddress>
#include <QTimer>
#include <QFile>
#include <QSslError>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAbstractButton>

BluecherryApp *bcApp = 0;

BluecherryApp::BluecherryApp()
    : nam(new QNetworkAccessManager(this)), liveView(new LiveViewManager(this)),
      globalRate(new TransferRateCalculator(this)),
      m_livePaused(false), m_inPauseQuery(false),
      m_screensaverInhibited(false),
      m_doingUpdateCheck(false)
{
    Q_ASSERT(!bcApp);
    bcApp = this;

    m_serverRepository = new DVRServerRepository(this);
    connect(m_serverRepository, SIGNAL(serverAlertsChanged()), this, SIGNAL(serverAlertsChanged()));

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(aboutToQuit()));

    appIcon.addFile(QLatin1String(":/icons/icon16.png"));
    appIcon.addFile(QLatin1String(":/icons/icon32.png"));
    appIcon.addFile(QLatin1String(":/icons/icon64.png"));
    appIcon.addFile(QLatin1String(":/icons/bluecherry.png"));
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

    loadServers();
    if (shouldAddLocalServer())
        addLocalServer();
    autoConnectServers();

    sendSettingsChanged();

    performVersionCheck();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(performVersionCheck()));
    timer->setInterval(60 * 60 * 24 * 1000);
    timer->start();

    m_mediaDownloadManager = new MediaDownloadManager(this);
    m_mediaDownloadManager->setCookieJar(nam->cookieJar());

    m_eventDownloadManager = new EventDownloadManager(this);
}

void BluecherryApp::performVersionCheck()
{
    qDebug() << Q_FUNC_INFO << "Performing update check";

    // we don't want multiple "upgrade?" dialogs
    if (m_doingUpdateCheck) {
        qDebug() << Q_FUNC_INFO << "Check in progress; ignoring";
        return;
    }

    /* perform update check */
    QNetworkRequest versionCheck(QString::fromLatin1("http://keycheck.bluecherrydvr.com/client_version/?v=") + QApplication::applicationVersion());
    QNetworkReply *versionReply = nam->get(versionCheck);

    /* lack of error handling here is deliberate; we really don't care if it
     * fails. a stupid error dialog popping up because the user didn't have
     * internet connectivity (for instance) is worse than being a bit out of date.
     */
    connect(versionReply, SIGNAL(finished()), SLOT(versionInfoReceived()));
}

void BluecherryApp::versionInfoReceived()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << Q_FUNC_INFO << "Error from server: " << reply->errorString();
        return;
    }

    QByteArray versionXmlData = reply->readAll();
    QXmlStreamReader xmlStream(versionXmlData);

    if (xmlStream.hasError() || !xmlStream.readNextStartElement() || xmlStream.name() != QLatin1String("response"))
    {
        qDebug() << Q_FUNC_INFO << "Malformed version info from server";
        qDebug() << versionXmlData;
        qDebug() << xmlStream.errorString();
        return;
    }

    forever {
        if (xmlStream.tokenType() == QXmlStreamReader::StartElement &&
            xmlStream.name() == QLatin1String("version"))
            break;

        if (!xmlStream.readNext()) {
            qWarning() << "Didn't find valid version info";
            return;
        }
    }

    QString latest = xmlStream.attributes().value(QLatin1String("latest")).toString();

    qDebug() << Q_FUNC_INFO << "Latest version info: " << latest;

    if (latest != QApplication::applicationVersion()) {
        qDebug() << Q_FUNC_INFO << "Version differs:";
        qDebug() << Q_FUNC_INFO << "Current: " << QApplication::applicationVersion();
        qDebug() << Q_FUNC_INFO << "New: " << latest;

        /* slight hack to work around timers triggering while a dialog is
         * potentially open: tell the update requester that we're busy so it
         * doesn't go and ask for the latest version again while the dialog is
         * already open.
         *
         * this is only a problem if people don't close the dialog once a day,
         * (and don't upgrade) but we might as well.
         */
        m_doingUpdateCheck = true;

        QMessageBox messageBox(mainWindow);
        messageBox.setWindowTitle(tr("Bluecherry"));
        messageBox.setText(tr("A new Bluecherry client update is available"));
        messageBox.setInformativeText(tr("Would you like to download version %2 now?").arg(latest));
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        messageBox.button(QMessageBox::Ok)->setText(tr("Download"));
        messageBox.button(QMessageBox::Cancel)->setText(tr("Remind Me Later"));

        if (messageBox.exec() == QMessageBox::Ok) {
            qDebug() << Q_FUNC_INFO << "Upgrading";
            QDesktopServices::openUrl(QString::fromLatin1("http://www.bluecherrydvr.com/downloads/?v=%1").arg(QApplication::applicationVersion()));
        }

        m_doingUpdateCheck = false;
    }
}

void BluecherryApp::aboutToQuit()
{
    setScreensaverInhibited(false);
}

void BluecherryApp::sendSettingsChanged()
{
    emit settingsChanged();

    QSettings settings;
    /* If always is disabled while another condition would keep the screensaver off right now,
     * this will incorrectly turn off inhibition. Detecting this case is complicated. */
    setScreensaverInhibited(settings.value(QLatin1String("ui/disableScreensaver/always"), true).toBool());
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
    s->setHostname(QHostAddress(QHostAddress::LocalHost).toString());
    /* This must match the default username and password for the server */
    s->setUsername(QLatin1String("Admin"));
    s->setPassword(QLatin1String("bluecherry"));
    s->setPort(7001);
    s->setAutoConnect(true);

    DVRServerSettingsWriter writer;
    writer.writeServer(s);
#endif
}

bool BluecherryApp::serverExists(DVRServer *server) const
{
    return m_serverRepository->serverExists(server);
}

void BluecherryApp::autoConnectServers()
{
    foreach (DVRServer *server, m_serverRepository->servers())
        if (server->autoConnect() && !server->hostname().isEmpty() && !server->username().isEmpty())
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
                QUrl serverUrl = s->api->serverUrl();
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
            server->api->setError(tr("Unrecognized SSL certificate"));
            return;
        }
    }

    DVRServerSettingsWriter writer;
    writer.writeServer(server);

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

QList<DVRServer *> BluecherryApp::serversWithAlerts() const
{
    return m_serverRepository->serversWithAlerts();
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
