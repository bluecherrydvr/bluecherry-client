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

#include "SetupWizard.h"
#include "SetupWizard_p.h"
#include "core/BluecherryApp.h"
#include "server/DVRServer.h"
#include "server/DVRServerSettingsWriter.h"
#include "ui/WebRtpPortCheckerWidget.h"
#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QIntValidator>
#include <QMovie>
#include <QVariant>
#include <QNetworkReply>
#include <QHideEvent>

SetupWizard::SetupWizard(QWidget *parent)
    : QWizard(parent), skipFlag(false)
{
    setWindowTitle(tr("Bluecherry - Setup"));

    addPage(new SetupWelcomePage);
    addPage(new SetupServerPage);
    addPage(new SetupFinishPage);
}

bool SetupWizard::validateCurrentPage()
{
    bool ok = QWizard::validateCurrentPage();

    if (ok && !skipFlag && qobject_cast<SetupServerPage*>(currentPage()))
        static_cast<SetupServerPage*>(currentPage())->save();

    return ok;
}

void SetupWizard::skip()
{
    skipFlag = true;
    next();
    skipFlag = false;
}

SetupWelcomePage::SetupWelcomePage()
{
    setTitle(tr("Welcome"));
    setSubTitle(tr("Welcome to the Bluecherry Surveillance DVR! This wizard will help you connect "
                   "to your DVR server and get started."));

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();

    QLabel *logo = new QLabel;
    logo->setPixmap(QPixmap(QLatin1String(":/images/logo.png")));
    logo->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(logo, 0, Qt::AlignHCenter | Qt::AlignBottom);
}

SetupServerPage::SetupServerPage()
    : loginReply(0), saved(false)
{
    loginRequestTimer.setSingleShot(true);
    connect(&loginRequestTimer, SIGNAL(timeout()), SLOT(testLogin()));

    setTitle(tr("Configure a DVR Server"));
    setSubTitle(tr("Setup a connection to your remote DVR server. You can connect to any number of "
                   "servers, from anywhere in the world."));
    setCommitPage(true);
    setButtonText(QWizard::CommitButton, tr("Finish"));
    setButtonText(QWizard::CustomButton1, tr("Skip"));

    QGridLayout *layout = new QGridLayout(this);

    int row = 0;

    layout->setRowMinimumHeight(row, 16);

    row++;
    QLineEdit *hostEdit = new QLineEdit;
    layout->addWidget(new QLabel(tr("Hostname:")), row, 0);
    layout->addWidget(hostEdit, row, 1);

    QLineEdit *portEdit = new QLineEdit;
    portEdit->setValidator(new QIntValidator(1, 65534, portEdit)); // we need 65535 for rtp
    portEdit->setText(QLatin1String("7001"));
    portEdit->setFixedWidth(75);
    layout->addWidget(new QLabel(tr("Port:")), row, 2);
    layout->addWidget(portEdit, row, 3, 1, 1, Qt::AlignRight | Qt::AlignVCenter);

    portChecker = new WebRtpPortCheckerWidget;
    layout->addWidget(portChecker, row, 4);

    row++;
    nameEdit = new QLineEdit;
    layout->addWidget(new QLabel(tr("Name:")), row, 0);
    layout->addWidget(nameEdit, row, 1, 1, 4);

    row++;
    layout->setRowMinimumHeight(row, 16);

    row++;
    QLineEdit *usernameEdit = new QLineEdit;
    layout->addWidget(new QLabel(tr("Username:")), row, 0);
    layout->addWidget(usernameEdit, row, 1);

    QLabel *loginDefault = new QLabel(QLatin1String("<a href='default'>") + tr("Use Default")
                                      + QLatin1String("</a>"));
    loginDefault->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(loginDefault, row, 2, 1, 3, Qt::AlignRight | Qt::AlignVCenter);

    row++;
    QLineEdit *passwordEdit = new QLineEdit;
    layout->addWidget(new QLabel(tr("Password:")), row, 0);
    layout->addWidget(passwordEdit, row, 1);
    QCheckBox *passSaveChk = new QCheckBox(tr("Save password"));
    passSaveChk->setChecked(true);
    passSaveChk->setEnabled(false); // not implemented yet (#565)
    layout->addWidget(passSaveChk, row, 2, 1, 3, Qt::AlignRight | Qt::AlignVCenter);

    row++;
    QCheckBox *autoConnectChk = new QCheckBox(tr("Connect automatically at startup"));
    autoConnectChk->setChecked(true);
    layout->addWidget(autoConnectChk, row, 1, 1, 4);

    row++;
    layout->setRowMinimumHeight(row, 8);

    row++;
    QBoxLayout *loadingLayout = new QHBoxLayout;
    loadingLayout->setMargin(0);

    testResultIcon = new QLabel;
    loadingLayout->addWidget(testResultIcon);

    testResultText = new QLabel;
    testResultText->setStyleSheet(QLatin1String("font-weight:bold"));
    loadingLayout->addWidget(testResultText, 0, Qt::AlignLeft);

    layout->addLayout(loadingLayout, row, 0, 1, 5, Qt::AlignHCenter);

    registerField(QLatin1String("serverName*"), nameEdit);
    registerField(QLatin1String("serverHostname*"), hostEdit);
    registerField(QLatin1String("serverPort"), portEdit);
    registerField(QLatin1String("serverUsername*"), usernameEdit);
    registerField(QLatin1String("serverPassword*"), passwordEdit);
    registerField(QLatin1String("serverPasswordSaved"), passSaveChk);
    registerField(QLatin1String("serverAutoConnect"), autoConnectChk);

    connect(hostEdit, SIGNAL(editingFinished()), SLOT(serverChanged()));
    connect(portEdit, SIGNAL(editingFinished()), SLOT(serverChanged()));
    connect(hostEdit, SIGNAL(textChanged(QString)), SLOT(hostTextChanged(QString)));
    connect(loginDefault, SIGNAL(linkActivated(QString)), SLOT(setDefaultLogin()));

    connect(hostEdit, SIGNAL(textChanged(QString)), SLOT(testLoginDelayed()));
    connect(portEdit, SIGNAL(textChanged(QString)), SLOT(testLoginDelayed()));
    connect(usernameEdit, SIGNAL(textChanged(QString)), SLOT(testLoginDelayed()));
    connect(passwordEdit, SIGNAL(textChanged(QString)), SLOT(testLoginDelayed()));
}

