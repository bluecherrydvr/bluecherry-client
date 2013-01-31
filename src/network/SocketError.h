#ifndef SOCKETERROR_H
#define SOCKETERROR_H

#include <QAbstractSocket>

class SocketError
{
    Q_DISABLE_COPY(SocketError)

public:
    static QString toString(QAbstractSocket::SocketError socketError);

private:
    SocketError() {}
};

#endif // SOCKETERROR_H
