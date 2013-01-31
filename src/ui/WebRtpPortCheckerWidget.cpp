#include "WebRtpPortCheckerWidget.h"
#include "ui/RemotePortCheckerWidget.h"
#include <QHBoxLayout>

WebRtpPortCheckerWidget::WebRtpPortCheckerWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    webPortChecker = new RemotePortCheckerWidget(tr("Web port"));
    rtpPortChecker = new RemotePortCheckerWidget(tr("RTP port"));
    layout->addWidget(webPortChecker);
    layout->addWidget(rtpPortChecker);
}

WebRtpPortCheckerWidget::~WebRtpPortCheckerWidget()
{
}

void WebRtpPortCheckerWidget::check(const QString &name, quint16 webPort)
{
    webPortChecker->check(name, webPort);
    rtpPortChecker->check(name, webPort + 1);
}
