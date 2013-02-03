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

#ifndef OPTIONSSERVERPAGE_H
#define OPTIONSSERVERPAGE_H

#include "OptionsDialog.h"

class QTreeView;
class QLineEdit;
class DVRServer;
class QModelIndex;
class QLabel;
class QCheckBox;
class WebRtpPortCheckerWidget;

class OptionsServerPage : public OptionsDialogPage
{
    Q_OBJECT

public:
    explicit OptionsServerPage(QWidget *parent = 0);

    virtual bool hasUnsavedChanges() const;
    virtual void saveChanges();
    void saveChanges(DVRServer *server);

public slots:
    void setCurrentServer(DVRServer *server);
    void addNewServer();

private slots:
    void currentServerChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex);
    void checkServer();

    void deleteServer();

    void setLoginSuccessful();
    void setLoginConnecting();
    void setLoginError(const QString &message);

private:
    QTreeView *m_serversView;
    QLabel *m_connectionStatus;
    QLineEdit *m_nameEdit, *m_hostnameEdit, *m_portEdit, *m_usernameEdit, *m_passwordEdit;
    WebRtpPortCheckerWidget *m_portChecker;
    QCheckBox *m_autoConnect;
};

#endif // OPTIONSSERVERPAGE_H
