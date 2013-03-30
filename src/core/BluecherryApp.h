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

#ifndef BLUECHERRYAPP_H
#define BLUECHERRYAPP_H

#include <QObject>
#include <QList>
#include <QIcon>
#include "core/TransferRateCalculator.h"

class DVRServer;
class DVRServerRepository;
class QNetworkAccessManager;
class MainWindow;
class QNetworkReply;
class QSslError;
class QSslConfiguration;
class QTimer;
class LiveViewManager;
class EventDownloadManager;
class MediaDownloadManager;

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

    DVRServer * serverByID(int id);
    bool serverExists(DVRServer *server) const;

    QList<DVRServer *> serversWithAlerts() const;

    /* Used to create other QNAM instances, for use on other threads.
     * Keeps the correct SSL verification behavior, although changes in fingerprints
     * will error rather than prompting the user on any but the default (GUI thread). */
    QNetworkAccessManager *createNam();

    DVRServerRepository * serverRepository() const { return m_serverRepository; }
    MediaDownloadManager * mediaDownloadManager() const { return m_mediaDownloadManager; }
    EventDownloadManager * eventDownloadManager() const { return m_eventDownloadManager; }

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
    void serverAlertsChanged();

    void sslConfirmRequired(DVRServer *server, const QList<QSslError> &errors, const QSslConfiguration &config);

    void queryLivePaused();
    void livePausedChanged(bool paused);

    void settingsChanged();

private slots:
    void performVersionCheck();
    void versionInfoReceived();
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void aboutToQuit();
    void resetSystemActivity();

private:
    DVRServerRepository *m_serverRepository;
    MediaDownloadManager *m_mediaDownloadManager;
    EventDownloadManager *m_eventDownloadManager;
    bool m_livePaused, m_inPauseQuery, m_screensaverInhibited;
#ifdef Q_OS_WIN
    int m_screensaveValue;
#else
    QTimer *m_screensaveTimer;
#endif
    bool m_doingUpdateCheck;

    void loadServers();
    bool shouldAddLocalServer() const;
    void addLocalServer();
    void autoConnectServers();
};

extern BluecherryApp *bcApp;

inline MainWindow *BluecherryApp::globalParentWindow() const
{
#ifdef Q_OS_MAC
    return 0;
#else
    return mainWindow;
#endif
}

#endif // BLUECHERRYAPP_H
