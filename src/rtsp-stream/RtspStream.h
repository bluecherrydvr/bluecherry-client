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

#ifndef RTSPSTREAM_H
#define RTSPSTREAM_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QImage>
#include <QElapsedTimer>
#include "camera/DVRCamera.h"
#include "core/LiveStream.h"
#include "core/LiveViewManager.h"

class RtspStreamThread;

class RtspStream : public LiveStream
{
    Q_OBJECT

public:

    static void init();
    explicit RtspStream(DVRCamera *camera, QObject *parent = 0);
    virtual ~RtspStream();

    QUrl url() const;

    int bandwidthMode() const { return m_bandwidthMode; }

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QImage currentFrame() const;
    QSize streamSize() const;

    float receivedFps() const { return m_fps; }

    bool isPaused() const { return state() == Paused; }
    bool isConnected() const { return state() > Connecting; }

public slots:
    void start();
    void stop();

    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }
    void setOnline(bool online);
    void setBandwidthMode(int bandwidthMode);

private slots:
    void updateFrame();
    void fatalError(const QString &message);
    void updateSettings();
    void checkState();

private:
    static QTimer *m_renderTimer, *m_stateTimer;

    QWeakPointer<DVRCamera> m_camera;
    QScopedPointer<RtspStreamThread> m_thread;
    QImage m_currentFrame;
    mutable QMutex m_currentFrameMutex;
    class RtspStreamFrame *m_frame;
    QString m_errorMessage;
    State m_state;
    bool m_autoStart;
    LiveViewManager::BandwidthMode m_bandwidthMode;

    int m_fpsUpdateCnt;
    int m_fpsUpdateHits;
    float m_fps;

    QElapsedTimer m_frameInterval;

    void setState(State newState);

};

#endif // RTSPSTREAM_H
