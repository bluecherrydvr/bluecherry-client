#include "StatusBarServerAlert.h"
#include "ServerConfigWindow.h"
#include "core/BluecherryApp.h"
#include "core/DVRServer.h"
#include <QTextDocument>
#include <QLabel>
#include <QBoxLayout>
#include <QMouseEvent>

StatusBarServerAlert::StatusBarServerAlert(QWidget *parent)
    : QWidget(parent)
{
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

    connect(bcApp, SIGNAL(serverAlertsChanged()), SLOT(updateAlert()));
}

void StatusBarServerAlert::updateAlert()
{
    QList<DVRServer*> servers = bcApp->serverAlerts();
    QString message;

    if (servers.size() == 1)
        message = tr("Alert on %1").arg(servers[0]->displayName());
    else if (servers.size() > 1)
        message = tr("Alerts on %1 servers").arg(servers.size());

    alertText->setText(Qt::escape(message));
    setVisible(!message.isEmpty());
}

void StatusBarServerAlert::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton)
        return;

    QList<DVRServer*> servers = bcApp->serverAlerts();
    if (servers.isEmpty())
        return;

    ev->accept();
    ServerConfigWindow::instance()->setServer(servers[0]);
    ServerConfigWindow::instance()->show();
    ServerConfigWindow::instance()->raise();
}
