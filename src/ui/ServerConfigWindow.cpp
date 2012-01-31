#include "ServerConfigWindow.h"
#include "core/DVRServer.h"
#include "core/BluecherryApp.h"
#include "ui/MainWindow.h"
#include <QBoxLayout>
#include <QWebView>
#include <QWebFrame>
#include <QDesktopServices>

ServerConfigWindow *ServerConfigWindow::m_instance = 0;

ServerConfigWindow *ServerConfigWindow::instance()
{
    if (!m_instance)
    {
        m_instance = new ServerConfigWindow(bcApp->globalParentWindow());
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
    }

    return m_instance;
}

class ConfigWebPage : public QWebPage
{
public:
    ConfigWebPage(QObject *parent)
        : QWebPage(parent)
    {
    }

protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
    {
        if (type == QWebPage::NavigationTypeLinkClicked && request.url().authority() != frame->url().authority())
        {
            QDesktopServices::openUrl(request.url());
            return false;
        }

        return true;
    }
};

ServerConfigWindow::ServerConfigWindow(QWidget *parent)
    : QWidget(parent, Qt::Window), m_server(0)
{
    setWindowTitle(tr("Bluecherry - Server Configuration"));
    setMinimumSize(970, 600);
    resize(minimumSize());

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_webView = new QWebView;
    m_webView->setPage(new ConfigWebPage(m_webView));
    m_webView->page()->setNetworkAccessManager(bcApp->nam);
    layout->addWidget(m_webView);
}

ServerConfigWindow::~ServerConfigWindow()
{
    if (m_instance == this)
        m_instance = 0;
}

void ServerConfigWindow::setServer(DVRServer *server)
{
    if (m_server == server)
        return;

    m_server = server;

    if (server)
    {
        m_webView->load(server->api->serverUrl());
        setWindowTitle(tr("Bluecherry - %1").arg(server->displayName()));
    }

    emit serverChanged(m_server);
}
