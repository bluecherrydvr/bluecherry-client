#include "MainWindow.h"
#include "liveview/LiveViewWindow.h"
#include "DVRServersView.h"
#include "OptionsDialog.h"
#include "EventsWindow.h"
#include "NumericOffsetWidget.h"
#include "EventsModel.h"
#include "AboutDialog.h"
#include "OptionsServerPage.h"
#include "ServerConfigWindow.h"
#include "EventsView.h"
#include "EventViewWindow.h"
#include "core/DVRServer.h"
#include "core/BluecherryApp.h"
#include <QBoxLayout>
#include <QTreeView>
#include <QGroupBox>
#include <QMenuBar>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QPushButton>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QMacStyle>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QTextDocument>
#include <QShowEvent>
#include <QSystemTrayIcon>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_trayIcon(0)
{
    setWindowTitle(tr("Bluecherry"));
    resize(1100, 750);
    createMenu();
    updateTrayIcon();

    QWidget *centerWidget = new QWidget;
    QBoxLayout *mainLayout = new QHBoxLayout(centerWidget);
    mainLayout->setSpacing(5);

    /* Create left side */
    QBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);

    leftLayout->addWidget(createSourcesList());

    //leftLayout->addWidget(createServerBox());

    mainLayout->addLayout(leftLayout);

    /* Middle */
    m_centerSplit = new QSplitter(Qt::Vertical);
    mainLayout->addWidget(m_centerSplit, 1);

    QFrame *cameraContainer = new QFrame;
    m_centerSplit->addWidget(cameraContainer);

    QBoxLayout *middleLayout = new QVBoxLayout(cameraContainer);
    middleLayout->setSpacing(0);
    middleLayout->setMargin(0);

    /* Controls area */
    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->setSpacing(0);
    middleLayout->addLayout(controlLayout);

    m_liveView = new LiveViewWindow;
    middleLayout->addWidget(m_liveView, 1);

    QPushButton *eventsBtn = new QPushButton(tr("Search Events"));
    connect(eventsBtn, SIGNAL(clicked()), SLOT(showEventsWindow()));
    leftLayout->addWidget(eventsBtn, 0, Qt::AlignCenter);

    /* Recent events */
    QWidget *eventsWidget = createRecentEvents();
    m_centerSplit->addWidget(eventsWidget);
    m_centerSplit->setStretchFactor(0, 1);

    /* Set center widget */
    setCentralWidget(centerWidget);

    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/main/geometry")).toByteArray());
    if (!m_centerSplit->restoreState(settings.value(QLatin1String("ui/main/centerSplit")).toByteArray()))
    {
        m_centerSplit->setSizes(QList<int>() << 1000 << 100);
    }

    QString lastLayout = settings.value(QLatin1String("ui/cameraArea/lastLayout"), tr("Default")).toString();
    m_liveView->setLayout(lastLayout);

    connect(m_liveView, SIGNAL(layoutChanged(QString)), SLOT(liveViewLayoutChanged(QString)));

    connect(bcApp, SIGNAL(sslConfirmRequired(DVRServer*,QList<QSslError>,QSslConfiguration)),
            SLOT(sslConfirmRequired(DVRServer*,QList<QSslError>,QSslConfiguration)));
    connect(bcApp, SIGNAL(queryLivePaused()), SLOT(queryLivePaused()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    if (settings.value(QLatin1String("ui/main/closeToTray"), false).toBool())
    {
        event->ignore();
        hide();
        return;
    }

    emit closing();

    settings.setValue(QLatin1String("ui/main/geometry"), saveGeometry());
    settings.setValue(QLatin1String("ui/main/centerSplit"), m_centerSplit->saveState());
    settings.setValue(QLatin1String("ui/main/eventsView"), m_eventsView->header()->saveState());
    QMainWindow::closeEvent(event);
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

void MainWindow::createMenu()
{
    QMenu *appMenu = menuBar()->addMenu(tr("&Application"));
    appMenu->addAction(tr("Add new &server..."), this, SLOT(addServer()));
    appMenu->addAction(tr("&Options"), this, SLOT(showOptionsDialog()));
    appMenu->addSeparator();
    appMenu->addAction(tr("&Quit"), qApp, SLOT(quit()));

    QMenu *serverMenu = menuBar()->addMenu(tr("&Server"));
    menuServerName = serverMenu->addAction(QString());
    QFont f;
    f.setBold(true);
    menuServerName->setFont(f);
    serverMenu->addSeparator();
    serverMenu->addAction(tr("&Configuration"), this, SLOT(openServerConfig()));
    serverMenu->addAction(tr("&Refresh devices"), this, SLOT(refreshServerDevices()));
    serverMenu->addSeparator();
    serverMenu->addAction(tr("&Edit Server"), this, SLOT(editCurrentServer()));

    connect(serverMenu, SIGNAL(aboutToShow()), SLOT(showServersMenu()));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&Documentation"), this, SLOT(openDocumentation()));
    helpMenu->addAction(tr("Bluecherry &support"), this, SLOT(openSupport()));
    helpMenu->addAction(tr("Suggest a &feature"), this, SLOT(openIdeas()));
    helpMenu->addSeparator();
    helpMenu->addAction(tr("&About Bluecherry DVR"), this, SLOT(openAbout()));
}

void MainWindow::showServersMenu()
{
    DVRServer *server = m_sourcesList->currentServer();
    menuServerName->setText(server ? server->displayName() : tr("No Server"));
}

QWidget *MainWindow::createSourcesList()
{
    m_sourcesList = new DVRServersView;
    m_sourcesList->setMinimumHeight(220);
    m_sourcesList->setFixedWidth(170);

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

    EventsModel *model = new EventsModel(m_eventsView);
    m_eventsView->setModel(model);

    QSettings settings;
    model->setUpdateInterval(settings.value(QLatin1String("ui/main/eventRefreshInterval"), 10000).toInt());
    model->setEventLimit(50);

    m_eventsView->header()->restoreState(settings.value(QLatin1String("ui/main/eventsView")).toByteArray());

    connect(m_eventsView, SIGNAL(doubleClicked(QModelIndex)), m_eventsView, SLOT(openEvent(QModelIndex)));
    connect(m_eventsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventsContextMenu(QPoint)));

    return m_eventsView;
}

void MainWindow::showOptionsDialog()
{
    OptionsDialog *dlg = new OptionsDialog(this);
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
    /* TODO: Get a link for this */
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

void MainWindow::addServer()
{
    OptionsDialog *dlg = new OptionsDialog(this);
    dlg->showPage(OptionsDialog::ServerPage);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    OptionsServerPage *pg = static_cast<OptionsServerPage*>(dlg->pageWidget(OptionsDialog::ServerPage));
    pg->addNewServer();

    dlg->show();
}

void MainWindow::editCurrentServer()
{
    DVRServer *server = m_sourcesList->currentServer();
    if (!server)
        return;

    OptionsDialog *dlg = new OptionsDialog(this);
    dlg->showPage(OptionsDialog::ServerPage);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    OptionsServerPage *pg = static_cast<OptionsServerPage*>(dlg->pageWidget(OptionsDialog::ServerPage));
    pg->setCurrentServer(server);

    dlg->show();
}

void MainWindow::openServerConfig()
{
    DVRServer *server = m_sourcesList->currentServer();
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
                                       .arg(Qt::escape(server->displayName()), server->api->serverUrl().toString(),
                                            fingerprint));
    QPushButton *ab = dlg->addButton(tr("Accept Certificate"), QMessageBox::AcceptRole);
    dlg->setDefaultButton(dlg->addButton(QMessageBox::Cancel));
    dlg->setParent(this);
    dlg->setWindowModality(Qt::WindowModal);

    dlg->exec();
    if (dlg->clickedButton() != ab)
        return;

    server->setKnownCertificate(config.peerCertificate());
}

void MainWindow::eventsContextMenu(const QPoint &pos)
{
    Q_ASSERT(sender() == m_eventsView);

    EventData *eventPtr = m_eventsView->currentIndex().data(EventsModel::EventDataPtr).value<EventData*>();
    if (!eventPtr)
        return;

    /* Make a copy of the event data, because it could be refreshed during QMenu::exec */
    EventData event(*eventPtr);

    QMenu menu(m_eventsView);

    QAction *aOpen = menu.addAction(event.hasMedia() ? tr("Play video") : tr("Open"));
    menu.setDefaultAction(aOpen);

    QAction *aViewLive = 0;
    if (event.isCamera())
        aViewLive = menu.addAction(tr("View camera live"));

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
    if (event.hasMedia())
        menu.addAction(tr("Save video"));
#endif

    QAction *act = menu.exec(m_eventsView->mapToGlobal(pos));
    if (!act)
        return;
    else if (act == aOpen)
        EventViewWindow::open(event);
    else if (act == aViewLive)
        LiveViewWindow::openWindow(this, event.locationCamera())->show();
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
