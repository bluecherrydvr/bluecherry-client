#include "LiveStream.h"
#include "LiveStreamWorker.h"
#include <QMutex>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>

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

void LiveStream::init()
{
    av_lockmgr_register(bc_av_lockmgr);
    av_register_all();
    avformat_network_init();

    renderTimer = new AutoTimer;
    renderTimer->setInterval(33);
    renderTimer->setSingleShot(false);
}

LiveStream::LiveStream(const DVRCamera &c, QObject *parent)
    : QObject(parent), camera(c), thread(0), worker(0), m_frameData(0), m_state(NotConnected), m_autoStart(false)
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

    connect(renderTimer, SIGNAL(timeout()), SLOT(updateFrame()));

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

    AVFrame *newFrame = worker->takeFrame();
    if (!newFrame)
        return false;

    if (state() == Connecting)
        setState(Streaming);

    bool sizeChanged = (m_currentFrame.width() != newFrame->width ||
                        m_currentFrame.height() != newFrame->height);
    if (m_frameData)
    {
        av_free(m_frameData->data[0]);
        av_free(m_frameData);
    }
    m_frameData = newFrame;

    m_currentFrame = QImage(m_frameData->data[0], m_frameData->width, m_frameData->height,
                            m_frameData->linesize[0], QImage::Format_RGB32);

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updated();
    return true;
}
