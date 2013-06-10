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

#include "MainWindow.h"
#include "liveview/LiveViewWindow.h"
#include "event/CameraEventFilter.h"
#include "event/EventDownloadManager.h"
#include "event/EventList.h"
#include "event/MediaEventFilter.h"
#include "DVRServersView.h"
#include "EventsWindow.h"
#include "AboutDialog.h"
#include "OptionsServerPage.h"
#include "ServerConfigWindow.h"
#include "EventsView.h"
#include "EventViewWindow.h"
#include "SetupWizard.h"
#include "MacSplitter.h"
#include "StatusBandwidthWidget.h"
#include "StatusBarServerAlert.h"
#include "model/DVRServersModel.h"
#include "model/DVRServersProxyModel.h"
#include "server/DVRServerRepository.h"
#include "core/BluecherryApp.h"
#include "core/LiveViewManager.h"
#include "event/ModelEventsCursor.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QSettings>
#include <QPushButton>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QMacStyle>
#include <QSslConfiguration>
#include <QTextDocument>
#include <QShowEvent>
#include <QHeaderView>
#include <QToolBar>
#include <QStatusBar>

MainWindow::MainWindow(DVRServerRepository *serverRepository, QWidget *parent)
    : QMainWindow(parent), m_serverRepository(serverRepository), m_trayIcon(0)
{
    Q_ASSERT(m_serverRepository);

    bcApp->mainWindow = this;
    connect(bcApp->eventDownloadManager(), SIGNAL(eventVideoDownloadAdded(EventVideoDownload*)),
            this, SLOT(showDownloadsWindow()));

    setUnifiedTitleAndToolBarOnMac(true);
    setWindowTitle(tr("Bluecherry"));
    resize(1100, 750);
    createMenu();
    updateTrayIcon();

    statusBar()->addPermanentWidget(new StatusBandwidthWidget(statusBar()));
    statusBar()->addWidget(new StatusBarServerAlert(m_serverRepository, statusBar()));

#ifdef Q_OS_MAC
    statusBar()->setSizeGripEnabled(false);
    if (style()->inherits("QMacStyle"))
        statusBar()->setFixedHeight(24);
#endif

    /* Experimental toolbar */
    m_mainToolbar = new QToolBar(tr("Main"));
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setIconSize(QSize(16, 16));
    m_mainToolbar->addAction(QIcon(QLatin1String(":/icons/cassette.png")), tr("Events"), this, SLOT(showEventsWindow()));
    QAction *expandAllServersAction = m_mainToolbar->addAction(QIcon(QLatin1String(":/icons/expand-all.png")), tr("Expand All Servers"));
    QAction *collapseAllServersAction = m_mainToolbar->addAction(QIcon(QLatin1String(":/icons/collapse-all.png")), tr("Collapse All Servers"));
    addToolBar(Qt::TopToolBarArea, m_mainToolbar);

    /* Splitters */
    m_leftSplit = new MacSplitter(Qt::Horizontal);
    m_centerSplit = new MacSplitter(Qt::Vertical);

    /* Live view */
    m_liveView = new LiveViewWindow(this);

    /* Recent events */
    QWidget *eventsWidget = createRecentEvents();

    /* Layouts */
    m_leftSplit->addWidget(createSourcesList());
    m_leftSplit->addWidget(m_centerSplit);
    m_leftSplit->setStretchFactor(1, 1);
    m_leftSplit->setCollapsible(0, false);
    m_leftSplit->setCollapsible(1, false);

    m_centerSplit->addWidget(m_liveView);
    m_centerSplit->addWidget(eventsWidget);
    m_centerSplit->setStretchFactor(0, 1);
    m_centerSplit->setCollapsible(0, false);
    m_centerSplit->setCollapsible(1, false);

    /* Set center widget */
    QWidget *center = new QWidget;
    QBoxLayout *centerLayout = new QVBoxLayout(center);
    centerLayout->setMargin(0);
    centerLayout->setSpacing(0);
    centerLayout->addWidget(m_leftSplit, 1);
    setCentralWidget(center);

#ifdef Q_OS_WIN
    /* There is no top border on the statusbar on Windows, and we need one. */
    if (style()->inherits("QWindowsStyle"))
    {
        QFrame *line = new QFrame;
        line->setFrameStyle(QFrame::Plain | QFrame::HLine);
        QPalette p = line->palette();
        p.setColor(QPalette::WindowText, QColor(171, 175, 183));
        line->setPalette(p);
        line->setFixedHeight(1);
        centerLayout->addWidget(line);
    }
#endif

    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/main/geometry")).toByteArray());
    if (!m_centerSplit->restoreState(settings.value(QLatin1String("ui/main/centerSplit")).toByteArray()))
    {
        m_centerSplit->setSizes(QList<int>() << 1000 << 130);
    }
    if (!m_leftSplit->restoreState(settings.value(QLatin1String("ui/main/leftSplit")).toByteArray()))
    {
#ifdef Q_OS_MAC
        m_leftSplit->setSizes(QList<int>() << 210 << 1000);
#else
        m_leftSplit->setSizes(QList<int>() << 190 << 1000);
#endif
    }

    m_leftSplit->setHandleWidth(2);
    m_centerSplit->setHandleWidth(2);

    QString lastLayout = settings.value(QLatin1String("ui/cameraArea/lastLayout"), tr("Default")).toString();
    m_liveView->setLayout(lastLayout);

    connect(m_liveView, SIGNAL(layoutChanged(QString)), SLOT(liveViewLayoutChanged(QString)));
    connect(m_leftSplit, SIGNAL(splitterMoved(int,int)), SLOT(updateToolbarWidth()));
    updateToolbarWidth();

    connect(bcApp, SIGNAL(sslConfirmRequired(DVRServer*,QList<QSslError>,QSslConfiguration)),
            SLOT(sslConfirmRequired(DVRServer*,QList<QSslError>,QSslConfiguration)));
    connect(bcApp, SIGNAL(queryLivePaused()), SLOT(queryLivePaused()));
    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(onServerAdded(DVRServer*)));

    foreach (DVRServer *s, m_serverRepository->servers())
        onServerAdded(s);

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(saveSettings()));

    m_sourcesList->setFocus(Qt::OtherFocusReason);

    connect(expandAllServersAction, SIGNAL(triggered()), m_sourcesList, SLOT(expandAll()));
    connect(collapseAllServersAction, SIGNAL(triggered()), m_sourcesList, SLOT(collapseAll()));
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
    {
        if (m_serverRepository->serverCount() == 0)
            addServer();
    }
    else
        bcApp->releaseLive();

    QMainWindow::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous())
        bcApp->pauseLive();

    QMainWindow::hideEvent(event);
}

