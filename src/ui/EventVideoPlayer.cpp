#include "EventVideoPlayer.h"
#include "EventVideoDownload.h"
#include "video/VideoContainer.h"
#include "video/GstSinkWidget.h"
#include "video/VideoHttpBuffer.h"
#include "core/BluecherryApp.h"
#include "ui/MainWindow.h"
#include "utils/FileUtils.h"
#include "core/EventData.h"
#include <QBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QApplication>
#include <QThread>
#include <QFrame>
#include <QFileDialog>
#include <QShortcut>
#include <QMenu>
#include <QDebug>
#include <QToolTip>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>

EventVideoPlayer::EventVideoPlayer(QWidget *parent)
    : QWidget(parent), m_event(0), m_video(0), m_videoWidget(0)
{
    connect(bcApp, SIGNAL(queryLivePaused()), SLOT(queryLivePaused()));
    connect(&m_posTimer, SIGNAL(timeout()), SLOT(updatePosition()));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_videoContainer = new VideoContainer;
    m_videoContainer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_videoContainer, SIGNAL(customContextMenuRequested(QPoint)), SLOT(videoContextMenu(QPoint)));
    layout->addWidget(m_videoContainer, 1);

    QBoxLayout *sliderLayout = new QHBoxLayout;
    layout->addLayout(sliderLayout);

    m_seekSlider = new QSlider(Qt::Horizontal);
    connect(m_seekSlider, SIGNAL(valueChanged(int)), SLOT(seek(int)));
    m_seekSlider->setEnabled(false);
    sliderLayout->addWidget(m_seekSlider);

    m_posText = new QLabel;
    sliderLayout->addWidget(m_posText);

    QBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(3);
    layout->addLayout(btnLayout);

    m_playBtn = new QToolButton;
    m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
    btnLayout->addWidget(m_playBtn);
    connect(m_playBtn, SIGNAL(clicked()), SLOT(playPause()));

    btnLayout->addSpacing(9);

    m_restartBtn = new QToolButton;
    m_restartBtn->setIcon(QIcon(QLatin1String(":/icons/control-stop-180.png")));
    btnLayout->addWidget(m_restartBtn);
    connect(m_restartBtn, SIGNAL(clicked()), SLOT(restart()));

    btnLayout->addSpacing(14);

    m_statusText = new QLabel;
    m_statusText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    btnLayout->addWidget(m_statusText);

    btnLayout->addStretch();

    m_saveBtn = new QPushButton(tr("Save Video"));
    btnLayout->addWidget(m_saveBtn);
    connect(m_saveBtn, SIGNAL(clicked()), SLOT(saveVideo()));

    QShortcut *sc = new QShortcut(QKeySequence(Qt::Key_Space), m_videoContainer);
    connect(sc, SIGNAL(activated()), SLOT(playPause()));

    sc = new QShortcut(QKeySequence(Qt::Key_F), m_videoContainer);
    connect(sc, SIGNAL(activated()), m_videoContainer, SLOT(toggleFullScreen()));

    sc = new QShortcut(QKeySequence(Qt::Key_R), m_videoContainer);
    connect(sc, SIGNAL(activated()), SLOT(restart()));

    sc = new QShortcut(QKeySequence::Save, m_videoContainer);
    connect(sc, SIGNAL(activated()), SLOT(saveVideo()));

    sc = new QShortcut(QKeySequence(Qt::Key_F5), m_videoContainer);
    connect(sc, SIGNAL(activated()), SLOT(saveSnapshot()));

    setControlsEnabled(false);
}

EventVideoPlayer::~EventVideoPlayer()
{
    bcApp->disconnect(SIGNAL(queryLivePaused()), this);
    bcApp->releaseLive();

    if (m_video)
    {
        m_video->clear();
        delete m_video;
    }
}

