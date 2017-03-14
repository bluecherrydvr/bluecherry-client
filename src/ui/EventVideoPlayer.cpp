/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EventVideoPlayer.h"
#include "event/EventDownloadManager.h"
#include "event/EventVideoDownload.h"
#include "video/VideoHttpBuffer.h"
#include "video/VideoPlayerBackend.h"
#include "video/VideoPlayerFactory.h"
#include "video/VideoWidget.h"
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
#include <QMouseEvent>
#include <math.h>

/* Hack to disable the "page step" behavior for left click on non-Mac. For seeking, especially in
 * incomplete downloads, it's much better to seek right to the position that was clicked on. */
class CustomSlider : public QSlider
{
public:
    CustomSlider(Qt::Orientation orientation, QWidget *parent = 0)
        : QSlider(orientation, parent)
    {
    }

protected:
    virtual void mousePressEvent(QMouseEvent *ev)
    {
        if (ev->button() == Qt::LeftButton && ev->button() & style()->styleHint(QStyle::SH_Slider_PageSetButtons))
        {
            Qt::MouseButton btn = static_cast<Qt::MouseButton>(style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons));
            QMouseEvent fake(ev->type(), ev->pos(), ev->globalPos(), btn, btn, ev->modifiers());
            QSlider::mousePressEvent(&fake);
            if (fake.isAccepted())
                ev->accept();
        }
        else
            QSlider::mousePressEvent(ev);
    }
};

EventVideoPlayer::EventVideoPlayer(QWidget *parent)
    : QWidget(parent), m_event(0),
      m_videoWidget(0), m_zoomFactor(1.0)
{
    connect(bcApp, SIGNAL(queryLivePaused()), SLOT(queryLivePaused()));
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(settingsChanged()));
    connect(&m_uiTimer, SIGNAL(timeout()), SLOT(updateUI()));

    m_uiTimer.setInterval(333);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_videoWidget = bcApp->videoPlayerFactory()->createWidget();
    m_videoWidget->setFrameStyle(QFrame::NoFrame);
    m_videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_videoWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(videoContextMenu(QPoint)));
    layout->addWidget(m_videoWidget, 1);

    QBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin), 0,
                                       style()->pixelMetric(QStyle::PM_LayoutRightMargin), 0);
    layout->addLayout(controlsLayout);

    QBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->setMargin(0);
    controlsLayout->addLayout(sliderLayout);

    m_startTime = new QLabel;
    sliderLayout->addWidget(m_startTime);

    m_seekSlider = new CustomSlider(Qt::Horizontal);
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

    QSettings settings;

    if (settings.value(QLatin1String("ui/disableScreensaver/onVideo")).toBool())
    {
        bcApp->setScreensaverInhibited(true);
    }

    m_muteBtn = new QToolButton;
    m_muteBtn->setCheckable(true);
    m_muteBtn->setChecked(settings.value(QLatin1String("eventPlayer/isMuted"), false).toBool());
    m_muteBtn->setIcon(m_muteBtn->isChecked() ? style()->standardIcon(QStyle::SP_MediaVolumeMuted) : style()->standardIcon(QStyle::SP_MediaVolume));

    btnLayout->addWidget(m_muteBtn);
    connect(m_muteBtn, SIGNAL(clicked()), SLOT(mute()));

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setTickInterval(2);
    m_volumeSlider->setTickPosition(QSlider::TicksBelow);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(10);
    m_volumeSlider->setValue(settings.value(QLatin1String("eventPlayer/volume"), 10).toInt());
    connect(m_volumeSlider, SIGNAL(sliderMoved(int)), SLOT(setVolume(int)));
    btnLayout->addWidget(m_volumeSlider);

    btnLayout->addSpacing(13);

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

    m_zoomOutBtn = new QPushButton;
    btnLayout->addWidget(m_zoomOutBtn);
    connect(m_zoomOutBtn, SIGNAL(clicked()), SLOT(zoomOut()));

    m_zoomText = new QLabel(tr("zoom 1x"));
    m_zoomText->setStyleSheet(QLatin1String("color: #777777"));
    m_zoomText->setFixedWidth(QFontMetrics(m_rateText->font()).width(QLatin1String("zoom 12.99x")));
    m_zoomText->setAlignment(Qt::AlignCenter);
    btnLayout->addWidget(m_zoomText);

    m_zoomInBtn = new QPushButton;
    btnLayout->addWidget(m_zoomInBtn);
    connect(m_zoomInBtn, SIGNAL(clicked()), SLOT(zoomIn()));

    btnLayout->addStretch();

    m_statusText = new QLabel;
    m_statusText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    btnLayout->addWidget(m_statusText);

    btnLayout->addStretch();

	m_saveBtn = new QPushButton;
    btnLayout->addWidget(m_saveBtn);
    connect(m_saveBtn, SIGNAL(clicked()), SLOT(saveVideo()));

    QShortcut *sc = new QShortcut(QKeySequence(Qt::Key_Space), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(playPause()));

    sc = new QShortcut(QKeySequence(Qt::Key_F), m_videoWidget);
    connect(sc, SIGNAL(activated()), m_videoWidget, SLOT(toggleFullScreen()));

    sc = new QShortcut(QKeySequence(Qt::Key_R), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(restart()));

    sc = new QShortcut(QKeySequence::Save, m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(saveVideo()));

    sc = new QShortcut(QKeySequence(Qt::Key_F5), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(saveSnapshot()));

    sc = new QShortcut(QKeySequence(Qt::Key_E), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(zoomIn()));

    sc = new QShortcut(QKeySequence(Qt::Key_W), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(zoomOut()));

    sc = new QShortcut(QKeySequence(Qt::Key_Left + Qt::ALT), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(moveRight()));

    sc = new QShortcut(QKeySequence(Qt::Key_Right + Qt::ALT), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(moveLeft()));

    sc = new QShortcut(QKeySequence(Qt::Key_Up + Qt::ALT), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(moveDown()));

    sc = new QShortcut(QKeySequence(Qt::Key_Down + Qt::ALT), m_videoWidget);
    connect(sc, SIGNAL(activated()), SLOT(moveUp()));

    setControlsEnabled(false);

    m_lastspeed = 1.0;

    retranslateUI();
}

