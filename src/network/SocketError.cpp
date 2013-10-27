#include "SocketError.h"
#include <QApplication>

QString SocketError::toString(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::ConnectionRefusedError:
        return qApp->translate("@default" ,"Connection refused");
    case QAbstractSocket::RemoteHostClosedError:
        return qApp->translate("@default" ,"Remote host closed connection");
    case QAbstractSocket::HostNotFoundError:
        return qApp->translate("@default" ,"Host not found");
    case QAbstractSocket::SocketAccessError:
        return qApp->translate("@default" ,"Socket access error");
    case QAbstractSocket::SocketResourceError:
        return qApp->translate("@default" ,"Socket resource error");
    case QAbstractSocket::SocketTimeoutError:
        return qApp->translate("@default" ,"Socket operation timed out");
    case QAbstractSocket::DatagramTooLargeError:
        return qApp->translate("@default" ,"Datagram too large");
    case QAbstractSocket::NetworkError:
        return qApp->translate("@default" ,"Network error");
    case QAbstractSocket::AddressInUseError:
        return qApp->translate("@default" ,"Address already in use");
    case QAbstractSocket::SocketAddressNotAvailableError:
        return qApp->translate("@default" ,"Socket address not available");
    case QAbstractSocket::UnsupportedSocketOperationError:
        return qApp->translate("@default" ,"Unsupported socket operation");
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return qApp->translate("@default" ,"Proxy authentication required");
    case QAbstractSocket::SslHandshakeFailedError:
        return qApp->translate("@default" ,"SSL handshake failed");
    case QAbstractSocket::UnfinishedSocketOperationError:
        return qApp->translate("@default" ,"Unfinished socket operation");
    case QAbstractSocket::ProxyConnectionRefusedError:
        return qApp->translate("@default" ,"Proxy connection refused");
    case QAbstractSocket::ProxyConnectionClosedError:
        return qApp->translate("@default" ,"Proxy connection closed");
    case QAbstractSocket::ProxyConnectionTimeoutError:
        return qApp->translate("@default" ,"Proxy connection timeout");
    case QAbstractSocket::ProxyNotFoundError:
        return qApp->translate("@default" ,"Proxy not found");
    case QAbstractSocket::ProxyProtocolError:
        return qApp->translate("@default" ,"Proxy error");
    case QAbstractSocket::UnknownSocketError:
    default:
        return qApp->translate("@default" ,"Unknown error");
    }
}
