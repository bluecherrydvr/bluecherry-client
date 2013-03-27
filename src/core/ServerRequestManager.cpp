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

#include "ServerRequestManager.h"
#include "server/DVRServer.h"
#include "BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QSslSocket>
#include <QDebug>

ServerRequestManager::ServerRequestManager(DVRServer *s)
    : QObject(s), server(s), m_loginReply(0), m_status(Offline)
{
}

void ServerRequestManager::setStatus(Status s, const QString &errmsg)
{
    if (s == m_status && (errmsg.isEmpty() || errmsg == m_errorMessage))
        return;

    Status old = m_status;
    m_status = s;
    m_errorMessage = errmsg;

    qDebug() << "ServerRequestManager" << server->displayName() << ": Status" << m_status << "message" << m_errorMessage;

    emit statusChanged(m_status);

    switch (m_status)
    {
    case LoginError:
        emit loginError(m_errorMessage);
        break;
    case ServerError:
        emit serverError(m_errorMessage);
        break;
    case Online:
        emit loginSuccessful();
        emit onlineChanged(true);
        break;
    default:
        break;
    }

    if (old == Online && m_status < Online)
    {
        emit disconnected();
        emit onlineChanged(false);
    }
}

QUrl ServerRequestManager::serverUrl() const
{
    QUrl url;
    url.setScheme(QLatin1String("https"));
    url.setHost(server->hostname());
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
    return bcApp->nam->get(request);
}

QNetworkReply *ServerRequestManager::sendRequest(const QUrl &relativeUrl)
{
    return bcApp->nam->get(buildRequest(relativeUrl));
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

    QNetworkRequest req = buildRequest(QLatin1String("/ajax/login.php"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    if (req.url().scheme() == QLatin1String("https") && !QSslSocket::supportsSsl())
    {
        setStatus(ServerError, QLatin1String("SSL support is not enabled"));
        return;
    }

    QUrl queryData;
    queryData.addQueryItem(QLatin1String("login"), username);
    queryData.addQueryItem(QLatin1String("password"), password);
    queryData.addQueryItem(QLatin1String("from_client"), QLatin1String("true"));

    emit loginRequestStarted();

    m_loginReply = bcApp->nam->post(req, queryData.encodedQuery());
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
        setStatus(ServerError, tr("Request failed: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    if (!data.startsWith("OK"))
    {
        setStatus(LoginError, data.isEmpty() ? tr("Unknown error") : QString::fromUtf8(data));
        return;
    }

    if (bcApp->nam->cookieJar()->cookiesForUrl(reply->url()).isEmpty())
    {
        setStatus(LoginError, QLatin1String("No authentication data provided by server"));
        return;
    }

    setStatus(Online);
}

void ServerRequestManager::logout()
{
    if (m_status < Online)
        return;

    setStatus(Offline);
}