EventVideoPlayer::~EventVideoPlayer()
{
    bcApp->disconnect(SIGNAL(queryLivePaused()), this);
    bcApp->releaseLive();

    if (m_videoBackend)
    {
        m_videoBackend.data()->clear();

#ifdef Q_OS_MAC
        qDebug() << "deleting videoBackend first, before VideoWidget\n";
        delete m_videoBackend.data();
        m_videoBackend.clear();
#else
        m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "deleteLater", Qt::QueuedConnection);
#endif
    }

    QSettings settings;
    if (settings.value(QLatin1String("ui/disableScreensaver/onVideo")).toBool())
    {
        bcApp->setScreensaverInhibited(false);
    }
    settings.setValue(QLatin1String("eventPlayer/isMuted"), m_muteBtn->isChecked());
    settings.setValue(QLatin1String("eventPlayer/volume"), m_volumeSlider->value());

}

void EventVideoPlayer::setVideo(const QUrl &url, EventData *event)
{
    if (m_videoBackend)
        clearVideo();

    if (url.isEmpty())
        return;

    m_event = event;

    //if (!m_videoThread)
    //{
        /* Not parented to this instance, because it may live slightly beyond the window while
         * the pipeline is being destroyed. */
        //m_videoThread = new QThread;
        //m_videoThread.data()->start();
    //}

    m_videoBackend = bcApp->videoPlayerFactory()->createBackend();
    //m_videoBackend.data()->moveToThread(m_videoThread.data());
	m_videoBackend.data()->setLastSpeed(m_lastspeed);

	settingsChanged();

    connect(m_videoBackend.data(), SIGNAL(stateChanged(int,int)), SLOT(stateChanged(int)));
    connect(m_videoBackend.data(), SIGNAL(nonFatalError(QString)), SLOT(videoNonFatalError(QString)));
    connect(m_videoBackend.data(), SIGNAL(durationChanged(int)), SLOT(durationChanged(int)));
    connect(m_videoBackend.data(), SIGNAL(endOfStream()), SLOT(durationChanged()));
    connect(m_videoBackend.data(), SIGNAL(playbackSpeedChanged(double)), SLOT(playbackSpeedChanged(double)));
    connect(m_videoBackend.data(), SIGNAL(streamsInitialized(bool)), SLOT(streamsInitialized(bool)));
    connect(m_videoBackend.data(), SIGNAL(currentPosition(double))
            ,this, SLOT(updateSliderPosition(double)));

    m_videoWidget->initVideo(m_videoBackend.data());

    //connect(m_videoBackend.data(), SIGNAL(bufferingStatus(int)), m_videoWidget, SLOT(setBufferStatus(int)));
    connect(m_videoBackend.data(), SIGNAL(bufferingStopped()), SLOT(bufferingStopped()), Qt::QueuedConnection);
    connect(m_videoBackend.data(), SIGNAL(bufferingStarted()), SLOT(bufferingStarted()));

    bool ok = m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "start", Qt::QueuedConnection, Q_ARG(QUrl, url));
    Q_ASSERT(ok);
    Q_UNUSED(ok);

    setControlsEnabled(true);
    QDateTime evd = event->localStartDate();
    m_startTime->setText(evd.time().toString());
    if (event->hasDuration())
        m_endTime->setText(event->localEndDate().time().toString());
    else
        m_endTime->clear();
}