void EventVideoPlayer::setVideo(const QUrl &url, EventData *event)
{
    if (m_video)
        clearVideo();

    if (url.isEmpty())
        return;

    m_event = event;

    m_video = new VideoPlayerBackend(this);
    connect(m_video, SIGNAL(stateChanged(int,int)), SLOT(stateChanged(int)));
    connect(m_video, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(m_video, SIGNAL(endOfStream()), SLOT(durationChanged()));

    m_videoWidget = new GstSinkWidget;
    m_videoContainer->setInnerWidget(m_videoWidget);
    m_video->setSink(m_videoWidget->gstElement());

    if (!m_video->start(url))
    {
        /* TODO: Proper error handling! */
        qWarning() << "EventVideoPlayer: start error:" << m_video->errorMessage();
        clearVideo();
        return;
    }

    connect(m_video->videoBuffer(), SIGNAL(bufferingStopped()), SLOT(bufferingStopped()), Qt::QueuedConnection);
    connect(m_video->videoBuffer(), SIGNAL(bufferingStarted()), SLOT(bufferingStarted()));
    connect(m_video->videoBuffer(), SIGNAL(bufferUpdated()), SLOT(updateBufferStatus()));

    if (m_video->videoBuffer()->isBuffering())
        bufferingStarted();

    setControlsEnabled(true);
}

void EventVideoPlayer::clearVideo()
{
    if (m_video)
    {
        m_video->clear();
        delete m_video;
    }

    m_video = 0;
    m_event = 0;

    m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
    m_seekSlider->setRange(0, 0);
    m_posText->clear();
    m_statusText->clear();
    m_posTimer.stop();
    m_videoContainer->setInnerWidget(0);
    setControlsEnabled(false);
}

void EventVideoPlayer::playPause()
{
    if (!m_video)
        return;

    if (m_video->state() == VideoPlayerBackend::Playing)
    {
        m_video->pause();
    }
    else
    {
        if (m_video->atEnd())
            m_video->restart();
        m_video->play();
    }
}

void EventVideoPlayer::restart()
{
    if (!m_video)
        return;

    m_video->restart();
    m_video->play();
}

void EventVideoPlayer::seek(int position)
{
    if (!m_video)
        return;

    m_video->seek(qint64(position) * 1000000);
}

void EventVideoPlayer::queryLivePaused()
{
    if (!m_video)
        return;

    QSettings settings;
    if (m_video->videoBuffer() && m_video->videoBuffer()->isBuffering()
        && settings.value(QLatin1String("eventPlayer/pauseLive")).toBool())
        bcApp->pauseLive();
}

void EventVideoPlayer::bufferingStarted()
{
    QSettings settings;
    if (settings.value(QLatin1String("eventPlayer/pauseLive")).toBool())
        bcApp->pauseLive();
    updateBufferStatus();
}

void EventVideoPlayer::updateBufferStatus()
{
    if (!m_video)
    {
        m_statusText->clear();
        return;
    }

    int pcnt = qRound((double(m_video->videoBuffer()->bufferedSize()) / m_video->videoBuffer()->fileSize()) * 100.0);
    m_statusText->setText(tr("<b>Buffering:</b> %1%").arg(pcnt));
}

void EventVideoPlayer::bufferingStopped()
{
    bcApp->releaseLive();
    m_statusText->clear();
}

void EventVideoPlayer::stateChanged(int state)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    qDebug("state change %d", state);
    if (state == VideoPlayerBackend::Playing)
    {
        m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control-pause.png")));
        m_posTimer.start(30);
    }
    else
    {
        m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
        m_posTimer.stop();
        updatePosition();
    }
}

void EventVideoPlayer::durationChanged(qint64 nsDuration)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!m_video)
        return;

    if (nsDuration == -1)
        nsDuration = m_video->duration();

    /* Time is assumed to be nanoseconds; convert to milliseconds */
    int duration = int(nsDuration / 1000000);
    /* BUG: Shouldn't mindlessly chop to int */
    m_seekSlider->blockSignals(true);
    m_seekSlider->setMaximum(duration);
    m_seekSlider->blockSignals(false);
    updatePosition();
}

