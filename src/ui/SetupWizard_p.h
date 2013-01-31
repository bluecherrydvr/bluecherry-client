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
