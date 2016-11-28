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

#include "RtspStream.h"
#include "RtspStreamFrame.h"
#include "RtspStreamThread.h"
#include "RtspStreamWorker.h"
#include "core/BluecherryApp.h"
#include "core/LiveViewManager.h"
#include "core/LoggableUrl.h"
#include "audio/AudioPlayer.h"
#include <QMutex>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QDateTime>

extern "C" {
#   include "libavcodec/avcodec.h"
#   include "libavformat/avformat.h"
#   include "libavutil/mathematics.h"
}

static int bc_av_lockmgr(void **mutex, enum AVLockOp op)
{
    switch (op)
    {
    case AV_LOCK_CREATE:
        *mutex = (void*) new QMutex;
        return 0;
    case AV_LOCK_OBTAIN:
        static_cast<QMutex*>(*mutex)->lock();
        return 0;
    case AV_LOCK_RELEASE:
        static_cast<QMutex*>(*mutex)->unlock();
        return 0;
    case AV_LOCK_DESTROY:
        delete static_cast<QMutex*>(*mutex);
        return 0;
    }
    return 1;
}

class AutoTimer : public QTimer
{
protected:
    virtual void connectNotify(const char *signal)
    {
        if (!strcmp(signal, SIGNAL(timeout())))
            start();
    }

    virtual void disconnectNotify(const char *signal)
    {
        Q_UNUSED(signal);
        if (!receivers(SIGNAL(timeout())))
            stop();
    }
};

QTimer *RtspStream::m_renderTimer = 0;
static const int renderTimerFps = 30;
QTimer *RtspStream::m_stateTimer = 0;

void RtspStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    //av_log_set_level(AV_LOG_FATAL);
    avformat_network_init();

    m_renderTimer = new AutoTimer;
    m_renderTimer->setInterval(1000 / renderTimerFps);
    m_renderTimer->setSingleShot(false);

    m_stateTimer = new AutoTimer;
    m_stateTimer->setInterval(5000);
    m_stateTimer->setSingleShot(false);
}

RtspStream::RtspStream(DVRCamera *camera, QObject *parent)
    : LiveStream(parent), m_camera(camera), m_thread(0), m_currentFrameMutex(QMutex::Recursive),
      m_frame(0), m_state(NotConnected),
      m_autoStart(false), m_bandwidthMode(LiveViewManager::FullBandwidth), m_fpsUpdateCnt(0), m_fpsUpdateHits(0),
      m_fps(0), m_hasAudio(false), m_isAudioEnabled(false)
{
    Q_ASSERT(m_camera);
    //connect(m_camera.data(), SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    bcApp->liveView->addStream(this);
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
    connect(m_stateTimer, SIGNAL(timeout()), SLOT(checkState()));
}

RtspStream::~RtspStream()
{
    stop();
    bcApp->liveView->removeStream(this);
}

void RtspStream::setAudioFormat(enum AVSampleFormat fmt, int channelsNum, int sampleRate)
{
    m_hasAudio = true;

    qDebug() << "RtspStream::setAudioFormat channels " << channelsNum << " sample rate " << sampleRate;
    m_audioSampleFmt = fmt;
    m_audioChannels = channelsNum;
    m_audioSampleRate = sampleRate;

    if (m_isAudioEnabled)
        enableAudio(m_isAudioEnabled);

    emit audioChanged();
}

void RtspStream::enableAudio(bool enable)
{
    if (!bcApp->audioPlayer->isDeviceEnabled())
       return;

    //turn audio off on all other streams
    if (enable)
        bcApp->liveView->switchAudio(this);

    m_isAudioEnabled = enable;

    emit audioChanged();

    bcApp->audioPlayer->stop();

    if (!m_thread || !m_thread->hasWorker())
        return;

    if (enable)
    {
        bcApp->audioPlayer->setAudioFormat(m_audioSampleFmt, m_audioChannels, m_audioSampleRate);
        connect(m_thread.data(), SIGNAL(audioSamplesAvailable(void *, int, int)), bcApp->audioPlayer, SLOT(feedSamples(void *, int, int)), Qt::DirectConnection);
        bcApp->audioPlayer->play();
    }
    else
    {
        disconnect(m_thread.data(), SIGNAL(audioSamplesAvailable(void *, int, int)), 0, 0);
    }

    m_thread->enableAudio(enable);
}

void RtspStream::setState(State newState)
{
    if (m_state == newState)
        return;

    State oldState = m_state;
    m_state = newState;

    if (m_state != Error)
        m_errorMessage.clear();

    emit stateChanged(newState);

    if (newState >= Streaming && oldState < Streaming)
        emit streamRunning();
    else if (oldState >= Streaming && newState < Streaming)
        emit streamStopped();

    if (oldState == Paused || newState == Paused)
        emit pausedChanged(isPaused());
}

