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

#ifndef SETUPWIZARD_P_H
#define SETUPWIZARD_P_H

#include <QWizardPage>
#include <QTimer>

class SetupWelcomePage : public QWizardPage
{
    Q_OBJECT

public:
    SetupWelcomePage();
};

class QLineEdit;
class QNetworkReply;
class QLabel;
class WebRtpPortCheckerWidget;

class SetupServerPage : public QWizardPage
{
    Q_OBJECT

public:
    SetupServerPage();

    virtual void initializePage();
    virtual void cleanupPage();

public slots:
    void save();

private slots:
    void serverChanged();
    void hostTextChanged(const QString &host);
    void setDefaultLogin();

    void testLogin();
    void testLoginDelayed();
    void loginRequestFinished();

protected:
    virtual void hideEvent(QHideEvent *ev);

private:
    QLineEdit *nameEdit;
    WebRtpPortCheckerWidget *portChecker;
    QLabel *testResultIcon;
    QLabel *testResultText;
    QNetworkReply *loginReply;
    QTimer loginRequestTimer;
    bool saved;
};

class SetupFinishPage : public QWizardPage
{
    Q_OBJECT

public:
    SetupFinishPage();

    virtual void initializePage();
    virtual void cleanupPage();
};

#endif // SETUPWIZARD_P_H
