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

#ifndef VIDEO_PLAYER_BACKEND_H
#define VIDEO_PLAYER_BACKEND_H

#include <QObject>
#include <QUrl>

class VideoBuffer;

class VideoPlayerBackend : public QObject
{
    Q_OBJECT

public:
    enum VideoState
    {
        PermanentError = -2, /* Permanent errors; i.e., playback will not work even if restarted */
        Error = -1, /* Recoverable errors, generally by stopping and restarting the pipeline */
        Stopped,
        Playing,
        Paused,
        Done
    };

    explicit VideoPlayerBackend(QObject *parent = 0);
    virtual ~VideoPlayerBackend();

    virtual qint64 duration() const = 0;
    virtual qint64 position() const = 0;
    virtual double playbackSpeed() const = 0;
    virtual bool isSeekable() const = 0;
    virtual bool atEnd() const = 0;
    virtual VideoState state() const = 0;
    virtual bool isError() const = 0;
    virtual bool isPermanentError() const = 0;
    virtual QString errorMessage() const = 0;
    virtual VideoBuffer * videoBuffer() const = 0;

public slots:
    virtual bool start(const QUrl &url) = 0;
    virtual void clear() = 0;

    virtual void play() = 0;
    virtual void playIfReady() = 0;
    virtual void pause() = 0;
    virtual bool seek(qint64 position) = 0;
    virtual bool setSpeed(double speed) = 0;
    virtual void restart() = 0;

signals:
    void stateChanged(int newState, int oldState);
    void durationChanged(qint64 duration);
    void playbackSpeedChanged(double playbackSpeed);
    void endOfStream();
    /* This reports the status of buffering enough for playback, not the buffering of the entire file.
     * The size of this buffer depends on how it's configured in decodebin2, currently 10 seconds of
     * playback. */
    void bufferingStatus(int percent);
    void bufferingStarted();
    void bufferingStopped();
    void nonFatalError(const QString &message);

};

#endif // VIDEO_PLAYER_BACKEND_H