void MainWindow::queryLivePaused()
{
    if (isHidden())
        bcApp->pauseLive();
}

void MainWindow::saveSettings()
{
    if (liveView())
        liveView()->saveLayout();

    QSettings settings;
    settings.setValue(QLatin1String("ui/main/geometry"), saveGeometry());
    settings.setValue(QLatin1String("ui/main/centerSplit"), m_centerSplit->saveState());
    settings.setValue(QLatin1String("ui/main/leftSplit"), m_leftSplit->saveState());
    settings.setValue(QLatin1String("ui/main/eventsView"), m_eventsView->header()->saveState());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QSettings settings;
    if (settings.value(QLatin1String("ui/main/closeToTray"), false).toBool())
    {
        event->ignore();
        hide();
        return;
    }

    emit closing();
    QMainWindow::closeEvent(event);
    QApplication::quit();
}

void MainWindow::updateTrayIcon()
{
    QSettings settings;
    if (bool(m_trayIcon) == settings.value(QLatin1String("ui/main/closeToTray"), false))
        return;

    if (m_trayIcon)
    {
        m_trayIcon->hide();
        m_trayIcon->deleteLater();
        m_trayIcon = 0;
    }
    else
    {
        m_trayIcon = new QSystemTrayIcon(windowIcon(), this);
        m_trayIcon->setToolTip(tr("Bluecherry Client"));

        QMenu *menu = new QMenu(this);
        menu->setDefaultAction(menu->addAction(tr("Open Bluecherry Client"), this, SLOT(showFront())));
        menu->addAction(tr("Quit"), qApp, SLOT(quit()));
        m_trayIcon->setContextMenu(menu);

        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

        m_trayIcon->show();
    }
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        if (isHidden())
            showFront();
        else
            hide();
    }
}

