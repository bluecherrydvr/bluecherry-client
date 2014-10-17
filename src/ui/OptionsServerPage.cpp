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

#include "OptionsServerPage.h"
#include "model/DVRServersModel.h"
#include "model/DVRServersProxyModel.h"
#include "core/BluecherryApp.h"
#include "server/DVRServer.h"
#include "server/DVRServerConfiguration.h"
#include "server/DVRServerConnectionType.h"
#include "server/DVRServerRepository.h"
#include "ui/WebRtpPortCheckerWidget.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTextDocument>
#include <QTreeView>

OptionsServerPage::OptionsServerPage(DVRServerRepository *serverRepository, QWidget *parent)
    : OptionsDialogPage(parent), m_serverRepository(serverRepository)
{
    Q_ASSERT(m_serverRepository);

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    QBoxLayout *topLayout = new QHBoxLayout;
    mainLayout->addLayout(topLayout);

    /* Servers list */
    m_serversView = new QTreeView;

    m_model = new DVRServersModel(serverRepository, false, m_serversView);
    m_proxyModel = new DVRServersProxyModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->sort(0);

    m_serversView->setModel(m_proxyModel);

    m_serversView->header()->setHighlightSections(false);
    m_serversView->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_serversView->header()->setResizeMode(0, QHeaderView::Stretch);
    m_serversView->header()->setStretchLastSection(false);
    m_serversView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_serversView->setMinimumSize(480, 150);
    m_serversView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_serversView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_serversView->setItemsExpandable(false);
    m_serversView->setRootIsDecorated(false);
    topLayout->addWidget(m_serversView);

    connect(m_serversView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(currentServerChanged(QModelIndex,QModelIndex)));

    /* Editing area */
    QGridLayout *editsLayout = new QGridLayout;
    mainLayout->addLayout(editsLayout);

    QLabel *label = new QLabel(tr("Name:"));
    editsLayout->addWidget(label, 0, 0, Qt::AlignRight);

    m_nameEdit = new QLineEdit;
    editsLayout->addWidget(m_nameEdit, 0, 1);

    label = new QLabel(tr("Hostname:"));
    editsLayout->addWidget(label, 1, 0, Qt::AlignRight);

    QBoxLayout *hnLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_hostnameEdit = new QLineEdit;
    hnLayout->addWidget(m_hostnameEdit);

    m_portEdit = new QLineEdit;
    m_portEdit->setValidator(new QIntValidator(1, 65535, m_portEdit));
    m_portEdit->setFixedWidth(50);
    hnLayout->addWidget(m_portEdit);

    m_portChecker = new WebRtpPortCheckerWidget;
    hnLayout->addWidget(m_portChecker);

    editsLayout->addLayout(hnLayout, 1, 1);

    label = new QLabel(tr("Connection Type:"));
    editsLayout->addWidget(label, 2, 0, Qt::AlignRight);

    m_connectionType = new QComboBox;
    m_connectionType->addItem(tr("RTSP"));
    m_connectionType->addItem(tr("MJPEG"));
    editsLayout->addWidget(m_connectionType, 2, 1);

    label = new QLabel(tr("Username:"));
    editsLayout->addWidget(label, 0, 2, Qt::AlignRight);

    m_usernameEdit = new QLineEdit;
    editsLayout->addWidget(m_usernameEdit, 0, 3);

    label = new QLabel(tr("Password:"));
    editsLayout->addWidget(label, 1, 2, Qt::AlignRight);

    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    editsLayout->addWidget(m_passwordEdit, 1, 3);

    m_autoConnect = new QCheckBox(tr("Connect Automatically"));
    m_autoConnect->setChecked(true);
    editsLayout->addWidget(m_autoConnect, 3, 1, 1, 1);

    /* Errors */
    m_connectionStatus = new QLabel;
    m_connectionStatus->setAlignment(Qt::AlignCenter);
    //m_connectionStatus->setContentsMargins(4, 4, 4, 4);
    m_connectionStatus->setVisible(false);
    mainLayout->addWidget(m_connectionStatus);

    /* Buttons */
    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    mainLayout->addWidget(line);

    QBoxLayout *btnLayout = new QHBoxLayout;
    mainLayout->addLayout(btnLayout);

    QPushButton *newBtn = new QPushButton(tr("Add Server"));
    newBtn->setAutoDefault(false);
    connect(newBtn, SIGNAL(clicked()), SLOT(addNewServer()));
    btnLayout->addWidget(newBtn);

    QPushButton *delBtn = new QPushButton(tr("Delete"));
    delBtn->setAutoDefault(false);
    connect(delBtn, SIGNAL(clicked()), SLOT(deleteServer()));
    btnLayout->addWidget(delBtn);

    btnLayout->addStretch();
    QPushButton *applyBtn = new QPushButton(tr("Apply"));
    applyBtn->setAutoDefault(false);
    connect(applyBtn, SIGNAL(clicked()), SLOT(saveChanges()));
    btnLayout->addWidget(applyBtn);

    connect(m_hostnameEdit, SIGNAL(editingFinished()), this, SLOT(checkServer()));
    connect(m_portEdit, SIGNAL(editingFinished()), this, SLOT(checkServer()));
}