void EventVideoPlayer::clearVideo()
{
    if (m_videoBackend)
    {
        m_videoBackend.data()->disconnect(this);
        bool ok = m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "clear", Qt::QueuedConnection);
        ok &= m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "deleteLater", Qt::QueuedConnection);
        Q_ASSERT(ok);
        Q_UNUSED(ok);
    }

    m_videoBackend.clear();
    m_event = 0;

    m_playBtn->setIcon(QIcon(QLatin1String(":/icons/control.png")));
    m_seekSlider->setRange(0, 0);
    m_startTime->clear();
    m_endTime->clear();
    m_statusText->clear();
    m_rateText->clear();
    m_uiTimer.stop();
    m_videoWidget->clearVideo();
    setControlsEnabled(false);
}

void EventVideoPlayer::playPause()
{
    if (!m_videoBackend)
        return;

    if (m_videoBackend.data()->state() == VideoPlayerBackend::Playing)
        m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "pause", Qt::QueuedConnection);
    else if (m_videoBackend.data()->atEnd())
        restart();
    else
        m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "play", Qt::QueuedConnection);
}

void EventVideoPlayer::restart()
{
    if (!m_videoBackend)
        return;

    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "restart", Qt::QueuedConnection);
    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "play", Qt::QueuedConnection);
}

void EventVideoPlayer::seek(int position)
{
    if (!m_videoBackend)
        return;

    bool ok = m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "seek", Qt::QueuedConnection,
                                                  Q_ARG(int, position));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}

void EventVideoPlayer::playbackSpeedChanged(double speed)
{
    if (!m_videoBackend)
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
    if (!m_videoBackend)
        return;

    float speed = m_videoBackend.data()->playbackSpeed() * 1.1f;
    for (int i = 0; i < playbackRateCount; ++i)
    {
        if (speed < playbackRates[i])
        {
            speed = playbackRates[i];
            break;
        }
    }

    speed = qBound(playbackRates[0], speed, playbackRates[playbackRateCount-1]);
    m_lastspeed = speed;
    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "setSpeed", Qt::QueuedConnection,
                                        Q_ARG(double, speed));
}

void EventVideoPlayer::slower()
{
    if (!m_videoBackend)
        return;

    float speed = m_videoBackend.data()->playbackSpeed() * 0.9f;
    for (int i = 0; i < playbackRateCount; ++i)
    {
        if (speed <= playbackRates[i])
        {
            speed = playbackRates[i-1];
            break;
        }
    }

    speed = qBound(playbackRates[0], speed, playbackRates[playbackRateCount-1]);

    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "setSpeed", Qt::QueuedConnection,
													  Q_ARG(double, speed));
}

void EventVideoPlayer::changeEvent(QEvent *event)
{
	if (event && event->type() == QEvent::LanguageChange)
		retranslateUI();

	QWidget::changeEvent(event);
}

void EventVideoPlayer::queryLivePaused()
{
    if (!m_videoBackend)
        return;

    QSettings settings;
    if (m_videoBackend.data()->videoBuffer() && m_videoBackend.data()->videoBuffer()->isBuffering()
        && settings.value(QLatin1String("eventPlayer/pauseLive")).toBool())
        bcApp->pauseLive();
}

bool EventVideoPlayer::uiRefreshNeeded() const
{
	return m_videoBackend && (m_videoBackend.data()->videoBuffer()) && (m_videoBackend.data()->videoBuffer()->isBuffering() || m_videoBackend.data()->state() == VideoPlayerBackend::Playing);
}

void EventVideoPlayer::retranslateUI()
{
	m_saveBtn->setText(tr("Save"));
    m_zoomInBtn->setText(tr("+"));
    m_zoomOutBtn->setText(tr("-"));
	updateBufferStatus();
}

void EventVideoPlayer::updateUI()
{
    updatePosition();
    updateBufferStatus();
}

