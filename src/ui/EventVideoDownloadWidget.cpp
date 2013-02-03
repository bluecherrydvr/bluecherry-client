#include "EventVideoDownloadWidget.h"
#include "core/BluecherryApp.h"
#include "core/DVRServer.h"
#include "event/EventVideoDownload.h"
#include "network/MediaDownloadManager.h"
#include "video/MediaDownload.h"
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

EventVideoDownloadWidget::EventVideoDownloadWidget(EventVideoDownload *eventVideoDownload, QWidget *parent)
    : QWidget(parent), m_eventVideoDownload(eventVideoDownload)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(12);
    layout->setSpacing(2);

    const EventData &event = m_eventVideoDownload.data()->eventData();
    QLabel *label = new QLabel(QString::fromLatin1("<b>%2</b> %1<br /> %3")
                               .arg(event.server->displayName())
                               .arg(event.locationCamera().displayName())
                               .arg(event.serverLocalDate().toString())
                               );
    layout->addWidget(label, 0, 0);

    m_closeButton = new QPushButton(tr("Close"));
    m_closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
    layout->addWidget(m_closeButton, 0, 1);
    m_closeButton->hide(); // canceling does not work right now

    m_progressBar = new QProgressBar;
    m_progressBar->setTextVisible(false);
    m_progressBar->setMaximumHeight(8);
    m_progressBar->setRange(0, 0);
    layout->addWidget(m_progressBar, 1, 0, 1, 2);

    m_progressLabel = new QLabel(EventVideoDownload::statusToString(m_eventVideoDownload.data()->status()));
    connect(m_eventVideoDownload.data(), SIGNAL(statusChanged(EventVideoDownload::DownloadStatus)),
            this, SLOT(updateProgress()));
    layout->addWidget(m_progressLabel, 2, 0, 1, 2);

    QFrame *hLine = new QFrame;
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Plain);
    layout->addWidget(hLine, 3, 0, 1, 2);

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, SIGNAL(timeout()), SLOT(updateProgress()));

    updateProgress();
}

EventVideoDownloadWidget::~EventVideoDownloadWidget()
{
}

void EventVideoDownloadWidget::closeClicked()
{
    if (m_eventVideoDownload && m_eventVideoDownload.data()->media())
    {
        MediaDownload *mediaDownload = m_eventVideoDownload.data()->media();
        bcApp->mediaDownloadManager()->releaseMediaDownload(mediaDownload->url());
        m_eventVideoDownload.data()->deleteLater();
        m_eventVideoDownload.clear();
    }

    deleteLater();
}

void EventVideoDownloadWidget::updateProgress()
{
    if (!m_eventVideoDownload || !m_eventVideoDownload.data()->media())

        return;

    EventVideoDownload *eventVideoDownload = m_eventVideoDownload.data();
    MediaDownload *mediaDownload = eventVideoDownload->media();
    m_progressLabel->setText(EventVideoDownload::statusToString(eventVideoDownload->status()));
    m_progressTimer->start(200);

    EventVideoDownload::DownloadStatus status = eventVideoDownload->status();
    switch (status)
    {
    case EventVideoDownload::NotStarted:
    case EventVideoDownload::CopyingFile:
        m_progressBar->setRange(0, 0);
        break;
    case EventVideoDownload::InProgress:
        m_progressBar->setRange(0, mediaDownload->fileSize());
        m_progressBar->setValue(mediaDownload->downloadedSize());
        break;
    case EventVideoDownload::Finished:
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(100);
        m_closeButton->show();
        break;
    case EventVideoDownload::Failed:
        m_progressBar->setRange(0, mediaDownload->fileSize());
        m_progressBar->setValue(0);
        break;
    }

    m_closeButton->setVisible(EventVideoDownload::Finished == status || EventVideoDownload::Failed == status);
}
