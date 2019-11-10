/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#include "ServerRequestManager.h"
#include "server/DVRServerConfiguration.h"
#include "BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QDebug>
#include <QNetworkCookie>

ServerRequestManager::ServerRequestManager(DVRServer *s)
    : QObject(s), server(s), m_loginReply(0), m_status(DVRServer::Offline)
{
}

void ServerRequestManager::setStatus(DVRServer::Status s, const QString &errmsg)
{
    if (s == m_status && (errmsg.isEmpty() || errmsg == m_errorMessage))
        return;

    DVRServer::Status old = m_status;
    m_status = s;
    m_errorMessage = errmsg;

    qDebug() << "ServerRequestManager" << server->configuration().displayName() << ": Status" << m_status << "message" << m_errorMessage;

    emit statusChanged(m_status);

    switch (m_status)
    {
    case DVRServer::LoginError:
        emit loginError(m_errorMessage);
        break;
    case DVRServer::ServerError:
        emit serverError(m_errorMessage);
        break;
    case DVRServer::Online:
        emit loginSuccessful();
        emit onlineChanged(true);
        break;
    default:
        break;
    }

    if (old == DVRServer::Online && m_status < DVRServer::Online)
    {
        emit disconnected();
        emit onlineChanged(false);
    }
}

QUrl ServerRequestManager::serverUrl() const
{
    QUrl url;
    url.setScheme(QLatin1String("https"));
    url.setHost(server->configuration().hostname());
    url.setPort(server->serverPort());
    return url;
}

QNetworkRequest ServerRequestManager::buildRequest(const QUrl &relativeUrl)
{
    Q_ASSERT(relativeUrl.isRelative());
    return QNetworkRequest(serverUrl().resolved(relativeUrl));
}

QNetworkReply *ServerRequestManager::sendRequest(const QNetworkRequest &request)
{
    QNetworkReply *result = bcApp->nam->get(request);
    result->ignoreSslErrors();
    return result;
}

QNetworkReply *ServerRequestManager::sendRequest(const QUrl &relativeUrl)
{
    QNetworkReply *result = bcApp->nam->get(buildRequest(relativeUrl));
    result->ignoreSslErrors();
    return result;
}

void ServerRequestManager::login(const QString &username, const QString &password)
{
    if (m_loginReply)
    {
        m_loginReply->disconnect(this);
        m_loginReply->abort();
        m_loginReply->deleteLater();
        m_loginReply = 0;
    }

    /* Essentially duplicated in SetupWizard's server page; changes are probably
     * necessary there too. */

    QNetworkRequest req = buildRequest(QUrl(QLatin1String("/ajax/login.php")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    if (req.url().scheme() == QLatin1String("https") && !QSslSocket::supportsSsl())
    {
        setStatus(DVRServer::ServerError, QLatin1String("SSL support is not enabled"));
        return;
    }

    QUrl queryData;
    queryData.addQueryItem(QLatin1String("login"), username);
    queryData.addQueryItem(QLatin1String("password"), password);
    queryData.addQueryItem(QLatin1String("from_client"), QLatin1String("true"));

    emit loginRequestStarted();

    m_loginReply = bcApp->nam->post(req, queryData.encodedQuery());
    m_loginReply->ignoreSslErrors();
    connect(m_loginReply, SIGNAL(finished()), SLOT(loginReplyReady()));
}

void ServerRequestManager::loginReplyReady()
{
    if (!m_loginReply || sender() != m_loginReply)
        return;

    QNetworkReply *reply = m_loginReply;
    m_loginReply->deleteLater();
    m_loginReply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        setStatus(DVRServer::ServerError, tr("Request failed: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    if (!data.startsWith("OK") && !data.startsWith("{\"status\":\"1\""))
    {
        setStatus(DVRServer::LoginError, data.isEmpty() ? tr("Unknown error") : QString::fromUtf8(data));
        return;
    }

    if (bcApp->nam->cookieJar()->cookiesForUrl(reply->url()).isEmpty())
    {
        setStatus(DVRServer::LoginError, QLatin1String("No authentication data provided by server"));
        return;
    }

    setStatus(DVRServer::Online);
}

void ServerRequestManager::logout()
{
    if (m_status < DVRServer::Online)
        return;

    setStatus(DVRServer::Offline);
}
