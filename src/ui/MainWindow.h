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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "ui/EventVideoDownloadsWindow.h"

class DVRServersView;
class LiveViewWindow;
class EventsModel;
class EventsView;
class EventsWindow;
class QSplitter;
class DVRServer;
class DVRServerRepository;
class QSslError;
class QSslConfiguration;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(DVRServerRepository *serverRepository, QWidget *parent = 0);
    ~MainWindow();

    LiveViewWindow *liveView() const { return m_liveView; }

    QMenu *serverMenu(DVRServer *server);

    void updateTrayIcon();

public slots:
    void showOptionsDialog();
    void showEventsWindow();

    void addServer();
    void openServerConfig();
    void openServerSettings();
    void refreshServerDevices();

    void openDocumentation();
    void openSupport();
    void openIdeas();
    void openAbout();
    void openLiveWindow();

    void showFront();
    void showDownloadsWindow();

private slots:
    void updateMenuForServer(DVRServer *server = 0);
    void updateServersMenu();
    void sslConfirmRequired(DVRServer *server, const QList<QSslError> &errors, const QSslConfiguration &config);
    void trayActivated(QSystemTrayIcon::ActivationReason);
    void queryLivePaused();
    void liveViewLayoutChanged(const QString &layout);
    void updateToolbarWidth();
    void bandwidthModeChanged(int value);
    void eventsContextMenu(const QPoint &pos);
    void saveSettings();
    void onServerAdded(DVRServer *server);
    void serverDevicesLoaded();

signals:
    void closing();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    DVRServerRepository *m_serverRepository;
    DVRServersView *m_sourcesList;
    EventsModel *m_eventsModel;
    EventsView *m_eventsView;
    LiveViewWindow *m_liveView;
    QSplitter *m_centerSplit, *m_leftSplit;
    QMenu *m_serversMenu;
    QSystemTrayIcon *m_trayIcon;
    QToolBar *m_mainToolbar;
    QWidget *serverAlertWidget;
    QLabel *serverAlertText;
    QWeakPointer<EventVideoDownloadsWindow> m_eventVideoDownloadsWindow;
    QWeakPointer<EventsWindow> m_eventsWindow;

    void createMenu();

    QWidget *createSourcesList();
    QWidget *createServerBox();

    QWidget *createRecentEvents();
};

#endif // MAINWINDOW_H
