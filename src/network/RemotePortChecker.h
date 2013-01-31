#ifndef REMOTEPORTCHECKER_H
#define REMOTEPORTCHECKER_H

#include <QHostAddress>
#include <QTcpSocket>

class RemotePortChecker : public QObject
{
    Q_OBJECT

public:
    explicit RemotePortChecker(const QString &name, quint16 port, QObject *parent = 0);
    virtual ~RemotePortChecker();

    QString name() const { return m_name; }
    quint16 port() const { return m_port; }

signals:
    void available();
    void notAvailable(const QString &errorMessage);

private slots:
    void checkNow();
    void socketConnected();
    void socketError(QAbstractSocket::SocketError socketError);

private:
    QScopedPointer<QTcpSocket> m_tcpSocket;
    QString m_name;
    quint16 m_port;

    bool isValid() const;
};

#endif // REMOTEPORTCHECKER_H
