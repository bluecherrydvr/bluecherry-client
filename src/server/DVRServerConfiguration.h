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

#ifndef DVRSERVERCONFIGURATION_H
#define DVRSERVERCONFIGURATION_H

#include <QObject>

class DVRServerConfiguration : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DVRServerConfiguration)

public:
    explicit DVRServerConfiguration(int id, QObject *parent = 0);
    virtual ~DVRServerConfiguration();

    int id() const;

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString hostname() const;
    void setHostname(const QString &hostname);

    int port() const;
    void setPort(int port);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    bool autoConnect() const;
    void setAutoConnect(bool autoConnect);

    QByteArray sslDigest() const;
    void setSslDigest(const QByteArray &sslDigest);

    int connectionType() const;
    void setConnectionType(int type);

signals:
    void changed();

private:
    const int m_id;
    QString m_displayName;
    QString m_hostname;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_autoConnect;
    QByteArray m_sslDigest;
    int m_connectionType;

};

#endif // DVRSERVERCONFIGURATION_H