void SetupServerPage::initializePage()
{
    wizard()->setOptions(wizard()->options() | QWizard::HaveCustomButton1);
    connect(wizard(), SIGNAL(customButtonClicked(int)), wizard(), SLOT(skip()));
}

void SetupServerPage::save()
{
    if (!isComplete())
    {
        qDebug("Not saving new server: setup page isn't complete. Probably a bug.");
        return;
    }

    DVRServer *server = bcApp->addNewServer(field(QLatin1String("serverName")).toString());
    server->setHostname(field(QLatin1String("serverHostname")).toString());
    server->setPort(field(QLatin1String("serverPort")).toInt());
    server->setUsername(field(QLatin1String("serverUsername")).toString());
    server->setPassword(field(QLatin1String("serverPassword")).toString());
    server->setAutoConnect(field(QLatin1String("serverAutoConnect")).toBool());

    DVRServerSettingsWriter writer;
    writer.writeServer(server);

    server->login();

    saved = true;
}

void SetupServerPage::hideEvent(QHideEvent *ev)
{
    if (!ev->spontaneous() && loginReply)
    {
        loginReply->disconnect(this);
        loginReply->abort();
        loginReply->deleteLater();
        loginReply = 0;
    }

    QWizardPage::hideEvent(ev);
}

void SetupServerPage::cleanupPage()
{
    if (loginReply)
    {
        loginReply->disconnect(this);
        loginReply->abort();
        loginReply->deleteLater();
        loginReply = 0;
    }

    if (testResultIcon->movie())
    {
        delete testResultIcon->movie();
        testResultIcon->setMovie(0);
    }
    if (testResultIcon->pixmap())
        testResultIcon->setPixmap(QPixmap());

    testResultText->clear();

    QWizardPage::cleanupPage();
    wizard()->button(QWizard::CustomButton1)->setVisible(false);
    wizard()->setOptions(wizard()->options() & ~QWizard::HaveCustomButton1);
}

void SetupServerPage::serverChanged()
{
    QString serverHostname = field(QLatin1String("serverHostname")).toString();
    int serverPort = field(QLatin1String("serverPort")).toUInt();

    portChecker->check(serverHostname, serverPort);
}

void SetupServerPage::hostTextChanged(const QString &host)
{
    if (!nameEdit->isModified())
        nameEdit->setText(host);
}

void SetupServerPage::setDefaultLogin()
{
    setField(QLatin1String("serverUsername"), QLatin1String("Admin"));
    setField(QLatin1String("serverPassword"), QLatin1String("bluecherry"));
}

