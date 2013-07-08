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

#ifndef SERVERCONFIGWINDOW_H
#define SERVERCONFIGWINDOW_H

#include <QWidget>

class DVRServer;
class QWebView;
class QUrl;

class ServerConfigWindow : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(DVRServer* server READ server WRITE setServer NOTIFY serverChanged)

public:
    explicit ServerConfigWindow(QWidget *parent = 0);
    virtual ~ServerConfigWindow();

    static ServerConfigWindow *instance();

    DVRServer *server() const { return m_server; }

public slots:
    void setServer(DVRServer *server);

signals:
    void serverChanged(DVRServer *server);

private:
    static ServerConfigWindow *m_instance;

    DVRServer *m_server;
    QWebView *m_webView;
};

#endif // SERVERCONFIGWINDOW_H
