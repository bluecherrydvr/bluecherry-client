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
#include <QStyle>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <math.h>

EventVideoPlayer::EventVideoPlayer(QWidget *parent)
    : QWidget(parent), m_event(0), m_videoThread(0), m_video(0), m_videoWidget(0)
{
    connect(bcApp, SIGNAL(queryLivePaused()), SLOT(queryLivePaused()));
    connect(&m_uiTimer, SIGNAL(timeout()), SLOT(updateUI()));

    m_uiTimer.setInterval(100);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_videoContainer = new VideoContainer;
    m_videoContainer->setFrameStyle(QFrame::NoFrame);
    m_videoContainer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_videoContainer, SIGNAL(customContextMenuRequested(QPoint)), SLOT(videoContextMenu(QPoint)));
    layout->addWidget(m_videoContainer, 1);

    QBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin), 0,
                                       style()->pixelMetric(QStyle::PM_LayoutRightMargin), 0);
    layout->addLayout(controlsLayout);

    QBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->setMargin(0);
    controlsLayout->addLayout(sliderLayout);

    m_startTime = new QLabel;
    sliderLayout->addWidget(m_startTime);

    m_seekSlider = new QSlider(Qt::Horizontal);
    connect(m_seekSlider, SIGNAL(valueChanged(int)), SLOT(seek(int)));
    sliderLayout->addWidget(m_seekSlider);

    m_endTime = new QLabel;
    sliderLayout->addWidget(m_endTime);

    QBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setMargin(0);
    btnLayout->setSpacing(3);
    controlsLayout->addLayout(btnLayout);

    m_restartBtn = new QToolButton;
    m_restartBtn->setIcon(QIcon(QLatin1String(":/icons/control-stop-180.png")));
    btnLayout->addWidget(m_restartBtn);
    connect(m_restartBtn, SIGNAL(clicked()), SLOT(restart()));

    m_playBtn = new QToolButton;
    m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
    btnLayout->addWidget(m_playBtn);
    connect(m_playBtn, SIGNAL(clicked()), SLOT(playPause()));

    btnLayout->addSpacing(26);

    m_slowBtn = new QToolButton;
    m_slowBtn->setIcon(QIcon(QLatin1String(":/icons/control-double-180.png")));
    btnLayout->addWidget(m_slowBtn);
    connect(m_slowBtn, SIGNAL(clicked()), SLOT(slower()));

    m_rateText = new QLabel(tr("1x"));
    m_rateText->setStyleSheet(QLatin1String("color: #777777"));
    m_rateText->setFixedWidth(QFontMetrics(m_rateText->font()).width(QLatin1String("6.66x")));
    m_rateText->setAlignment(Qt::AlignCenter);
    btnLayout->addWidget(m_rateText);

    m_fastBtn = new QToolButton;
    m_fastBtn->setIcon(QIcon(QLatin1String(":/icons/control-double.png")));
    btnLayout->addWidget(m_fastBtn);
    connect(m_fastBtn, SIGNAL(clicked()), SLOT(faster()));

    btnLayout->addStretch();

    m_statusText = new QLabel;
    m_statusText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    btnLayout->addWidget(m_statusText);

    btnLayout->addStretch();

    m_saveBtn = new QPushButton(tr("Save"));
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
        m_video->metaObject()->invokeMethod(m_video, "deleteLater", Qt::QueuedConnection);

    if (m_videoThread)
    {
        connect(m_videoThread, SIGNAL(finished()), m_videoThread, SLOT(deleteLater()));

        if (m_video)
            connect(m_video, SIGNAL(destroyed()), m_videoThread, SLOT(quit()));
        else
            m_videoThread->quit();
    }
}

