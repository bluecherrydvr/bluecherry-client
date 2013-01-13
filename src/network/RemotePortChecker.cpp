#include "RemotePortChecker.h"
#include "network/SocketError.h"
#include <QTcpSocket>
#include <QTimer>

RemotePortChecker::RemotePortChecker(const QString &name, quint16 port, QObject *parent)
    : QObject(parent), m_name(name), m_port(port)
{
    QTimer::singleShot(0, this, SLOT(checkNow()));
}

RemotePortChecker::~RemotePortChecker()
{
}

bool RemotePortChecker::isValid() const
{
    return m_port != 0 && !m_name.isEmpty();
}

void RemotePortChecker::checkNow()
{
    if (!isValid())
    {
        emit notAvailable(tr("Invalid server address"));
        return;
    }

    if (m_tcpSocket)
        m_tcpSocket->disconnect(this);

    m_tcpSocket.reset(new QTcpSocket());
    connect(m_tcpSocket.data(), SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_tcpSocket.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    m_tcpSocket->connectToHost(m_name, m_port);
}

void RemotePortChecker::socketConnected()
{
    emit available();
}

void RemotePortChecker::socketError(QAbstractSocket::SocketError socketError)
{
    emit notAvailable(SocketError::toString(socketError));
}
