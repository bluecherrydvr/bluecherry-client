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

#ifndef LIVEVIEWMANAGER_H
#define LIVEVIEWMANAGER_H

#include <QObject>

class RtspStream;
class QAction;

class LiveViewManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(BandwidthMode)

    Q_PROPERTY(int bandwidthMode READ bandwidthMode WRITE setBandwidthMode NOTIFY bandwidthModeChanged)

public:
    enum BandwidthMode
    {
        FullBandwidth,
        LowBandwidth
    };

    explicit LiveViewManager(QObject *parent = 0);

    QList<RtspStream*> streams() const;

    BandwidthMode bandwidthMode() const { return m_bandwidthMode; }

    QList<QAction*> bandwidthActions(int currentMode, QObject *target, const char *slot) const;

public slots:
    void setBandwidthMode(int value);
    void setBandwidthModeFromAction();

signals:
    void bandwidthModeChanged(int value);

private:
    QList<RtspStream*> m_streams;
    BandwidthMode m_bandwidthMode;

    friend class RtspStream;
    void addStream(RtspStream *stream);
    void removeStream(RtspStream *stream);
};

#endif // LIVEVIEWMANAGER_H
