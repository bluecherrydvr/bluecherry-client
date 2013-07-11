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

#include "LiveStream.h"
#include "LiveStreamFrame.h"
#include "LiveStreamThread.h"
#include "core/BluecherryApp.h"
#include "core/LiveViewManager.h"
#include "core/LoggableUrl.h"
#include "live-stream/LiveStreamWorker.h"
#include <QMutex>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>
#include <QSettings>

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

QTimer *LiveStream::m_renderTimer = 0;
static const int renderTimerFps = 30;
QTimer *LiveStream::m_stateTimer = 0;

void LiveStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    avformat_network_init();

    m_renderTimer = new AutoTimer;
    m_renderTimer->setInterval(1000 / renderTimerFps);
    m_renderTimer->setSingleShot(false);

    m_stateTimer = new AutoTimer;
    m_stateTimer->setInterval(15000);
    m_stateTimer->setSingleShot(false);
}

LiveStream::LiveStream(DVRCamera *camera, QObject *parent)
    : QObject(parent), m_camera(camera), m_currentFrameMutex(QMutex::Recursive),
      m_frame(0), m_state(NotConnected),
      m_autoStart(false), m_fpsUpdateCnt(0), m_fpsUpdateHits(0),
      m_fps(0)
{
    Q_ASSERT(m_camera);
    connect(m_camera.data(), SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    m_thread = new LiveStreamThread(this);
    connect(m_thread, SIGNAL(fatalError(QString)), this, SLOT(fatalError(QString)));

    bcApp->liveView->addStream(this);
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
    connect(m_stateTimer, SIGNAL(timeout()), SLOT(checkState()));
}

LiveStream::~LiveStream()
{
    stop();
    bcApp->liveView->removeStream(this);
}

void LiveStream::setState(State newState)
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

QUrl LiveStream::url() const
{
    if (!m_camera)
        return QUrl();

    QUrl streamUrl = m_camera.data()->streamUrl();
    if (m_bandwidthMode == LiveViewManager::LowBandwidth)
        streamUrl.setPath(streamUrl.path() + QLatin1String("/mode=keyframe"));
    return streamUrl;
}

void LiveStream::setBandwidthMode(int value)
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

void LiveStream::start()
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
    m_thread->start(url());

    updateSettings();
    setState(Connecting);
}

void LiveStream::stop()
{
    disconnect(m_renderTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

    m_thread->stop();

    delete m_frame;
    m_frame = 0;

    if (state() > NotConnected)
    {
        setState(NotConnected);
        m_autoStart = false;
    }
}

void LiveStream::setOnline(bool online)
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

void LiveStream::setPaused(bool pause)
{
    if (pause == (state() == Paused) || state() < Streaming || !m_thread->worker())
        return;

    m_thread->setPaused(pause);

    if (pause)
        setState(Paused);
    else
        setState(Streaming);
    m_frameInterval.restart();
}

void LiveStream::updateFrame()
{
    if (state() < Connecting || !m_thread->isRunning())
        return;

    if (++m_fpsUpdateCnt == int(1.5*renderTimerFps))
    {
        m_fps = m_fpsUpdateHits/1.5;
        m_fpsUpdateCnt = m_fpsUpdateHits = 0;
    }

    if (!m_thread || !m_thread->worker())
        return;

    LiveStreamFrame *sf = m_thread->worker()->frameToDisplay();
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
                            sf->avFrame()->linesize[0], QImage::Format_RGB32);

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updated();
}

QImage LiveStream::currentFrame()
{
    QMutexLocker locker(&m_currentFrameMutex);
    return m_currentFrame.copy();
}

QSize LiveStream::streamSize()
{
    QMutexLocker locker(&m_currentFrameMutex);
    return m_currentFrame.size();
}

void LiveStream::fatalError(const QString &message)
{
    qDebug() << "Fatal error:" << LoggableUrl(url()) << message;

    m_errorMessage = message;
    setState(Error);
    /* stateTimer will handle reconnection */
}

void LiveStream::checkState()
{
    if (state() == Error)
        start();
}

void LiveStream::updateSettings()
{
    if (!m_thread->worker())
        return;

    QSettings settings;
    m_thread->worker()->setAutoDeinterlacing(settings.value(QLatin1String("ui/liveview/autoDeinterlace"), false).toBool());
}