void EventVideoPlayer::setVideo(const QUrl &url, EventData *event)
{
    if (m_video)
        clearVideo();

    if (url.isEmpty())
        return;

    m_event = event;

    if (!m_videoThread)
    {
        /* Not parented to this instance, because it may live slightly beyond the window while
         * the pipeline is being destroyed. */
        m_videoThread = new QThread;
        m_videoThread->start();
    }

    m_video = new VideoPlayerBackend;
    m_video->moveToThread(m_videoThread);
    connect(m_video, SIGNAL(stateChanged(int,int)), SLOT(stateChanged(int)));
    connect(m_video, SIGNAL(nonFatalError(QString)), SLOT(videoNonFatalError(QString)));
    connect(m_video, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(m_video, SIGNAL(endOfStream()), SLOT(durationChanged()));
    connect(m_video, SIGNAL(playbackSpeedChanged(double)), SLOT(playbackSpeedChanged(double)));

    m_videoWidget = new GstSinkWidget;
    m_videoContainer->setInnerWidget(m_videoWidget);
    m_video->setSink(m_videoWidget->gstElement());

    connect(m_video, SIGNAL(bufferingStatus(int)), m_videoWidget, SLOT(setBufferStatus(int)));
    connect(m_video->videoBuffer(), SIGNAL(bufferingStopped()), SLOT(bufferingStopped()), Qt::QueuedConnection);
    connect(m_video->videoBuffer(), SIGNAL(bufferingStarted()), SLOT(bufferingStarted()));

    m_video->videoBuffer()->setCookies(bcApp->nam->cookieJar()->cookiesForUrl(url));

    bool ok = m_video->metaObject()->invokeMethod(m_video, "start", Qt::QueuedConnection, Q_ARG(QUrl, url));
    Q_ASSERT(ok);
    Q_UNUSED(ok);

    setControlsEnabled(true);
    QDateTime evd = event->serverLocalDate();
    m_startTime->setText(evd.time().toString());
    if (event->duration > 0)
        m_endTime->setText(evd.addSecs(qMax(0, event->duration)).time().toString());
    else
        m_endTime->clear();
}

void EventVideoPlayer::clearVideo()
{
    if (m_video)
    {
        m_video->disconnect(this);
        bool ok = m_video->metaObject()->invokeMethod(m_video, "clear", Qt::QueuedConnection);
        ok &= m_video->metaObject()->invokeMethod(m_video, "deleteLater", Qt::QueuedConnection);
        Q_ASSERT(ok);
        Q_UNUSED(ok);
    }

    m_video = 0;
    m_event = 0;

    m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
    m_seekSlider->setRange(0, 0);
    m_startTime->clear();
    m_endTime->clear();
    m_statusText->clear();
    m_rateText->clear();
    m_uiTimer.stop();
    m_videoContainer->setInnerWidget(0);
    setControlsEnabled(false);
}

void EventVideoPlayer::playPause()
{
    if (!m_video)
        return;

    if (m_video->state() == VideoPlayerBackend::Playing)
    {
        m_video->metaObject()->invokeMethod(m_video, "pause", Qt::QueuedConnection);
    }
    else
    {
        if (m_video->atEnd())
            restart();
        else
            m_video->metaObject()->invokeMethod(m_video, "play", Qt::QueuedConnection);
    }
}

void EventVideoPlayer::restart()
{
    if (!m_video)
        return;

    m_video->metaObject()->invokeMethod(m_video, "restart", Qt::QueuedConnection);
    m_video->metaObject()->invokeMethod(m_video, "play", Qt::QueuedConnection);
}

void EventVideoPlayer::seek(int position)
{
    if (!m_video)
        return;

    bool ok = m_video->metaObject()->invokeMethod(m_video, "seek", Qt::QueuedConnection,
                                                  Q_ARG(qint64, qint64(position) * 1000000));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}

void EventVideoPlayer::playbackSpeedChanged(double speed)
{
    if (!m_video)
        return;

    int prc = (speed - floor(speed) >= 0.005) ? 2 : 0;
    m_rateText->setText(QString::fromLatin1("%L1x").arg(speed, 0, 'f', prc));
}

static const float playbackRates[] = {
    1.0/128, 1.0/64, 1.0/32, 1.0/16, 1.0/8, 1.0/4, 1.0/3, 1.0/2, 2.0/3,
    1.0/1,
    3.0/2, 2.0/1, 3.0/1, 4.0/1, 8.0/1, 16.0/1, 32.0/1, 64.0/1, 128.0/1
};
static const int playbackRateCount = 19;

void EventVideoPlayer::faster()
{
    if (!m_video)
        return;

    float speed = m_video->playbackSpeed() * 1.1f;
    for (int i = 0; i < playbackRateCount; ++i)
    {
        if (speed < playbackRates[i])
        {
            speed = playbackRates[i];
            break;
        }
    }

    speed = qBound(playbackRates[0], speed, playbackRates[playbackRateCount-1]);

    m_video->metaObject()->invokeMethod(m_video, "setSpeed", Qt::QueuedConnection,
                                        Q_ARG(double, speed));
}

void EventVideoPlayer::slower()
{
    if (!m_video)
        return;

    float speed = m_video->playbackSpeed() * 0.9f;
    for (int i = 0; i < playbackRateCount; ++i)
    {
        if (speed <= playbackRates[i])
        {
            speed = playbackRates[i-1];
            break;
        }
    }

    speed = qBound(playbackRates[0], speed, playbackRates[playbackRateCount-1]);

    m_video->metaObject()->invokeMethod(m_video, "setSpeed", Qt::QueuedConnection,
                                        Q_ARG(double, speed));
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

bool EventVideoPlayer::uiRefreshNeeded() const
{
    return m_video && (m_video->videoBuffer()->isBuffering() || m_video->state() == VideoPlayerBackend::Playing);
}

void EventVideoPlayer::updateUI()
{
    updatePosition();
    updateBufferStatus();
}

void EventVideoPlayer::bufferingStarted()
{
    QSettings settings;
    if (settings.value(QLatin1String("eventPlayer/pauseLive")).toBool())
        bcApp->pauseLive();
    m_uiTimer.start();
    updateBufferStatus();
}

void EventVideoPlayer::updateBufferStatus()
{
    if (!m_video || m_video->videoBuffer()->isBufferingFinished())
        return;

    int pcnt = m_video->videoBuffer()->bufferedPercent();
    m_statusText->setText(tr("<b>Downloading:</b> %1%").arg(pcnt));
}

void EventVideoPlayer::bufferingStopped()
{
    qDebug("bufferingstopped");
    bcApp->releaseLive();

    if (!m_video || m_video->videoBuffer()->isBufferingFinished())
        m_statusText->clear();

    if (!uiRefreshNeeded())
        m_uiTimer.stop();
}

void EventVideoPlayer::videoNonFatalError(const QString &message)
{
    if (message.isEmpty())
        return;

    m_statusText->setText(QLatin1String("<span style='color:red;font-weight:bold'>") + message +
                          QLatin1String("</span>"));
}

void EventVideoPlayer::stateChanged(int state)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    qDebug("state change %d", state);
    if (state == VideoPlayerBackend::Playing)
    {
        m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control-pause.png")));
        m_uiTimer.start();
        updatePosition();
    }
    else
    {
        m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
        updatePosition();
        if (!uiRefreshNeeded())
            m_uiTimer.stop();
    }

    QSettings settings;
    if (settings.value(QLatin1String("ui/disableScreensaver/onVideo")).toBool())
    {
        bcApp->setScreensaverInhibited(state == VideoPlayerBackend::Playing
                                       || state == VideoPlayerBackend::Paused);
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
    if (!m_seekSlider->isSliderDown())
    {
        m_seekSlider->blockSignals(true);
        m_seekSlider->setValue(position);
        m_seekSlider->blockSignals(false);
    }
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
    dl->setMediaDownload(m_video->videoBuffer()->mediaDownload());
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

    if (!m_video || !m_videoWidget)
        return;

    QMenu menu(qobject_cast<QWidget*>(sender()));

    if (m_video->state() == VideoPlayerBackend::Playing)
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