void EventVideoPlayer::updatePosition()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!m_video)
        return;

    if (!m_seekSlider->maximum())
    {
        qint64 nsDuration = m_video->duration();
        if (nsDuration && int(nsDuration / 1000000))
        {
            durationChanged(nsDuration);
            return;
        }
    }

    qint64 nsPosition = m_video->position();
    int position = int(nsPosition / 1000000);
    m_seekSlider->blockSignals(true);
    m_seekSlider->setValue(position);
    m_seekSlider->blockSignals(false);

    int secs = nsPosition / 1000000000;
    int durationSecs = m_video->duration() / 1000000000;

    m_posText->setText(QString::fromLatin1("%1:%2 / %3:%4").arg(secs / 60, 2, 10, QLatin1Char('0'))
                       .arg(secs % 60, 2, 10, QLatin1Char('0'))
                       .arg(durationSecs / 60, 2, 10, QLatin1Char('0'))
                       .arg(durationSecs % 60, 2, 10, QLatin1Char('0')));
}

void EventVideoPlayer::saveVideo(const QString &path)
{
    if (!m_video)
        return;

    if (path.isEmpty())
    {
        bool restart = false;
        if (m_video->state() == VideoPlayerBackend::Playing)
        {
            m_video->pause();
            restart = true;
        }

        QString upath = QFileDialog::getSaveFileName(this, tr("Save event video"), QString(),
                                     tr("Matroska Video (*.mkv)"));
        if (!upath.isEmpty())
            saveVideo(upath);

        if (restart)
            m_video->play();
        return;
    }

    EventVideoDownload *dl = new EventVideoDownload(bcApp->mainWindow);
    dl->setFilePath(path);
    dl->setVideoBuffer(m_video->videoBuffer());
    dl->start(bcApp->mainWindow);
}

void EventVideoPlayer::saveSnapshot(const QString &ifile)
{
    QImage frame = m_videoWidget ? m_videoWidget->currentFrame() : QImage();
    if (frame.isNull() || !m_video)
        return;

    QString file = ifile;

    if (file.isEmpty())
    {
        QString filename;
        if (m_event)
        {
            filename = QString::fromLatin1("%1 - %2.jpg").arg(m_event->uiLocation(),
                                                              m_event->date.addSecs(int(m_video->position() / 1000000000))
                                                              .toString(QLatin1String("yyyy-MM-dd hh-mm-ss")));
        }

        file = getSaveFileNameExt(this, tr("Save Video Snapshot"),
                           QDesktopServices::storageLocation(QDesktopServices::PicturesLocation),
                           QLatin1String("ui/snapshotSaveLocation"),
                           filename, tr("Image (*.jpg)"));

        if (file.isEmpty())
            return;
    }

    if (!frame.save(file, "jpeg"))
    {
        QMessageBox::critical(this, tr("Snapshot Error"), tr("An error occurred while saving the video snapshot."),
                              QMessageBox::Ok);
        return;
    }

    QToolTip::showText(m_videoWidget->mapToGlobal(QPoint(0,0)), tr("Snapshot Saved"), this);
}

void EventVideoPlayer::videoContextMenu(const QPoint &rpos)
{
    QPoint pos = rpos;
    if (qobject_cast<QWidget*>(sender()))
        pos = static_cast<QWidget*>(sender())->mapToGlobal(pos);

    QMenu menu(qobject_cast<QWidget*>(sender()));

    if (m_video && m_video->state() == VideoPlayerBackend::Playing)
        menu.addAction(tr("&Pause"), this, SLOT(playPause()));
    else
        menu.addAction(tr("&Play"), this, SLOT(playPause()));

    menu.addAction(tr("&Restart"), this, SLOT(restart()));

    menu.addSeparator();

    if (m_videoWidget->isFullScreen())
        menu.addAction(tr("Exit &full screen"), m_videoContainer, SLOT(toggleFullScreen()));
    else
        menu.addAction(tr("&Full screen"), m_videoContainer, SLOT(toggleFullScreen()));

    menu.addSeparator();

    menu.addAction(tr("Save video"), this, SLOT(saveVideo()));
    menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()));

    menu.exec(pos);
}

void EventVideoPlayer::setControlsEnabled(bool enabled)
{
    m_playBtn->setEnabled(enabled);
    m_restartBtn->setEnabled(enabled);
    m_saveBtn->setEnabled(enabled);
}
