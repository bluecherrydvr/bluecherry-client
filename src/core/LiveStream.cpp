#include "LiveStream.h"
#include "LiveStreamWorker.h"
#include <QMutex>

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
    : QObject(parent), camera(c)
{
    thread = new QThread(this);
    worker = new LiveStreamWorker;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(run()));
    connect(worker, SIGNAL(frame(QImage)), SLOT(updateFrame(QImage)));
    thread->start();
}

void LiveStream::updateFrame(const QImage &image)
{
    m_currentFrame = image;
    emit updated();
}
