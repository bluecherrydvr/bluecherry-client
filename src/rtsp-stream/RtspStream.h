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
#include "audio/AudioPlayer.h"

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
    bool hwAccelStatus() const { return m_isHWAccelEnabled; };

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QImage currentFrame() const;
    QSize streamSize() const;

    float receivedFps() const { return m_fps; }

    bool isPaused() const { return state() == Paused; }
    bool isConnected() const { return state() > Connecting; }
    bool hasAudio() const { return m_hasAudio; }
    bool isAudioEnabled() const { return m_isAudioEnabled; }
    void setFrameSizeHint(int width, int height);
    void ref();
    void unref();

public slots:
    void start();
    void stop();

    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }
    void setOnline(bool online);
    void setBandwidthMode(int bandwidthMode);
    void enableAudio(bool);
    void enableHWAccel(bool hwAccel);
    void setAudioFormat(enum AVSampleFormat, int, int);

private slots:
    void updateFrame();
    void fatalError(const QString &message);
    void updateSettings();
    void checkState();
    void hwAccelDisabled();
    void updateHwAccelSettings();

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
    bool m_hasAudio;
    bool m_isAudioEnabled;
    bool m_isHWAccelEnabled;

    QElapsedTimer m_frameInterval;

    enum AVSampleFormat m_audioSampleFmt;
    int m_audioChannels;
    int m_audioSampleRate;
    int m_refcount;

    void setState(State newState);

};

#endif // RTSPSTREAM_H
