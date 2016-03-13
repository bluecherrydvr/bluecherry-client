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

#ifndef MPL_VIDEO_PLAYER_BACKEND_H
#define MPL_VIDEO_PLAYER_BACKEND_H

#include "video/VideoPlayerBackend.h"
#include "MplayerProcess.h"


class QString;
class VideoHttpBuffer;


class MplVideoPlayerBackend : public VideoPlayerBackend
{
Q_OBJECT

public:
    explicit MplVideoPlayerBackend(QObject *parent = 0);
    virtual ~MplVideoPlayerBackend();



    virtual int duration() const;
    virtual int position() const;
    virtual void queryPosition() const;
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

public slots:
    virtual bool start(const QUrl &url);
    virtual void clear();

    virtual void play();
    virtual void playIfReady();
    virtual void pause();
    virtual bool seek(int position);
    virtual bool setSpeed(double speed);
    virtual void restart();
    virtual void mute(bool mute);
    virtual void setVolume(double volume);

private slots:
    void streamError(const QString &message);
    void setError(bool permanent, const QString message);
    void handleEof();
    void mplayerReady();
    void durationIsKnown();

signals:
    void currentPosition(double position);


private:
    VideoHttpBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
    double m_playbackSpeed;

    MplayerProcess *m_mplayer;
    //Window id for mplayer process, argument for -wid option
    QString m_wid;

    void setVideoBuffer(VideoHttpBuffer *videoHttpBuffer);
    void createMplayerProcess();
};

#endif // MPL_VIDEO_PLAYER_BACKEND_H
