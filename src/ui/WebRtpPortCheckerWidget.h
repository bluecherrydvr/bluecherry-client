#ifndef WEBRTPPORTCHECKERWIDGET_H
#define WEBRTPPORTCHECKERWIDGET_H

#include <QWidget>

class RemotePortCheckerWidget;

class WebRtpPortCheckerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebRtpPortCheckerWidget(QWidget *parent = 0);
    virtual ~WebRtpPortCheckerWidget();

    void check(const QString &name, quint16 webPort);

private:
    RemotePortCheckerWidget *webPortChecker;
    RemotePortCheckerWidget *rtpPortChecker;
};

#endif // WEBRTPPORTCHECKERWIDGET_H
