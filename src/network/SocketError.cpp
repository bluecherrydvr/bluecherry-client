#include "SocketError.h"
#include <QApplication>

QString SocketError::toString(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::ConnectionRefusedError:
        return qApp->tr("Connection refused");
    case QAbstractSocket::RemoteHostClosedError:
        return qApp->tr("Remote host closed connection");
    case QAbstractSocket::HostNotFoundError:
        return qApp->tr("Host not found");
    case QAbstractSocket::SocketAccessError:
        return qApp->tr("Socket access error");
    case QAbstractSocket::SocketResourceError:
        return qApp->tr("Socket resource error");
    case QAbstractSocket::SocketTimeoutError:
        return qApp->tr("Socket operation timed out");
    case QAbstractSocket::DatagramTooLargeError:
        return qApp->tr("Datagram too large");
    case QAbstractSocket::NetworkError:
        return qApp->tr("Network error");
    case QAbstractSocket::AddressInUseError:
        return qApp->tr("Address already in use");
    case QAbstractSocket::SocketAddressNotAvailableError:
        return qApp->tr("Socket address not available");
    case QAbstractSocket::UnsupportedSocketOperationError:
        return qApp->tr("Unsupported socket operation");
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return qApp->tr("Proxy authentication required");
    case QAbstractSocket::SslHandshakeFailedError:
        return qApp->tr("SSL handshake failed");
    case QAbstractSocket::UnfinishedSocketOperationError:
        return qApp->tr("Unfinished socket operation");
    case QAbstractSocket::ProxyConnectionRefusedError:
        return qApp->tr("Proxy connection refused");
    case QAbstractSocket::ProxyConnectionClosedError:
        return qApp->tr("Proxy connection closed");
    case QAbstractSocket::ProxyConnectionTimeoutError:
        return qApp->tr("Proxy connection timeout");
    case QAbstractSocket::ProxyNotFoundError:
        return qApp->tr("Proxy not found");
    case QAbstractSocket::ProxyProtocolError:
        return qApp->tr("Proxy error");
    case QAbstractSocket::UnknownSocketError:
    default:
        return qApp->tr("Unknown error");
    }
}
