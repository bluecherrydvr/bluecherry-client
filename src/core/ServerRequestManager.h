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

#ifndef SERVERREQUESTMANAGER_H
#define SERVERREQUESTMANAGER_H

#include "server/DVRServer.h"
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>

class ServerRequestManager : public QObject
{
    Q_OBJECT

public:
    DVRServer * const server;

    explicit ServerRequestManager(DVRServer *server);

    bool isOnline() const { return m_status == DVRServer::Online; }
    bool isLoginPending() const { return !isOnline() && m_loginReply; }
    DVRServer::Status status() const { return m_status; }
    QString errorMessage() const { return m_errorMessage; }

    void login(const QString &username, const QString &password);
    void switchSubstream(int device_id, bool substream_enabled);
    void setError(const QString &message) { setStatus(DVRServer::ServerError, message); }

    QUrl serverUrl() const;
    QNetworkRequest buildRequest(const QUrl &relativeUrl);
    QNetworkReply *sendRequest(const QNetworkRequest &request);
    QNetworkReply *sendRequest(const QUrl &relativeUrl);

signals:
    void loginRequestStarted();
    void loginSuccessful();
    void serverError(const QString &message);
    void loginError(const QString &message);
    void disconnected();
    void statusChanged(int status);
    void onlineChanged(bool online);

public slots:
    void logout();

private slots:
    void loginReplyReady();
    void switchSubstreamReplyReady();

private:
    QString m_errorMessage;
    QNetworkReply *m_loginReply;
    QNetworkReply *m_switchSubstreamReply;
    DVRServer::Status m_status;

    void setStatus(DVRServer::Status status, const QString &errorMessage = QString());
};

#endif // SERVERREQUESTMANAGER_H
