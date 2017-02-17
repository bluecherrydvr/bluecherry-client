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

#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QImage>
#include <QObject>
#include <QSize>

class LiveStream : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

    Q_PROPERTY(bool connected READ isConnected NOTIFY stateChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(int bandwidthMode READ bandwidthMode WRITE setBandwidthMode NOTIFY bandwidthModeChanged)
    Q_PROPERTY(float receivedFps READ receivedFps CONSTANT)
    Q_PROPERTY(QSize streamSize READ streamSize NOTIFY streamSizeChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString errdesc READ errorMessage CONSTANT)
    Q_PROPERTY(bool audio READ hasAudio NOTIFY audioChanged)
    Q_PROPERTY(bool audioPlaying READ isAudioEnabled NOTIFY audioChanged)
    Q_PROPERTY(bool hwVA READ hwAccelStatus NOTIFY hwAccelChanged)

public:
    enum State
    {
        Error,
        StreamOffline,
        NotConnected,
        Connecting,
        Buffering,
        Streaming,
        Paused
    };

    explicit LiveStream(QObject *parent = 0);
    
    virtual int bandwidthMode() const = 0;
    virtual bool hwAccelStatus() const = 0;

    virtual State state() const = 0;
    virtual QString errorMessage() const = 0;

    virtual QImage currentFrame() const = 0;
    virtual QSize streamSize() const = 0;

    virtual float receivedFps() const = 0;

    virtual bool isPaused() const = 0;
    virtual bool isConnected() const  = 0;

    virtual bool hasAudio() const = 0;
    virtual bool isAudioEnabled() const  = 0;
    virtual void setFrameSizeHint(int width, int height) = 0;
    virtual void ref() = 0;
    virtual void unref() = 0;

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void setPaused(bool paused) = 0;
    virtual void togglePaused() { setPaused(!isPaused()); }
    virtual void setOnline(bool online) = 0;
    virtual void setBandwidthMode(int bandwidthMode) = 0;
    virtual void enableAudio(bool enable) = 0;
    virtual void enableHWAccel(bool hwAccel) = 0;

signals:
    void stateChanged(int newState);
    void pausedChanged(bool paused);
    void bandwidthModeChanged(int mode);
    void hwAccelChanged(bool status);

    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);
    void updated();
    void audioChanged();

    
};

#endif // LIVESTREAM_H
