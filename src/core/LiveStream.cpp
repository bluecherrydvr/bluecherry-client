#include "LiveStream.h"
#include "LiveStreamWorker.h"
#include <QMutex>
#include <QMetaObject>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
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

void LiveStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    avformat_network_init();
}

LiveStream::LiveStream(const DVRCamera &c, QObject *parent)
    : QObject(parent), camera(c), thread(0), worker(0), m_state(NotConnected), m_autoStart(false)
{
}

LiveStream::~LiveStream()
{
    stop();
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

    Q_ASSERT(!thread && !worker);

    thread = new QThread;
    worker = new LiveStreamWorker;
    worker->moveToThread(thread);
    worker->setUrl(camera.streamUrl());

    connect(thread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(worker, SIGNAL(frame(QImage)), SLOT(updateFrame(QImage)));

    setState(Connecting);
    thread->start();
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

void LiveStream::updateFrame(const QImage &image)
{
    if (state() < Connecting)
        return;
    else if (state() == Connecting)
        setState(Streaming);

    bool sizeChanged = (image.size() != m_currentFrame.size());

    m_currentFrame = image;
    if (sizeChanged)
        emit streamSizeChanged(image.size());
    emit updated();
}