void MainWindow::showFront()
{
    show();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
}

void MainWindow::showDownloadsWindow()
{
    if (!m_eventVideoDownloadsWindow)
    {
        m_eventVideoDownloadsWindow = new EventVideoDownloadsWindow();
        m_eventVideoDownloadsWindow.data()->setEventDownloadManager(bcApp->eventDownloadManager());
    }

    m_eventVideoDownloadsWindow.data()->show();
    m_eventVideoDownloadsWindow.data()->raise();
}

void MainWindow::createMenu()
{
    QMenu *appMenu = menuBar()->addMenu(tr("&Bluecherry"));
    appMenu->addAction(tr("Browse &events"), this, SLOT(showEventsWindow()));
    appMenu->addAction(tr("Show &download manager"), this, SLOT(showDownloadsWindow()));
    appMenu->addSeparator();
    appMenu->addAction(tr("Add another server"), this, SLOT(addServer()));
    appMenu->addAction(tr("&Options"), this, SLOT(showOptionsDialog()));
    appMenu->addSeparator();
    appMenu->addAction(tr("&Quit"), qApp, SLOT(quit()));

    m_serversMenu = menuBar()->addMenu(QString());
    updateServersMenu();

    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(updateServersMenu()));
    connect(m_serverRepository, SIGNAL(serverRemoved(DVRServer*)), SLOT(updateServersMenu()));

    QMenu *liveMenu = menuBar()->addMenu(tr("&Live"));
    liveMenu->addAction(tr("New window"), this, SLOT(openLiveWindow()));
    liveMenu->addSeparator();
    liveMenu->setObjectName(QLatin1String("globalLiveMenu"));

    QList<QAction*> fpsActions = bcApp->liveView->bandwidthActions(bcApp->liveView->bandwidthMode(),
                                                                   bcApp->liveView,
                                                                   SLOT(setBandwidthModeFromAction()));
    foreach (QAction *a, fpsActions)
        a->setParent(liveMenu);
    liveMenu->addActions(fpsActions);

    connect(bcApp->liveView, SIGNAL(bandwidthModeChanged(int)), SLOT(bandwidthModeChanged(int)));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&Documentation"), this, SLOT(openDocumentation()));
    helpMenu->addAction(tr("Bluecherry &support"), this, SLOT(openSupport()));
    helpMenu->addAction(tr("Suggest a &feature"), this, SLOT(openIdeas()));
    helpMenu->addSeparator();
    helpMenu->addAction(tr("&About Bluecherry"), this, SLOT(openAbout()));
}

