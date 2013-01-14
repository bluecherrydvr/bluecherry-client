#ifndef REMOTEPORTCHECKERWIDGET_H
#define REMOTEPORTCHECKERWIDGET_H

#include <QHostAddress>
#include <QLabel>
#include <QScopedPointer>

class RemotePortChecker;

class RemotePortCheckerWidget : public QLabel
{
    Q_OBJECT

public:
    explicit RemotePortCheckerWidget(const QString &portName, QWidget *parent = 0);
    virtual ~RemotePortCheckerWidget();

    void check(const QString &name, quint16 port);

private slots:
    void available();
    void notAvailable(const QString &errorMessage);

private:
    QString m_portName;
    QScopedPointer<RemotePortChecker> m_checker;
    QString m_lastCheckedName;
    quint16 m_lastCheckedPort;
    void setChecker(RemotePortChecker *checker);
    void updateLastChecked();
};

#endif // REMOTEPORTCHECKERWIDGET_H
