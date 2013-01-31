#include "RemotePortCheckerWidget.h"
#include "network/RemotePortChecker.h"

RemotePortCheckerWidget::RemotePortCheckerWidget(const QString &portName, QWidget *parent)
    : QLabel(parent), m_portName(portName), m_lastCheckedPort(0)
{
    setPixmap(QPixmap(QLatin1String(":icons/status-offline.png")));
}

RemotePortCheckerWidget::~RemotePortCheckerWidget()
{
}

void RemotePortCheckerWidget::check(const QString &name, quint16 port)
{
    if (m_lastCheckedName == name && m_lastCheckedPort == port)
        return;

    setPixmap(QPixmap(QLatin1String(":icons/status-offline.png")));
    setChecker(new RemotePortChecker(name, port));
}

void RemotePortCheckerWidget::setChecker(RemotePortChecker *checker)
{
    if (m_checker)
    {
        m_checker->disconnect(this);
        m_checker->deleteLater();
    }

    m_checker.reset(checker);

    if (m_checker)
    {
        connect(m_checker.data(), SIGNAL(available()), this, SLOT(available()));
        connect(m_checker.data(), SIGNAL(notAvailable(QString)), this, SLOT(notAvailable(QString)));
    }
}

void RemotePortCheckerWidget::available()
{
    updateLastChecked();

    if (m_portName.isEmpty())
        setToolTip(QLatin1String("OK"));
    else
        setToolTip(QString::fromLatin1("%1: OK").arg(m_portName));
    setPixmap(QPixmap(QLatin1String(":icons/status.png")));
    setChecker(0);
}

void RemotePortCheckerWidget::notAvailable(const QString &errorMessage)
{
    updateLastChecked();

    if (m_portName.isEmpty())
        setToolTip(errorMessage);
    else
        setToolTip(QString::fromLatin1("%2: %1").arg(errorMessage).arg(m_portName));
    setPixmap(QPixmap(QLatin1String(":icons/status-error.png")));
    setChecker(0);
}

void RemotePortCheckerWidget::updateLastChecked()
{
    Q_ASSERT(m_checker);

    m_lastCheckedName = m_checker->name();
    m_lastCheckedPort = m_checker->port();
}
