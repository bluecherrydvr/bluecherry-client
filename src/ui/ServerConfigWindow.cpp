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

#include "ServerConfigWindow.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
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
        m_webView->load(server->url());
        setWindowTitle(tr("Bluecherry - %1").arg(server->configuration()->displayName()));
    }

    emit serverChanged(m_server);
}