void OptionsServerPage::setCurrentServer(DVRServer *server)
{
    QModelIndex index = m_model->indexForServer(server);
    if (!index.isValid())
        return;

    QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);
    if (!proxyIndex.isValid())
        return;
    
    m_serversView->setCurrentIndex(proxyIndex);
}

void OptionsServerPage::currentServerChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    DVRServer *server = oldIndex.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    if (server)
    {
        saveChanges(server);
        server->disconnect(this);
        m_connectionStatus->setVisible(false);
    }

    server = newIndex.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    if (!server)
    {
        m_nameEdit->clear();
        m_hostnameEdit->clear();
        m_portEdit->clear();
        m_usernameEdit->clear();
        m_passwordEdit->clear();
        m_connectionType->setCurrentIndex(DVRServerConnectionType::RTSP);
        return;
    }

    m_nameEdit->setText(server->configuration().displayName());
    m_hostnameEdit->setText(server->configuration().hostname());
    m_portEdit->setText(QString::number(server->serverPort()));
    m_usernameEdit->setText(server->configuration().username());
    m_passwordEdit->setText(server->configuration().password());
    m_autoConnect->setChecked(server->configuration().autoConnect());
    m_connectionType->setCurrentIndex(server->configuration().connectionType());

    connect(server, SIGNAL(loginSuccessful()), SLOT(setLoginSuccessful()));
    connect(server, SIGNAL(loginError(QString)), SLOT(setLoginError(QString)));
    connect(server, SIGNAL(loginRequestStarted()), SLOT(setLoginConnecting()));

    checkServer();

    if (server->isOnline())
        setLoginSuccessful();
    else if (server->isLoginPending())
        setLoginConnecting();
    else if (!server->errorMessage().isEmpty())
        setLoginError(server->errorMessage());
}

void OptionsServerPage::checkServer()
{
    m_portChecker->check(m_hostnameEdit->text(), m_portEdit->text().toUInt());
}

void OptionsServerPage::addNewServer()
{
    DVRServer *server = m_serverRepository->createServer(tr("New Server"));
    server->configuration().setAutoConnect(true);
    server->configuration().setPort(7001);
    server->configuration().setConnectionType(DVRServerConnectionType::RTSP);

    if (!m_serversView->currentIndex().isValid())
        saveChanges(server);
    setCurrentServer(server);

    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void OptionsServerPage::deleteServer()
{
    DVRServer *server = m_serversView->currentIndex().data(DVRServersModel::DVRServerRole).value<DVRServer *>();

    if (!server)
        return;

    QMessageBox dlg(QMessageBox::Question, tr("Delete DVR Server"), tr("Are you sure you want to delete <b>%1</b>?")
                    .arg(Qt::escape(server->configuration().displayName())), QMessageBox::NoButton, this);
    QPushButton *delBtn = dlg.addButton(tr("Delete"), QMessageBox::DestructiveRole);
    dlg.addButton(QMessageBox::Cancel);
    dlg.exec();

    if (dlg.clickedButton() != delBtn)
        return;

    server->removeServer();
}

bool OptionsServerPage::hasUnsavedChanges() const
{
    return m_nameEdit->isModified() || m_hostnameEdit->isModified()
            || m_usernameEdit->isModified() || m_passwordEdit->isModified();
}

void OptionsServerPage::saveChanges()
{
    saveChanges(0);
}

void OptionsServerPage::saveChanges(DVRServer *server)
{
    if (!server)
    {
        server = m_serversView->currentIndex().data(DVRServersModel::DVRServerRole).value<DVRServer *>();
        if (!server)
            return;
    }

    bool connectionModified = false;

    if (m_nameEdit->isModified())
    {
        server->configuration().setDisplayName(m_nameEdit->text().trimmed());
        m_nameEdit->setModified(false);
    }
    if (m_hostnameEdit->isModified())
    {
        m_hostnameEdit->setText(m_hostnameEdit->text().trimmed());
        server->configuration().setHostname(m_hostnameEdit->text());
        m_hostnameEdit->setModified(false);
        connectionModified = true;
    }
    if (m_portEdit->isModified())
    {
        server->configuration().setPort(m_portEdit->text().toInt());
        m_portEdit->setModified(false);
        connectionModified = true;
    }
    if (m_usernameEdit->isModified())
    {
        server->configuration().setUsername(m_usernameEdit->text());
        m_usernameEdit->setModified(false);
        connectionModified = true;
    }
    if (m_passwordEdit->isModified())
    {
        server->configuration().setPassword(m_passwordEdit->text());
        m_passwordEdit->setModified(false);
        connectionModified = true;
    }

    server->configuration().setAutoConnect(m_autoConnect->isChecked());
    server->configuration().setConnectionType(m_connectionType->currentIndex());

    if (connectionModified || (m_autoConnect->isChecked() && !server->isOnline()))
        server->login();

    m_serverRepository->storeServers();
}

void OptionsServerPage::setLoginSuccessful()
{
    m_connectionStatus->setVisible(false);
}

void OptionsServerPage::setLoginConnecting()
{
    m_connectionStatus->setText(tr("Connecting to server..."));
    m_connectionStatus->setVisible(true);
}

void OptionsServerPage::setLoginError(const QString &message)
{
    m_connectionStatus->setText(tr("<b>Login error:</b> %1").arg(Qt::escape(message)));
    m_connectionStatus->setVisible(true);
}
