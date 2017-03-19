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
class QAction;
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

    void saveTopWindow(QWidget *w);

private slots:
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

protected:
	virtual void changeEvent(QEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual bool event(QEvent *event);

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

	QAction *m_expandAllServersAction;
	QAction *m_collapseAllServersAction;

	QMenu *m_appMenu;
	QAction *m_browseEventsAction;
	QAction *m_downloadManagerAction;
	QAction *m_addServerAction;
	QAction *m_optionsAction;
	QAction *m_quitAction;

	QMenu *m_liveMenu;
	QAction *m_newWindowAction;

	QMenu *m_helpMenu;
	QAction *m_documentationAction;
	QAction *m_supportAction;
	QAction *m_suggestionsAction;
	QAction *m_aboutAction;


    void createMenu();

    QWidget *createSourcesList();

    QWidget *createRecentEvents();

    void updateLiveMenu();

    void retranslateUI();
};

#endif // MAINWINDOW_H
