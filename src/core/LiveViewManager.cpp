#include "LiveViewManager.h"
#include "MJpegStream.h"
#include <QAction>

LiveViewManager::LiveViewManager(QObject *parent)
    : QObject(parent), m_globalInterval(1)
{
}

void LiveViewManager::addStream(MJpegStream *stream)
{
    m_mjpegStreams.append(stream);
    connect(this, SIGNAL(globalIntervalChanged(int)), stream, SLOT(setInterval(int)));
    stream->setInterval(globalInterval());
}

void LiveViewManager::removeStream(MJpegStream *stream)
{
    m_mjpegStreams.removeOne(stream);
}

void LiveViewManager::setGlobalInterval(int interval)
{
    if (interval < 0)
        interval = 0;

    if (m_globalInterval == interval)
        return;

    m_globalInterval = interval;
    emit globalIntervalChanged(interval);
}

static QAction *createAction(const QString &text, int value, int cv, QObject *t, const char *s)
{
    QAction *a = new QAction(text, 0);
    a->setData(value);
    a->setCheckable(true);
    if (value == cv)
        a->setChecked(true);
    QObject::connect(a, SIGNAL(triggered()), t, s);
    return a;
}

QList<QAction*> LiveViewManager::fpsActions(int cv, QObject *target,
                                            const char *slot) const
{
    QList<QAction*> re;

    re << createAction(tr("Full", "FPS denominator"), 1, cv, target, slot)
       << createAction(tr("Half", "FPS denominator"), 2, cv, target, slot)
       << createAction(tr("Quarter", "FPS denominator"), 4, cv, target, slot)
       << createAction(tr("Eighth", "FPS denominator"), 8, cv, target, slot);

    QAction *separator = new QAction(0);
    separator->setSeparator(true);

    re << separator << createAction(tr("1 FPS"), 0, cv, target, slot);
    return re;
}
