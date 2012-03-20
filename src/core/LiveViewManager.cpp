#include "LiveViewManager.h"
#include "LiveStream.h"
#include <QAction>

LiveViewManager::LiveViewManager(QObject *parent)
    : QObject(parent), m_bandwidthMode(FullBandwidth)
{
}

void LiveViewManager::addStream(LiveStream *stream)
{
    m_streams.append(stream);
    connect(this, SIGNAL(bandwidthModeChanged(int)), stream, SLOT(setBandwidthMode(int)));
    stream->setBandwidthMode(bandwidthMode());
}

void LiveViewManager::removeStream(LiveStream *stream)
{
    m_streams.removeOne(stream);
}

void LiveViewManager::setBandwidthMode(int value)
{
    if (m_bandwidthMode == value)
        return;

    m_bandwidthMode = (BandwidthMode)value;
    emit bandwidthModeChanged(value);
}

void LiveViewManager::setBandwidthModeFromAction()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (!a)
        return;

    setBandwidthMode(a->data().toInt());
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

QList<QAction*> LiveViewManager::bandwidthActions(int cv, QObject *target,
                                                  const char *slot) const
{
    QList<QAction*> re;
    re << createAction(tr("Full Bandwidth"), FullBandwidth, cv, target, slot)
       << createAction(tr("Low Bandwidth"), LowBandwidth, cv, target, slot);
    return re;
}