QUrl RtspStream::url() const
{
    if (!m_camera)
        return QUrl();

    QUrl streamUrl = m_camera.data()->rtspStreamUrl();
    if (m_bandwidthMode == LiveViewManager::LowBandwidth)
        streamUrl.setPath(streamUrl.path() + QLatin1String("/mode=keyframe"));
    return streamUrl;
}

void RtspStream::setBandwidthMode(int value)
{
    if (value == m_bandwidthMode)
        return;

    m_bandwidthMode = (LiveViewManager::BandwidthMode)value;
    emit bandwidthModeChanged(value);

    if (state() >= Connecting)
    {
        stop();
        start();
    }
}

void RtspStream::start()
{
    if (state() >= Connecting)
        return;

    if (state() == StreamOffline)
    {
        m_autoStart = true;
        return;
    }

    connect(m_renderTimer, SIGNAL(timeout()), SLOT(updateFrame()), Qt::UniqueConnection);

    m_frameInterval.start();

    if (m_thread)
        m_thread->stop();

    m_thread.reset(new RtspStreamThread());
    connect(m_thread.data(), SIGNAL(fatalError(QString)), this, SLOT(fatalError(QString)));
    connect(m_thread.data(), SIGNAL(audioFormat(enum AVSampleFormat, int, int)), this, SLOT(setAudioFormat(AVSampleFormat,int,int)), Qt::DirectConnection);
    m_thread->start(url());

    updateSettings();
    setState(Connecting);
}

void RtspStream::stop()
{
    disconnect(m_renderTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

    m_thread.reset();

    delete m_frame;
    m_frame = 0;

    if (state() > NotConnected)
    {
        setState(NotConnected);
        m_autoStart = false;
    }
}

void RtspStream::setOnline(bool online)
{
    if (!online && state() != StreamOffline)
    {
        m_autoStart = (state() >= Connecting);
        setState(StreamOffline);
        stop();
    }
    else if (online && state() == StreamOffline)
    {
        setState(NotConnected);
        if (m_autoStart)
            start();
    }
}

void RtspStream::setPaused(bool pause)
{
    if (pause == (state() == Paused) || state() < Streaming || !m_thread || !m_thread->hasWorker())
        return;

    m_thread->setPaused(pause);

    if (pause)
        setState(Paused);
    else
        setState(Streaming);
    m_frameInterval.restart();
}

void RtspStream::updateFrame()
{
    if (state() < Connecting || !m_thread || !m_thread->isRunning())
        return;

    if (++m_fpsUpdateCnt == int(1.5*renderTimerFps))
    {
        m_fps = m_fpsUpdateHits/1.5;
        m_fpsUpdateCnt = m_fpsUpdateHits = 0;
    }

    if (!m_thread || !m_thread->hasWorker())
        return;

    RtspStreamFrame *sf = m_thread->frameToDisplay();
    if (!sf) // no new frame
        return;

    delete m_frame;
    m_frame = sf;

    m_fpsUpdateHits++;

    if (state() == Connecting)
        setState(Streaming);
    m_frameInterval.restart();

    QMutexLocker locker(&m_currentFrameMutex);
    bool sizeChanged = (m_currentFrame.width() != sf->avFrame()->width ||
                        m_currentFrame.height() != sf->avFrame()->height);

    m_currentFrame = QImage(sf->avFrame()->data[0], sf->avFrame()->width, sf->avFrame()->height,
                            sf->avFrame()->linesize[0], QImage::Format_RGB32).copy();

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updated();
}

QImage RtspStream::currentFrame() const
{
    QMutexLocker locker(&m_currentFrameMutex);
    return m_currentFrame.copy();
}

QSize RtspStream::streamSize() const
{
    QMutexLocker locker(&m_currentFrameMutex);
    return m_currentFrame.size();
}

void RtspStream::fatalError(const QString &message)
{
    qDebug() << QDateTime::currentDateTime().toString(tr("yyyy-MM-dd hh:mm:ss")) << " Fatal error:" << LoggableUrl(url()) << message;

    if (m_isAudioEnabled)
        bcApp->audioPlayer->stop();

    m_errorMessage = message;
    setState(Error);
    /* stateTimer will handle reconnection */
}

void RtspStream::checkState()
{
    if (state() == Error)
        start();
}

void RtspStream::updateSettings()
{
    if (!m_thread || !m_thread->hasWorker())
        return;

    QSettings settings;
    m_thread->setAutoDeinterlacing(settings.value(QLatin1String("ui/liveview/autoDeinterlace"), false).toBool());
}
