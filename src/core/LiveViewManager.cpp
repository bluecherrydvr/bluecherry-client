/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#include "LiveViewManager.h"
#include "core/LiveStream.h"
#include <QAction>

LiveViewManager::LiveViewManager(QObject *parent)
    : QObject(parent), m_bandwidthMode(FullBandwidth)
{
}

void LiveViewManager::switchAudio(LiveStream *stream)
{
    //disable audio on all streams except passed as argument
    QList<LiveStream*>::const_iterator i;

    for (i = m_streams.constBegin(); i != m_streams.constEnd(); ++i)
        if (*i != stream)
            (*i)->enableAudio(false);
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
