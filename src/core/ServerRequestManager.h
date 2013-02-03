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

#ifndef SERVERREQUESTMANAGER_H
#define SERVERREQUESTMANAGER_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>

class DVRServer;

class ServerRequestManager : public QObject
{
    Q_OBJECT

public:
    enum Status
    {
        LoginError = -2,
        ServerError = -1,
        Offline,
        Online
    };

    DVRServer * const server;

    explicit ServerRequestManager(DVRServer *server);

    bool isOnline() const { return m_status == Online; }
    bool isLoginPending() const { return !isOnline() && m_loginReply; }
    Status status() const { return m_status; }
    QString errorMessage() const { return m_errorMessage; }

    void login(const QString &username, const QString &password);
    void setError(const QString &message) { setStatus(ServerError, message); }

    QUrl serverUrl() const;
    QNetworkRequest buildRequest(const QUrl &relativeUrl);
    QNetworkRequest buildRequest(const QString &relativeUrl) { return buildRequest(QUrl(relativeUrl)); }
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

private:
    QString m_errorMessage;
    QNetworkReply *m_loginReply;
    Status m_status;

    void setStatus(Status status, const QString &errorMessage = QString());
};

#endif // SERVERREQUESTMANAGER_H