void EventVideoPlayer::settingsChanged()
{
    if (!m_videoBackend)
        return;

    QSettings settings;
    m_videoBackend.data()->setHardwareDecodingEnabled(settings.value(QLatin1String("ui/eventplayer/enableHardwareDecoding"), false).toBool());
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
    if (!m_videoBackend || !m_videoBackend.data()->videoBuffer() || m_videoBackend.data()->videoBuffer()->isBufferingFinished())
        return;

    int pcnt = m_videoBackend.data()->videoBuffer()->bufferedPercent();
    m_statusText->setText(tr("<b>Downloading:</b> %1%").arg(pcnt));
}

void EventVideoPlayer::bufferingStopped()
{
    bcApp->releaseLive();

    if (!m_videoBackend || !m_videoBackend.data()->videoBuffer() || (m_videoBackend.data()->videoBuffer()->isBufferingFinished() && m_videoBackend.data()->state() > VideoPlayerBackend::Error))
        m_statusText->clear();

    if (!uiRefreshNeeded())
        m_uiTimer.stop();
}

void EventVideoPlayer::mute()
{
    if (!m_videoBackend)
        return;

    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "mute", Qt::QueuedConnection, Q_ARG(bool, m_muteBtn->isChecked()));

    m_muteBtn->setIcon(m_muteBtn->isChecked() ? style()->standardIcon(QStyle::SP_MediaVolumeMuted) : style()->standardIcon(QStyle::SP_MediaVolume));
}

void EventVideoPlayer::setVolume(int volume)
{
    if (!m_videoBackend)
        return;

    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "setVolume", Qt::QueuedConnection, Q_ARG(double, volume/10.0));
    m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "mute", Qt::QueuedConnection, Q_ARG(bool, false));

    m_muteBtn->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_muteBtn->setChecked(false);
}

void EventVideoPlayer::videoNonFatalError(const QString &message)
{
    if (message.isEmpty())
        return;

    m_statusText->setText(QLatin1String("<span style='color:red;font-weight:bold'>") + message +
                          QLatin1String("</span>"));
}

void EventVideoPlayer::streamsInitialized(bool hasAudioSupport)
{
    if (hasAudioSupport)
    {
        m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "setVolume", Qt::QueuedConnection, Q_ARG(double, m_volumeSlider->value()/10.0));
        m_videoBackend.data()->metaObject()->invokeMethod(m_videoBackend.data(), "mute", Qt::QueuedConnection, Q_ARG(bool, m_muteBtn->isChecked()));
    }

    m_volumeSlider->setEnabled(hasAudioSupport);
    m_muteBtn->setEnabled(hasAudioSupport);
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

    if (state == VideoPlayerBackend::Error || state == VideoPlayerBackend::PermanentError)
    {
        m_statusText->setText(QLatin1String("<span style='color:red;font-weight:bold'>") +
                              m_videoBackend.data()->errorMessage() + QLatin1String("</span>"));
    }

}

void EventVideoPlayer::durationChanged(int msDuration)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!m_videoBackend)
        return;

    if (msDuration == -1)
        //return;
        msDuration = m_videoBackend.data()->duration();

    m_seekSlider->blockSignals(true);
    m_seekSlider->setMaximum(msDuration);
    m_seekSlider->blockSignals(false);
    updatePosition();
}

void EventVideoPlayer::updatePosition()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!m_videoBackend)
        return;

    if (!m_seekSlider->maximum())
    {
        int msDuration = m_videoBackend.data()->duration();
        if (msDuration)
        {
            durationChanged(msDuration);
            return;
        }
    }

    m_videoBackend.data()->queryPosition();
}

void EventVideoPlayer::updateSliderPosition(double position)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!m_videoBackend)
    {
        return;
    }

    if ( m_seekSlider->maximum() == 0)
    {
        return;
    }

    int msPosition = static_cast<int>(position > 0 ? position * 1000.0 : -1);

    if (m_videoBackend.data()->atEnd())
    {
        msPosition = m_seekSlider->maximum();
    }

    if (!m_seekSlider->isSliderDown())
    {
        m_seekSlider->blockSignals(true);
        m_seekSlider->setValue(msPosition);
        m_seekSlider->blockSignals(false);
    }
}

