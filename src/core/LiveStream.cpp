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
#include "LiveStreamWorker.h"
#include "BluecherryApp.h"
#include "LiveViewManager.h"
#include <QMutex>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>
#include <QSettings>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
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

QTimer *LiveStream::renderTimer = 0;
static const int renderTimerFps = 30;
QTimer *LiveStream::stateTimer = 0;

void LiveStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    avformat_network_init();

    renderTimer = new AutoTimer;
    renderTimer->setInterval(1000 / renderTimerFps);
    renderTimer->setSingleShot(false);

    stateTimer = new AutoTimer;
    stateTimer->setInterval(15000);
    stateTimer->setSingleShot(false);
}

LiveStream::LiveStream(const DVRCamera &c, QObject *parent)
    : QObject(parent), camera(c), thread(0), worker(0), m_frame(0), m_state(NotConnected),
      m_autoStart(false), m_fpsUpdateCnt(0), m_fpsUpdateHits(0),
      m_fps(0), m_ptsBase(AV_NOPTS_VALUE)
{
    bcApp->liveView->addStream(this);
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
    connect(stateTimer, SIGNAL(timeout()), SLOT(checkState()));
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

QByteArray LiveStream::url() const
{
    QByteArray re = camera.streamUrl();
    if (m_bandwidthMode == LiveViewManager::LowBandwidth)
        re.append("/mode=keyframe");
    return re;
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

    m_frameInterval.start();

    if (!worker)
    {
        Q_ASSERT(!thread);
        thread = new QThread;
        worker = new LiveStreamWorker;
        worker->moveToThread(thread);
        worker->setUrl(url());

        connect(thread, SIGNAL(started()), worker, SLOT(run()));
        connect(worker, SIGNAL(destroyed()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        connect(renderTimer, SIGNAL(timeout()), SLOT(updateFrame()));

        connect(worker, SIGNAL(fatalError(QString)), SLOT(fatalError(QString)));

        updateSettings();
        setState(Connecting);
        thread->start();
    }
    else
    {
        setState(Connecting);
        worker->metaObject()->invokeMethod(worker, "run");
    }
}

void LiveStream::stop()
{
    if (worker)
    {
        /* See LiveStreamWorker's destructor for how this frame is freed */
        m_frame = 0;
        /* Worker will delete itself, which will then destroy the thread */
        worker->staticMetaObject.invokeMethod(worker, "stop");
        worker = 0;
        thread = 0;
    }

    Q_ASSERT(!m_frame);
    Q_ASSERT(!thread);

    disconnect(renderTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

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
    if (pause == (state() == Paused) || state() < Streaming || !worker)
        return;

    worker->metaObject()->invokeMethod(worker, "setPaused", Q_ARG(bool, pause));
    if (pause)
        setState(Paused);
    else
        setState(Streaming);
    m_frameInterval.restart();
}

bool LiveStream::updateFrame()
{
    if (state() < Connecting)
        return false;

    if (++m_fpsUpdateCnt == int(1.5*renderTimerFps))
    {
        m_fps = m_fpsUpdateHits/1.5;
        m_fpsUpdateCnt = m_fpsUpdateHits = 0;
    }

    QMutexLocker l(&worker->frameLock);
    StreamFrame *sf = worker->frameHead;
    if (!sf)
        return false;

    if (sf == m_frame)
    {
        qint64 now = m_ptsTimer.elapsed()*1000;
        StreamFrame *next;
        /* Cannot use the (AVRational){1,90000} syntax due to MSVC */
        AVRational r = {1, 90000}, tb = {1, AV_TIME_BASE};

        while ((next = sf->next))
        {
            qint64 rescale = av_rescale_q(next->d->pts - m_ptsBase, r, tb);
            if (abs(rescale - now) >= AV_TIME_BASE/2)
            {
                m_ptsBase = next->d->pts;
                m_ptsTimer.restart();
                now = rescale = 0;
            }

            if (now >= rescale || (rescale - now) <= AV_TIME_BASE/(renderTimerFps*2))
            {
                /* Target rendering time is in the past, or is less than half a repaint interval in
                 * the future, so it's time to draw this frame. */
                delete sf;
                sf = next;
            }
            else
                break;
        }

        if (sf == m_frame)
            return false;
        worker->frameHead = sf;
    }
    else if (m_frame)
        delete m_frame;

    l.unlock();

    m_frame = sf;
    if (m_ptsBase == (int64_t)AV_NOPTS_VALUE)
    {
        m_ptsBase = m_frame->d->pts;
        m_ptsTimer.restart();
    }

    m_fpsUpdateHits++;

    if (state() == Connecting)
        setState(Streaming);
    m_frameInterval.restart();

    bool sizeChanged = (m_currentFrame.width() != sf->d->width ||
                        m_currentFrame.height() != sf->d->height);

    m_currentFrame = QImage(sf->d->data[0], sf->d->width, sf->d->height,
                            sf->d->linesize[0], QImage::Format_RGB32);

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updated();
    return true;
}

void LiveStream::fatalError(const QString &message)
{
    m_errorMessage = message;
    setState(Error);
    /* stateTimer will handle reconnection */
}

void LiveStream::checkState()
{
    if (state() == Error)
        start();

    if ((state() == Streaming || state() == Connecting) &&
        m_frameInterval.elapsed() >= 10000)
    {
        fatalError(QLatin1String("Stream timeout"));
        stop();
    }
}

void LiveStream::updateSettings()
{
    if (!worker)
        return;

    QSettings settings;
    worker->setAutoDeinterlacing(settings.value(QLatin1String("ui/liveview/autoDeinterlace"), false).toBool());
}
