#include "BluecherryApp.h"
#include "DVRServer.h"
#include "LiveViewManager.h"
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

BluecherryApp *bcApp = 0;

BluecherryApp::BluecherryApp()
    : nam(new QNetworkAccessManager(this)), liveView(new LiveViewManager(this)),
      globalRate(new TransferRateCalculator(this)),
      m_maxServerId(-1), m_livePaused(false), m_inPauseQuery(false),
      m_screensaverInhibited(false)
{
    Q_ASSERT(!bcApp);
    bcApp = this;

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
    sendSettingsChanged();
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
    Q_ASSERT(m_servers.isEmpty());

    QSettings settings;
    settings.beginGroup(QLatin1String("servers"));
    QStringList groups = settings.childGroups();

    foreach (QString group, groups)
    {
        bool ok = false;
        int id = (int)group.toUInt(&ok);
        if (!ok)
        {
            qWarning("Ignoring invalid server ID from configuration");
            continue;
        }

        DVRServer *server = new DVRServer(id, this);
        connect(server, SIGNAL(serverRemoved(DVRServer*)), SLOT(onServerRemoved(DVRServer*)));
        connect(server, SIGNAL(statusAlertMessageChanged(QString)), SIGNAL(serverAlertsChanged()));

        m_servers.append(server);
        m_maxServerId = qMax(m_maxServerId, id);
    }

#ifdef Q_OS_LINUX
    /* If there are no servers configured, and the server application is installed here, automatically
     * configure a local server. */
    if (groups.isEmpty() && QFile::exists(QLatin1String("/etc/bluecherry.conf")))
    {
        DVRServer *s = addNewServer(tr("Local"));
        s->writeSetting("hostname", QHostAddress(QHostAddress::LocalHost).toString());
        /* This must match the default username and password for the server */
        s->writeSetting("username", QLatin1String("Admin"));
        s->writeSetting("password", QLatin1String("bluecherry"));
        s->writeSetting("autoConnect", true);
        QTimer::singleShot(0, s, SLOT(login()));
    }
#endif
}

DVRServer *BluecherryApp::addNewServer(const QString &name)
{
    int id = ++m_maxServerId;

    QSettings settings;
    settings.setValue(QString::fromLatin1("servers/%1/displayName").arg(id), name);

    DVRServer *server = new DVRServer(id, this);
    m_servers.append(server);
    connect(server, SIGNAL(serverRemoved(DVRServer*)), SLOT(onServerRemoved(DVRServer*)));
    connect(server, SIGNAL(statusAlertMessageChanged(QString)), SLOT(serverAlertsChanged()));

    emit serverAdded(server);
    return server;
}

DVRServer *BluecherryApp::findServerID(int id)
{
    for (QList<DVRServer*>::Iterator it = m_servers.begin(); it != m_servers.end(); ++it)
    {
        if ((*it)->configId == id)
            return *it;
    }

    return 0;
}

void BluecherryApp::onServerRemoved(DVRServer *server)
{
    if (m_servers.removeOne(server))
        emit serverRemoved(server);
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
            foreach (DVRServer *s, servers())
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

QList<DVRServer*> BluecherryApp::serverAlerts() const
{
    QList<DVRServer*> re;
    for (QList<DVRServer*>::ConstIterator it = m_servers.begin(); it != m_servers.end(); ++it)
    {
        if (!(*it)->statusAlertMessage().isEmpty())
            re.append(*it);
    }
    return re;
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