QMenu *MainWindow::serverMenu(DVRServer *server)
{
    QMenu *m = static_cast<QMenu*>(server->property("uiMenu").value<QObject*>());
    if (m)
        return m;

    m = new QMenu(server->configuration().displayName());

    m->addAction(tr("Connect"), server, SLOT(toggleOnline()))->setObjectName(QLatin1String("aConnect"));
    m->addSeparator();

    QAction *a = m->addAction(tr("Browse &events"), this, SLOT(showEventsWindow()));
    a->setEnabled(server->isOnline());
    connect(server, SIGNAL(onlineChanged(bool)), a, SLOT(setEnabled(bool)));

    a = m->addAction(tr("&Configure server"), this, SLOT(openServerConfig()));
    a->setProperty("associatedServer", QVariant::fromValue<QObject*>(server));
    a->setEnabled(server->isOnline());
    connect(server, SIGNAL(onlineChanged(bool)), a, SLOT(setEnabled(bool)));

    m->addSeparator();

    a = m->addAction(tr("Refresh devices"), server, SLOT(updateCameras()));
    a->setEnabled(server->isOnline());
    connect(server, SIGNAL(onlineChanged(bool)), a, SLOT(setEnabled(bool)));

    a = m->addAction(tr("Settings"), this, SLOT(openServerSettings()));
    a->setProperty("associatedServer", QVariant::fromValue<QObject*>(server));

    connect(server, SIGNAL(serverRemoved(DVRServer*)), m, SLOT(deleteLater()));
    connect(server, SIGNAL(changed()), SLOT(updateMenuForServer()));
    connect(server, SIGNAL(statusChanged(int)), SLOT(updateMenuForServer()));

    server->setProperty("uiMenu", QVariant::fromValue<QObject*>(m));
    updateMenuForServer(server);

    return m;
}

void MainWindow::updateMenuForServer(DVRServer *server)
{
    if (!server)
    {
        server = qobject_cast<DVRServer*>(sender());
        /* Handle ServerRequestManager signals by testing the object's parent as well */
        if (!server && (!sender() || !(server = qobject_cast<DVRServer*>(sender()->parent()))))
            return;
    }

    QMenu *m = serverMenu(server);
    m->setTitle(server->configuration().displayName());
    m->setIcon(QIcon(server->isOnline() ? QLatin1String(":/icons/status.png") :
                                          QLatin1String(":/icons/status-offline.png")));

    QAction *connect = m->findChild<QAction*>(QLatin1String("aConnect"));
    Q_ASSERT(connect);
    if (connect)
        connect->setText(server->isOnline() ? tr("Disconnect") : tr("Connect"));
}

void MainWindow::updateServersMenu()
{
    m_serversMenu->clear();

    QList<DVRServer *> servers = m_serverRepository->servers();
    m_serversMenu->setTitle((servers.size() > 1) ? tr("&Servers") : tr("&Server"));

    if (servers.isEmpty())
    {
        m_serversMenu->addAction(tr("Add a server"), this, SLOT(addServer()));
        return;
    }

    if (servers.size() == 1)
    {
        QMenu *sm = serverMenu(servers.first());
        m_serversMenu->addActions(sm->actions());
        return;
    }

    qSort(servers.begin(), servers.end(), DVRServer::lessThan);
    foreach (DVRServer *s, servers)
        m_serversMenu->addMenu(serverMenu(s));
}

QWidget *MainWindow::createSourcesList()
{
    m_sourcesList = new DVRServersView;
    m_sourcesList->setMinimumHeight(220);
    m_sourcesList->setFrameStyle(QFrame::NoFrame);
    m_sourcesList->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_sourcesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sourcesList->setMinimumWidth(140);

    DVRServersModel *model = new DVRServersModel(bcApp->serverRepository(), true, m_sourcesList);
    model->setOfflineDisabled(true);

    DVRServersProxyModel *proxyModel = new DVRServersProxyModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setHideDisabledCameras(true);
    proxyModel->setSourceModel(model);
    proxyModel->sort(0);

    m_sourcesList->setModel(proxyModel);

#ifdef Q_OS_MAC
    if (style()->inherits("QMacStyle"))
    {
        QLinearGradient grd(0, 0, 0, 1);
        grd.setCoordinateMode(QGradient::ObjectBoundingMode);
        grd.setColorAt(0, QColor(233, 237, 241));
        grd.setColorAt(1, QColor(209, 216, 223));

        QPalette p = m_sourcesList->palette();
        p.setBrush(QPalette::Base, QBrush(grd));
        m_sourcesList->setPalette(p);
    }
#endif

    return m_sourcesList;
}

QWidget *MainWindow::createServerBox()
{
    QGroupBox *box = new QGroupBox(tr("Server Information"));

    /* Placeholder */
    box->setMinimumHeight(200);

    return box;
}

