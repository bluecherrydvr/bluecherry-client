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

#ifndef VIDEOPLAYERBACKEND_GST_H
#define VIDEOPLAYERBACKEND_GST_H

#include <QObject>
#include <QWidget>
#include <QMutex>
#include <gst/gst.h>

class QUrl;
class VideoHttpBuffer;

typedef struct _GstDecodeBin GstDecodeBin;

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
    ~VideoPlayerBackend();

    static bool initGStreamer(QString *errorMessage = 0);

    GstElement *sink() const { return m_sink; }
    /* setSink must be called exactly and only once prior to setting up the pipeline */
    void setSink(GstElement *sink);

    qint64 duration() const;
    qint64 position() const;
    double playbackSpeed() const { return m_playbackSpeed; }
    bool isSeekable() const;
    bool atEnd() const { return m_state == Done; }
    VideoState state() const { return m_state; }
    bool isError() const { return m_state <= Error; }
    bool isPermanentError() const { return m_state == PermanentError; }
    QString errorMessage() const { return m_errorMessage; }
    VideoHttpBuffer *videoBuffer() const { return m_videoBuffer; }

public slots:
    bool start(const QUrl &url);
    void clear();

    void play();
    void playIfReady();
    void pause();
    bool seek(qint64 position);
    bool setSpeed(double speed);
    void restart();

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

private slots:
    void streamError(const QString &message);

private:
    QThread *m_controlThread;
    QMutex m_mutex;
    GstElement *m_pipeline, *m_videoLink, *m_sink;
    VideoHttpBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
    double m_playbackSpeed;

    void setError(bool permanent, const QString &message);
    void setVideoBuffer(VideoHttpBuffer *videoHttpBuffer);

    GstBusSyncReply busHandler(GstBus *bus, GstMessage *msg);
    void decodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast);

    static GstBusSyncReply staticBusHandler(GstBus *bus, GstMessage *msg, gpointer data);
    static void staticDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data);
};

#endif // VIDEOPLAYERBACKEND_GST_H
