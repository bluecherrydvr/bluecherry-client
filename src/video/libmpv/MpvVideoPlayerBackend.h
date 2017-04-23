/*
 * Copyright 2010-2014 Bluecherry
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

#ifndef MPV_VIDEO_PLAYER_BACKEND_H
#define MPV_VIDEO_PLAYER_BACKEND_H

#include "video/VideoPlayerBackend.h"
#include "video/libmpv/MpvVideoWidget.h"
#include "mpv/client.h"

class QString;
class VideoHttpBuffer;

class MpvVideoPlayerBackend : public VideoPlayerBackend
{
Q_OBJECT

public:
    explicit MpvVideoPlayerBackend(QObject *parent = 0);
    virtual ~MpvVideoPlayerBackend();


    virtual int duration() const;
    virtual int position() const;
    virtual void queryPosition();
    virtual double playbackSpeed() const { return m_playbackSpeed; }
    virtual bool isSeekable() const;
    virtual bool atEnd() const { return m_state == Done; }
    virtual VideoState state() const { return m_state; }
    virtual bool isError() const { return m_state <= Error; }
    virtual bool isPermanentError() const { return m_state == PermanentError; }
    virtual QString errorMessage() const { return m_errorMessage; }
    virtual VideoHttpBuffer *videoBuffer() const { return m_videoBuffer; }

    virtual bool saveScreenshot(QString &file);
    virtual void setHardwareDecodingEnabled(bool enable);
    void setWindowId(quint64 wid);
    void emitEvents();

public slots:
    virtual bool start(const QUrl &url);
    virtual void clear();

    virtual void play();
    virtual void playIfReady();
    virtual void pause();
    virtual bool seek(int position);
    virtual bool setSpeed(double speed);
    virtual void restart();
    virtual void playForward();
    virtual void playBackward();
    virtual void mute(bool mute);
    virtual void setVolume(double volume);
    virtual void setBrightness(int brighness);
    virtual void setContrast(int contrast);
    virtual void setColor(int balance);

private slots:
    void streamError(const QString &message);
    void setError(bool permanent, const QString message);
    void handleEof();
    void mpvPlayerReady();
    void durationIsKnown();
    void playDuringDownloadTimerShot();
    void checkDownloadAndPlayProgress();
    void receiveMpvEvents();

signals:
    void currentPosition(double position);
    void mpvEvents();

private:
    VideoHttpBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
    double m_playbackSpeed;

    mpv_handle *m_mpv;
    quint64 m_id;
    bool m_playDuringDownload;
    bool m_pausedBySlowDownload;
    double m_duration;
    double m_position;
    double m_seek_step;

    void setVideoBuffer(VideoHttpBuffer *videoHttpBuffer);
    bool createMpvProcess();
    void handleMpvEvent(mpv_event *event);
    void hrSeek();
};

#endif // MPV_VIDEO_PLAYER_BACKEND_H