QWidget *MainWindow::createRecentEvents()
{
    m_eventsView = new EventsView;
    m_eventsView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_eventsView->setFrameStyle(QFrame::NoFrame);
    m_eventsView->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_eventsModel = new EventsModel(bcApp->serverRepository(), m_eventsView);
    m_eventsView->setModel(m_eventsModel);

    QSettings settings;
    m_eventsModel->setUpdateInterval(settings.value(QLatin1String("ui/main/eventRefreshInterval"), 10000).toInt());
    m_eventsModel->setEventLimit(50);
    m_eventsModel->setIncompleteEventsFirst(true);

    m_eventsView->header()->restoreState(settings.value(QLatin1String("ui/main/eventsView")).toByteArray());

    connect(m_eventsView, SIGNAL(doubleClicked(QModelIndex)), m_eventsView, SLOT(openEvent(QModelIndex)));
    connect(m_eventsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventsContextMenu(QPoint)));

    return m_eventsView;
}

void MainWindow::showOptionsDialog()
{
    OptionsDialog *dlg = new OptionsDialog(m_serverRepository, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::showEventsWindow()
{
    EventsWindow *window = EventsWindow::instance();
    window->show();
    window->raise();
}

void MainWindow::openAbout()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
    {
        if (qobject_cast<AboutDialog*>(w))
        {
            w->raise();
            return;
        }
    }

    AboutDialog *dlg = new AboutDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::openDocumentation()
{
    QUrl url(QLatin1String("http://help.bluecherrydvr.com/s/manuals/m/version2"));
    bool ok = QDesktopServices::openUrl(url);
    if (!ok)
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while opening %1").arg(url.toString()));
}

void MainWindow::openIdeas()
{
    QUrl url(QLatin1String("http://ideas.bluecherrydvr.com/"));
    bool ok = QDesktopServices::openUrl(url);
    if (!ok)
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while opening %1").arg(url.toString()));
}

void MainWindow::openSupport()
{
    QUrl url(QLatin1String("http://support.bluecherrydvr.com/tickets/new"));
    bool ok = QDesktopServices::openUrl(url);
    if (!ok)
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while opening %1").arg(url.toString()));
}

void MainWindow::openLiveWindow()
{
    LiveViewWindow::openWindow(this, false)->show();
}

void MainWindow::addServer()
{
    SetupWizard *dlg = new SetupWizard(m_serverRepository, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->show();
}

void MainWindow::openServerSettings()
{
    DVRServer *server = 0;
    if (sender())
        server = qobject_cast<DVRServer*>(sender()->property("associatedServer").value<QObject*>());
    if (!server)
        return;

    OptionsDialog *dlg = new OptionsDialog(m_serverRepository, this);
    dlg->showPage(OptionsDialog::ServerPage);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    OptionsServerPage *pg = static_cast<OptionsServerPage*>(dlg->pageWidget(OptionsDialog::ServerPage));
    pg->setCurrentServer(server);

    dlg->show();
}

void MainWindow::openServerConfig()
{
    DVRServer *server = 0;
    if (sender())
        server = qobject_cast<DVRServer*>(sender()->property("associatedServer").value<QObject*>());
    if (!server)
        return;

    ServerConfigWindow::instance()->setServer(server);
    ServerConfigWindow::instance()->show();
    ServerConfigWindow::instance()->raise();
}

void MainWindow::refreshServerDevices()
{
    DVRServer *server = m_sourcesList->currentServer();
    if (!server)
        return;

    server->updateCameras();
}

void MainWindow::sslConfirmRequired(DVRServer *server, const QList<QSslError> &errors, const QSslConfiguration &config)
{
    Q_UNUSED(errors);
    Q_ASSERT(server);
    Q_ASSERT(!config.peerCertificate().isNull());

    if (server->property("ssl_verify_dialog").value<QObject*>())
        return;

    QByteArray digest = config.peerCertificate().digest(QCryptographicHash::Sha1);
    QString fingerprint = QString::fromLatin1(digest.toHex()).toUpper();
    for (int i = 4; i < fingerprint.size(); i += 5)
        fingerprint.insert(i, QLatin1Char(' '));

    QMessageBox *dlg = new QMessageBox(QMessageBox::Warning, tr("Security warning"),
                                       tr("The SSL certificate for <b>%1</b> has changed! This could indicate an "
                                          "attack on the secure connection, or that the server has recently been "
                                          "reinstalled.<br><br><b>Server:</b> %1<br><b>URL:</b> %2<br>"
                                          "<b>Fingerprint:</b> %3<br><br>Do you want to connect anyway, and trust "
                                          "this certificate in the future?")
                                       .arg(Qt::escape(server->configuration().displayName()), server->url().toString(),
                                            fingerprint));
    server->setProperty("ssl_verify_dialog", QVariant::fromValue<QObject*>(dlg));
    QPushButton *ab = dlg->addButton(tr("Accept Certificate"), QMessageBox::AcceptRole);
    dlg->setDefaultButton(dlg->addButton(QMessageBox::Cancel));
    dlg->setParent(this);
    dlg->setWindowModality(Qt::WindowModal);

    dlg->exec();
    server->setProperty("ssl_verify_dialog", QVariant());
    if (dlg->clickedButton() != ab)
        return;

    server->setKnownCertificate(config.peerCertificate());
}

void MainWindow::eventsContextMenu(const QPoint &pos)
{
    Q_ASSERT(sender() == m_eventsView);

    EventList selectedEvents = m_eventsView->selectedEvents();
    EventList selectedMediaEvents = selectedEvents.filter(MediaEventFilter());
    EventList selectedCameraEvents = selectedEvents.filter(CameraEventFilter());

    QMenu menu(m_eventsView);

    QAction *aOpen = menu.addAction(tr("Play video"));
    aOpen->setEnabled(selectedMediaEvents.size() == 1);
    menu.setDefaultAction(aOpen);

    QAction *aViewLive = 0;
    aViewLive = menu.addAction(tr("View camera live"));
    aViewLive->setEnabled(!selectedCameraEvents.isEmpty());

    menu.addSeparator();

    /* Disabled for now. Browse needs logic to update the UI when model filters change,
     * and save video is not yet implemented here. */
#if 0
    QMenu *searchMenu = menu.addMenu(tr("Browse events..."));

    QAction *aBrowseCamera = 0;
    if (event.isCamera())
        aBrowseCamera = searchMenu->addAction(tr("From this camera"));
    QAction *aBrowseServer = searchMenu->addAction(tr("From this server"));
    searchMenu->addSeparator();
    QAction *aBrowseMinute = searchMenu->addAction(tr("Within one minute"));
    QAction *aBrowseTenMin = searchMenu->addAction(tr("Within 10 minutes"));
    QAction *aBrowseHour = searchMenu->addAction(tr("Within one hour"));

    menu.addSeparator();
#endif

    QAction *aSaveVideo = 0;
    aSaveVideo = menu.addAction(tr("Save video"));
    aSaveVideo->setEnabled(!selectedMediaEvents.isEmpty());

    QAction *act = menu.exec(m_eventsView->mapToGlobal(pos));
    if (!act)
        return;
    if (act == aOpen)
    {
        EventData event(selectedMediaEvents.at(0));
        ModelEventsCursor *modelEventsCursor = new ModelEventsCursor();
        modelEventsCursor->setModel(m_eventsModel);
        modelEventsCursor->setCameraFilter(event.locationCamera());
        modelEventsCursor->setIndex(m_eventsView->currentIndex().row());
        EventViewWindow::open(event, modelEventsCursor);
    }
    else if (act == aViewLive)
    {
        QSet<DVRCamera *> cameras = selectedCameraEvents.cameras();
        foreach (DVRCamera *camera, cameras)
            LiveViewWindow::openWindow(this, false, camera)->show();
    }
    else if (act == aSaveVideo)
    {
        if (selectedMediaEvents.size() == 1)
            bcApp->eventDownloadManager()->startEventDownload(selectedMediaEvents.at(0));
        else
            bcApp->eventDownloadManager()->startMultipleEventDownloads(selectedMediaEvents);
    }
#if 0
    else if (act->parentWidget() == searchMenu)
    {
        EventsWindow *w = EventsWindow::instance();

        EventsModel *model = w->model();
        model->clearFilters();

        QDateTime date = event.serverLocalDate();

        if (act == aBrowseCamera)
            model->setFilterSource(event.locationCamera());
        else if (act == aBrowseServer)
            model->setFilterSource(event.server);
        else if (act == aBrowseMinute)
            model->setFilterDates(date.addSecs(-60), date.addSecs(60 + qMax(0, event.duration)));
        else if (act == aBrowseTenMin)
            model->setFilterDates(date.addSecs(-600), date.addSecs(600 + qMax(0, event.duration)));
        else if (act == aBrowseHour)
            model->setFilterDates(date.addSecs(-3600), date.addSecs(3600 + qMax(0, event.duration)));
        else
            Q_ASSERT_X(false, "Set events window filter", "Unknown action");

        showEventsWindow();
    }
#endif
}

void MainWindow::liveViewLayoutChanged(const QString &layout)
{
    QSettings settings;
    settings.setValue(QLatin1String("ui/cameraArea/lastLayout"), layout);
}

void MainWindow::updateToolbarWidth()
{
    /* Magic numbers used to align the live view toolbar with the separator
     * in most cases */
    if (style()->inherits("QMacStyle"))
        m_mainToolbar->setFixedWidth(m_sourcesList->width()-20);
    else if (style()->inherits("QWindowsXPStyle"))
        m_mainToolbar->setFixedWidth(m_sourcesList->width()+1);
    else
        m_mainToolbar->setFixedWidth(m_sourcesList->width() - (2*style()->pixelMetric(QStyle::PM_ToolBarItemMargin)) - 1);
}

void MainWindow::bandwidthModeChanged(int value)
{
    QMenu *menu = menuBar()->findChild<QMenu*>(QLatin1String("globalLiveMenu"));
    foreach (QAction *a, menu->actions())
    {
        if (!a->data().isNull())
            a->setChecked(a->data().toInt() == value);
    }
}

void MainWindow::onServerAdded(DVRServer *server)
{
    connect(server, SIGNAL(devicesReady()), SLOT(serverDevicesLoaded()));
}

void MainWindow::serverDevicesLoaded()
{
    DVRServer *server = qobject_cast<DVRServer*>(sender());
    if (!server)
        return;

    if (server->cameras().isEmpty())
    {
        /* On OS X, trying to open this modal dialog while another modal dialog is open
         * (such as the setup wizard) causes both to become impossible to use; work around
         * by parenting this to the current modal widget if there is one, which in practice
         * is likely the setup wizard. As a result, the configuration dialog will come up
         * before the wizard has ended, but that isn't much of a problem. */
        QMessageBox msg(qApp->activeModalWidget() ? qApp->activeModalWidget() : this);
        if (m_serverRepository->serverCount() > 1)
            msg.setText(tr("%1 hasn't been configured yet").arg(Qt::escape(server->configuration().displayName())));
        else
            msg.setText(tr("The server hasn't been configured yet"));
        msg.setInformativeText(tr("You can access configuration at any time by double-clicking on the server.<br><br>Do you want to configure devices for this server now?"));
        QPushButton *yes = msg.addButton(tr("Configure"), QMessageBox::YesRole);
        msg.addButton(tr("Later"), QMessageBox::NoRole);
        msg.setDefaultButton(yes);
        msg.setWindowModality(Qt::WindowModal);
        msg.exec();
        if (msg.clickedButton() == yes)
        {
            ServerConfigWindow::instance()->setServer(server);
            ServerConfigWindow::instance()->show();
            ServerConfigWindow::instance()->raise();
        }
    }
}
