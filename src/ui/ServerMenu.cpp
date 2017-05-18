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

#include "ServerMenu.h"

#include "server/DVRServer.h"

#include <QEvent>

ServerMenu::ServerMenu(DVRServer *server, const QString &title, QWidget *parent)
	: QMenu(title, parent), m_server(server)
{
	setProperty("associatedServer", QVariant::fromValue<QObject*>(m_server));

	createActions();

	connect(m_server, SIGNAL(serverRemoved(DVRServer*)), SLOT(deleteLater()));
	connect(m_server, SIGNAL(changed()), SLOT(updateMenuForServer()));
	connect(m_server, SIGNAL(statusChanged(int)), SLOT(updateMenuForServer()));
}

ServerMenu::~ServerMenu()
{
    m_connectAction->deleteLater();
    m_browseEventsAction->deleteLater();
    m_configureServerAction->deleteLater();
    m_refreshDevicesAction->deleteLater();
    m_settingsAction->deleteLater();
}

void ServerMenu::changeEvent(QEvent *event)
{
	if (event && event->type() == QEvent::LanguageChange)
		retranslateUI();

	QMenu::changeEvent(event);
}

void ServerMenu::createActions()
{
	m_connectAction = addAction(tr("Connect"), m_server, SLOT(toggleOnline()));

	addSeparator();

	m_browseEventsAction = addAction(tr("Browse &events"), this, SIGNAL(showEventsWindow()));
	m_browseEventsAction->setEnabled(m_server->isOnline());
	connect(m_server, SIGNAL(onlineChanged(bool)), m_browseEventsAction, SLOT(setEnabled(bool)));

	m_configureServerAction = addAction(tr("&Configure server"), this, SIGNAL(openServerConfig()));
	m_configureServerAction->setEnabled(m_server->isOnline());
	connect(m_server, SIGNAL(onlineChanged(bool)), m_configureServerAction, SLOT(setEnabled(bool)));

	addSeparator();

    m_refreshDevicesAction = addAction(tr("Refresh devices"), m_server, SLOT(updateCameras()));
    m_refreshDevicesAction->setEnabled(m_server->isOnline());
    connect(m_server, SIGNAL(onlineChanged(bool)), m_refreshDevicesAction, SLOT(setEnabled(bool)));

    m_settingsAction = addAction(tr("Settings"), this, SIGNAL(openServerSettings()));

	updateMenuForServer();
}

void ServerMenu::updateMenuForServer()
{
	setTitle(m_server->configuration().displayName());
	setIcon(QIcon(m_server->isOnline() ? QLatin1String(":/icons/status.png") :
										  QLatin1String(":/icons/status-offline.png")));

	m_connectAction->setText(m_server->isOnline() ? tr("Disconnect") : tr("Connect"));
}


void ServerMenu::retranslateUI()
{
	m_connectAction->setText(m_server->isOnline() ? tr("Disconnect") : tr("Connect"));
	m_browseEventsAction->setText(tr("Browse &events"));
	m_configureServerAction->setText(tr("&Configure server"));
    m_refreshDevicesAction->setText(tr("Refresh devices"));
    m_settingsAction->setText(tr("Settings"));
}
