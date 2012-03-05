#include "LiveStream.h"
#include "LiveStreamWorker.h"
#include "BluecherryApp.h"
#include "LiveViewManager.h"
#include <QMutex>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>

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

void LiveStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    avformat_network_init();

    renderTimer = new AutoTimer;
    renderTimer->setInterval(1000 / renderTimerFps);
    renderTimer->setSingleShot(false);
}

LiveStream::LiveStream(const DVRCamera &c, QObject *parent)
    : QObject(parent), camera(c), thread(0), worker(0), m_frameData(0), m_state(NotConnected),
      m_autoStart(false), m_fpsUpdateCnt(0), m_fpsUpdateHits(0), m_fps(0), m_ptsBase(AV_NOPTS_VALUE)
{
    bcApp->liveView->addStream(this);
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

    if (!worker)
    {
        Q_ASSERT(!thread);
        thread = new QThread;
        worker = new LiveStreamWorker;
        worker->moveToThread(thread);
        worker->setUrl(camera.streamUrl());

        connect(thread, SIGNAL(started()), worker, SLOT(run()));
        connect(worker, SIGNAL(destroyed()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        connect(renderTimer, SIGNAL(timeout()), SLOT(updateFrame()));

        connect(worker, SIGNAL(fatalError(QString)), SLOT(fatalError(QString)));

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
        /* Worker will delete itself, which will then destroy the thread */
        worker->staticMetaObject.invokeMethod(worker, "stop");
        worker = 0;
        thread = 0;
    }

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
        m_autoStart = (m_autoStart || state() >= Connecting);
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

bool LiveStream::updateFrame()
{
    if (state() < Connecting)
        return false;

    if (++m_fpsUpdateCnt == int(1.5*renderTimerFps))
    {
        m_fps = m_fpsUpdateHits/1.5;
        m_fpsUpdateCnt = m_fpsUpdateHits = 0;
    }

    StreamFrame *sf = worker->frameHead;
    if (!sf)
        return false;

    if (sf == m_frame)
    {
        if (!(sf = sf->next))
            return false;
        qint64 rescale = av_rescale_q(m_frame->d->pts - m_ptsBase, (AVRational){1,90000}, AV_TIME_BASE_Q);
        qint64 now     = m_ptsTimer.elapsed()*1000;
        qDebug() << "current" << m_frame->d->pts << "new" << sf->d->pts << "rescale" << rescale << "now" << now << "delta" << (rescale - now);

        /* XXX test the next frame(s) too, to handle dropping */
        /* XXX we still need decode thread dropping too. */

        if (rescale - now > 16000) {
            qDebug() << "wait" << rescale - now << "for frame";
            return false;
        }

        worker->frameHead.fetchAndStoreOrdered(sf);
        m_frame->free();
        delete m_frame;
    }

    /* XXX better m_frame maintainence and oddity handling */
    m_frame = sf;
    if (m_ptsBase == AV_NOPTS_VALUE)
    {
        m_ptsBase = m_frame->d->pts;
        m_ptsTimer.restart();
    }

    AVFrame *newFrame = sf->d;

    m_fpsUpdateHits++;

    if (state() == Connecting)
        setState(Streaming);

    bool sizeChanged = (m_currentFrame.width() != newFrame->width ||
                        m_currentFrame.height() != newFrame->height);
    m_frameData = newFrame;

    m_currentFrame = QImage(m_frameData->data[0], m_frameData->width, m_frameData->height,
                            m_frameData->linesize[0], QImage::Format_RGB32);

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updated();
    return true;
}

void LiveStream::fatalError(const QString &message)
{
    m_errorMessage = message;
    setState(Error);
    QTimer::singleShot(15000, this, SLOT(start()));
}
