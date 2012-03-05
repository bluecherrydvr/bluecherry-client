#ifndef BLUECHERRYAPP_H
#define BLUECHERRYAPP_H

#include <QObject>
#include <QList>
#include <QIcon>
#include "core/TransferRateCalculator.h"

class DVRServer;
class QNetworkAccessManager;
class MainWindow;
class QNetworkReply;
class QSslError;
class QSslConfiguration;
class QTimer;
class LiveViewManager;

class BluecherryApp : public QObject
{
    Q_OBJECT

public:
    QNetworkAccessManager * const nam;
    MainWindow *mainWindow;
    QIcon appIcon;
    LiveViewManager * const liveView;
    TransferRateCalculator * const globalRate;

    explicit BluecherryApp();

    MainWindow *globalParentWindow() const;

    QList<DVRServer*> servers() const { return m_servers; }
    DVRServer *addNewServer(const QString &name);
    DVRServer *findServerID(int id);
    bool serverExists(DVRServer *server) { return m_servers.contains(server); }

    /* Used to create other QNAM instances, for use on other threads.
     * Keeps the correct SSL verification behavior, although changes in fingerprints
     * will error rather than prompting the user on any but the default (GUI thread). */
    QNetworkAccessManager *createNam();

    /* Temporarily pause live feeds to free up bandwidth for other intensive transfers
     * (particularly event video buffering). The live feed can be paused with pauseLive(),
     * and released releaseLive(). Upon release, the queryLivePaused() signal will be emitted,
     * and anything still requiring live feeds to be paused should call pauseLive() again. */
    bool livePaused() const { return m_livePaused; }

    void sendSettingsChanged();

    bool screensaverInhibited() const { return m_screensaverInhibited; }

public slots:
    void pauseLive();
    void releaseLive();
    void setScreensaverInhibited(bool inhibit);

signals:
    void serverAdded(DVRServer *server);
    void serverRemoved(DVRServer *server);

    void sslConfirmRequired(DVRServer *server, const QList<QSslError> &errors, const QSslConfiguration &config);

    void queryLivePaused();
    void livePausedChanged(bool paused);

    void settingsChanged();

private slots:
    void onServerRemoved(DVRServer *server);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void aboutToQuit();
    void resetSystemActivity();

private:
    QList<DVRServer*> m_servers;
    int m_maxServerId;
    bool m_livePaused, m_inPauseQuery, m_screensaverInhibited;
#ifdef Q_OS_WIN
    int m_screensaveValue;
#else
    QTimer *m_screensaveTimer;
#endif

    void loadServers();
};

extern BluecherryApp *bcApp;

inline MainWindow *BluecherryApp::globalParentWindow() const
{
#ifdef Q_WS_MAC
    return 0;
#else
    return mainWindow;
#endif
}

#endif // BLUECHERRYAPP_H