void EventVideoPlayer::saveVideo()
{
    bcApp->eventDownloadManager()->startEventDownload(*m_event);
}
/*
void EventVideoPlayer::setZoom(double z)
{
    m_zoomFactor = z;

    if (m_videoWidget)
    {
        int x, y, w, h;

        x = m_videoWidget->x();
        y = m_videoWidget->y();
        w = m_videoWidget->width();
        h = m_videoWidget->height();

        if (m_zoomFactor != 1.0)
        {
            w = w * m_zoomFactor;
            h = h * m_zoomFactor;

            x = (this->width() - w) / 2;
            y = (this->height() -h) / 2;
        }

        m_videoWidget->move(x, y);
        m_videoWidget->resize(w, h);
    }
}*/

void EventVideoPlayer::zoomIn()
{
    if (m_videoWidget)
    {
        m_videoWidget->zoomIn();
        m_zoomText->setText(QString::fromLatin1("zoom %L1x").arg(m_videoWidget->zoom(), 0, 'f', 2));
    }
}

void EventVideoPlayer::zoomOut()
{
    if (m_videoWidget)
    {
        m_videoWidget->zoomOut();
        m_zoomText->setText(QString::fromLatin1("zoom %L1x").arg(m_videoWidget->zoom(), 0, 'f', 2));
    }
}

void EventVideoPlayer::moveLeft()
{
    if (m_videoWidget)
        m_videoWidget->moveFrame(-10, 0);
}

void EventVideoPlayer::moveRight()
{
    if (m_videoWidget)
        m_videoWidget->moveFrame(10, 0);
}

void EventVideoPlayer::moveUp()
{
    if (m_videoWidget)
        m_videoWidget->moveFrame(0, -10);
}

void EventVideoPlayer::moveDown()
{
    if (m_videoWidget)
        m_videoWidget->moveFrame(0, 10);
}

void EventVideoPlayer::saveSnapshot(const QString &ifile)
{
    QString file = ifile;

    if (file.isEmpty())
    {
        QString filename;
        if (m_event)
        {
            filename = QString::fromLatin1("%1 - %2.png").arg(m_event->uiLocation(),
                                                              m_event->localStartDate().addSecs(int(m_videoBackend.data()->position() / 1000000000))
                                                              .toString(QLatin1String("yyyy-MM-dd hh-mm-ss")));
        }

        file = getSaveFileNameExt(this, tr("Save Video Snapshot"),
                           QDesktopServices::storageLocation(QDesktopServices::PicturesLocation),
                           QLatin1String("ui/snapshotSaveLocation"),
                           filename, tr("Image (*.png)"));

        if (file.isEmpty())
            return;
        if (!file.endsWith(QLatin1String(".png"), Qt::CaseInsensitive))
            file.append(QLatin1String(".png"));
    }

    if (!m_videoBackend.data()->saveScreenshot(file))
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

    if (!m_videoBackend || !m_videoWidget)
        return;

    QMenu menu(qobject_cast<QWidget*>(sender()));

    if (m_videoBackend.data()->state() == VideoPlayerBackend::Playing)
        menu.addAction(tr("&Pause"), this, SLOT(playPause()));
    else
        menu.addAction(tr("&Play"), this, SLOT(playPause()));

    menu.addAction(tr("&Restart"), this, SLOT(restart()));

    menu.addSeparator();

    if (m_videoWidget->isFullScreen())
        menu.addAction(tr("Exit &full screen"), m_videoWidget, SLOT(toggleFullScreen()));
    else
        menu.addAction(tr("&Full screen"), m_videoWidget, SLOT(toggleFullScreen()));

    menu.addSeparator();

    menu.addAction(tr("Save video"), this, SLOT(saveVideo()));
    menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()));

    menu.addSeparator();

    menu.addAction(tr("Zoom In"), this, SLOT(zoomIn()), Qt::Key_E);
    menu.addAction(tr("Zoom Out"), this, SLOT(zoomOut()), Qt::Key_W);

    menu.addAction(tr("Move Left"), this, SLOT(moveRight()), Qt::ALT + Qt::Key_Left);
    menu.addAction(tr("Move Right"), this, SLOT(moveLeft()), Qt::ALT + Qt::Key_Right);
    menu.addAction(tr("Move Up"), this, SLOT(moveDown()), Qt::ALT + Qt::Key_Up);
    menu.addAction(tr("Move Down"), this, SLOT(moveUp()), Qt::ALT + Qt::Key_Down);

    menu.exec(pos);
}

void EventVideoPlayer::setControlsEnabled(bool enabled)
{
    m_playBtn->setEnabled(enabled);
    m_restartBtn->setEnabled(enabled);
    m_saveBtn->setEnabled(enabled);
    m_zoomInBtn->setEnabled(enabled);
    m_zoomOutBtn->setEnabled(enabled);
}