void SetupServerPage::testLoginDelayed()
{
    if (loginReply)
    {
        loginReply->disconnect(this);
        loginReply->abort();
        loginReply->deleteLater();
        loginReply = 0;
    }

    loginRequestTimer.start(300);
}

void SetupServerPage::testLogin()
{
    if (loginReply)
    {
        loginReply->disconnect(this);
        loginReply->abort();
        loginReply->deleteLater();
        loginReply = 0;
    }

    if (loginRequestTimer.isActive())
        loginRequestTimer.stop();

    QString hostname = field(QLatin1String("serverHostname")).toString();
    int port = field(QLatin1String("serverPort")).toInt();
    QString username = field(QLatin1String("serverUsername")).toString();
    QString password = field(QLatin1String("serverPassword")).toString();

    if (hostname.isEmpty() || port < 1 || port > 65535 || username.isEmpty() || password.isEmpty())
        return;

    /* UI */
    if (!testResultIcon->movie())
    {
        testResultIcon->setPixmap(QPixmap());
        QMovie *loadingMovie = new QMovie(QLatin1String(":/images/loading-circle.gif"), "gif", testResultIcon);
        testResultIcon->setMovie(loadingMovie);
        loadingMovie->start();
    }
    testResultText->setText(tr("Connecting..."));

    /* It'd be nice to find a better solution than (essentially) duplicating the
     * logic from ServerRequestManager, but we can't create a DVRServer yet, and
     * allowing that to operate without a DVRServer would be a non-trivial change. */
    QUrl url;
    url.setScheme(QLatin1String("https"));
    url.setHost(hostname);
    url.setPort(port);
    url.setPath(QLatin1String("/ajax/login.php"));

    QUrl queryData;
    queryData.addQueryItem(QLatin1String("login"), username);
    queryData.addQueryItem(QLatin1String("password"), password);
    queryData.addQueryItem(QLatin1String("from_client"), QLatin1String("true"));

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    loginReply = bcApp->nam->post(req, queryData.encodedQuery());
    loginReply->ignoreSslErrors();
    connect(loginReply, SIGNAL(finished()), SLOT(loginRequestFinished()));
}

void SetupServerPage::loginRequestFinished()
{
    if (!loginReply || sender() != loginReply)
        return;

    QNetworkReply *reply = loginReply;
    loginReply->deleteLater();
    loginReply = 0;

    if (testResultIcon->movie())
    {
        testResultIcon->movie()->deleteLater();
        testResultIcon->setMovie(0);
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        testResultIcon->setPixmap(QPixmap(QLatin1String(":/icons/exclamation-red.png")));
        testResultText->setText(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    if (!data.startsWith("OK"))
    {
        testResultIcon->setPixmap(QPixmap(QLatin1String(":/icons/exclamation-red.png")));
        if (!data.isEmpty())
            testResultText->setText(QString::fromLatin1(data));
        else
            testResultText->setText(tr("Unknown login error"));
        return;
    }

    testResultIcon->setPixmap(QPixmap(QLatin1String(":/icons/tick.png")));
    testResultText->setText(tr("Login successful! Click <b>Finish</b> to continue."));
}

SetupFinishPage::SetupFinishPage()
{
    setTitle(tr("Let's Go!"));
    setSubTitle(tr("Here's some tips on how to get started:"));
    setButtonText(QWizard::FinishButton, tr("Close"));

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->addSpacing(8);

    QLabel *text = new QLabel;
    text->setWordWrap(true);
    text->setText(tr("<ul><li>Cameras are shown on the left. You can double-click or drag a "
                     "camera into the live view area to view it<br></li>"
                     "<li>Use the buttons above the live view to create, save, and switch "
                     "layouts - even with cameras from multiple servers!<br></li>"
                     "<li>Double-click on a server to open its configuration page in a new "
                     "window, where you can configure cameras and recordings<br></li>"
                     "<li>Click the events icon ( <img src=':/icons/cassette.png'> ) to open the "
                     "event browser and watch or save recordings<br></li>"
                     "</ul>"
                     ));
    text->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layout->addWidget(text);

    text = new QLabel(tr("If you need help, browse our online documentation through the Help menu at "
                         "the top of the window."));
    text->setWordWrap(true);
    layout->addWidget(text);

    layout->addStretch();
}

void SetupFinishPage::initializePage()
{
    wizard()->button(QWizard::CancelButton)->setVisible(false);
    wizard()->button(QWizard::CustomButton1)->setVisible(false);
}

void SetupFinishPage::cleanupPage()
{
    wizard()->button(QWizard::CancelButton)->setVisible(true);
}
