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

#include "StatusBarServerAlert.h"
#include "ServerConfigWindow.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerRepository.h"
#include <QBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QTextDocument>

StatusBarServerAlert::StatusBarServerAlert(DVRServerRepository *serverRepository, QWidget *parent)
    : QWidget(parent), m_serverRepository(serverRepository)
{
    Q_ASSERT(m_serverRepository);

    QBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addSpacing(8);

    QLabel *icon = new QLabel;
    icon->setPixmap(QIcon(QLatin1String(":/icons/exclamation-yellow.png")).pixmap(16));
    layout->addWidget(icon);

    alertText = new QLabel;
    alertText->setMaximumWidth(700);
    layout->addWidget(alertText);

    setVisible(false);

    connect(m_serverRepository, SIGNAL(serverAlertsChanged()), SLOT(updateAlert()));
}

void StatusBarServerAlert::updateAlert()
{
    QList<DVRServer*> servers = m_serverRepository->serversWithAlerts();
    QString message;

    if (servers.size() == 1)
        message = tr("Alert on %1").arg(servers[0]->configuration()->displayName());
    else if (servers.size() > 1)
        message = tr("Alerts on %1 servers").arg(servers.size());

    alertText->setText(Qt::escape(message));
    setVisible(!message.isEmpty());
}

void StatusBarServerAlert::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton)
        return;

    QList<DVRServer*> servers = m_serverRepository->serversWithAlerts();
    if (servers.isEmpty())
        return;

    ev->accept();
    ServerConfigWindow::instance()->setServer(servers[0]);
    ServerConfigWindow::instance()->show();
    ServerConfigWindow::instance()->raise();
}
