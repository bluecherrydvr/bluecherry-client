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

#include "DVRServerConfiguration.h"

DVRServerConfiguration::DVRServerConfiguration(int id, QObject *parent)
    : QObject(parent), m_id(id), m_port(0), m_autoConnect(false)
{
}

DVRServerConfiguration::~DVRServerConfiguration()
{
}

void DVRServerConfiguration::setDisplayName(const QString &name)
{
    if (m_displayName == name)
        return;

    m_displayName = name;
    emit changed();
}

void DVRServerConfiguration::setHostname(const QString &hostname)
{
    if (m_hostname == hostname)
        return;

    m_hostname = hostname;
    emit changed();
}

void DVRServerConfiguration::setPort(int port)
{
    if (m_port == port)
        return;

    m_port = port;
    emit changed();
}

void DVRServerConfiguration::setUsername(const QString &username)
{
    if (m_username == username)
        return;

    m_username = username;
    emit changed();
}

void DVRServerConfiguration::setPassword(const QString &password)
{
    if (m_password == password)
        return;

    m_password = password;
    emit changed();
}

void DVRServerConfiguration::setAutoConnect(bool autoConnect)
{
    if (m_autoConnect == autoConnect)
        return;

    m_autoConnect = autoConnect;
    emit changed();
}

void DVRServerConfiguration::setSslDigest(const QByteArray &sslDigest)
{
    if (m_sslDigest == sslDigest)
        return;

    m_sslDigest = sslDigest;
    emit changed();
}

int DVRServerConfiguration::id() const
{
    return m_id;
}

QString DVRServerConfiguration::displayName() const
{
    return m_displayName;
}

QString DVRServerConfiguration::hostname() const
{
    return m_hostname;
}

int DVRServerConfiguration::port() const
{
    return m_port;
}

QString DVRServerConfiguration::username() const
{
    return m_username;
}

QString DVRServerConfiguration::password() const
{
    return m_password;
}

bool DVRServerConfiguration::autoConnect() const
{
    return m_autoConnect;
}

QByteArray DVRServerConfiguration::sslDigest() const
{
    return m_sslDigest;
}
