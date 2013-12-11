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

#ifndef GST_VIDEO_PLAYER_BACKEND_H
#define GST_VIDEO_PLAYER_BACKEND_H

#include "video/VideoPlayerBackend.h"
#include <QWidget>
#include <QMutex>
#include <gst/gst.h>

class QUrl;
class VideoHttpBuffer;

typedef struct _GstDecodeBin GstDecodeBin;

class GstVideoPlayerBackend : public VideoPlayerBackend
{
    Q_OBJECT

public:
    explicit GstVideoPlayerBackend(QObject *parent = 0);
    virtual ~GstVideoPlayerBackend();

    bool initGStreamer();

    GstElement *sink() const { return m_sink; }
    /* setSink must be called exactly and only once prior to setting up the pipeline */
    void setSink(GstElement *sink);

    virtual qint64 duration() const;
    virtual qint64 position() const;
    virtual double playbackSpeed() const { return m_playbackSpeed; }
    virtual bool isSeekable() const;
    virtual bool atEnd() const { return m_state == Done; }
    virtual VideoState state() const { return m_state; }
    virtual bool isError() const { return m_state <= Error; }
    virtual bool isPermanentError() const { return m_state == PermanentError; }
    virtual QString errorMessage() const { return m_errorMessage; }
    virtual VideoHttpBuffer *videoBuffer() const { return m_videoBuffer; }

public slots:
    virtual bool start(const QUrl &url);
    virtual void clear();

    virtual void play();
    virtual void playIfReady();
    virtual void pause();
    virtual bool seek(qint64 position);
    virtual bool setSpeed(double speed);
    virtual void restart();

private slots:
    void streamError(const QString &message);

private:
    QThread *m_controlThread;
    QMutex m_mutex;
    GstElement *m_pipeline, *m_videoLink, *m_sink, *m_audioLink, *m_audioQueue, *m_videoQueue;
    VideoHttpBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
    double m_playbackSpeed;
    bool m_hasAudio;

    GstElement *m_audioDecoder, *m_audioResample, *m_audioSink;

    void setError(bool permanent, const QString &message);
    void setVideoBuffer(VideoHttpBuffer *videoHttpBuffer);

    bool setupAudioPipeline();
    bool setupVideoPipeline();

    GstBusSyncReply busHandler(GstBus *bus, GstMessage *msg);
    void decodeAudioPadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast);
    void decodeVideoPadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast);
    void demuxerPadReady(GstElement *element, GstPad *pad);
    void demuxerNoMorePads(GstElement *demux);

    static GstBusSyncReply staticBusHandler(GstBus *bus, GstMessage *msg, gpointer data);
    static void staticVideoDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data);
    static void staticAudioDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data);
    static void staticDemuxerPadReady(GstElement *element, GstPad *pad, gpointer data);
    static void staticDemuxerNoMorePads(GstElement *demux, gpointer user_data);

};

#endif // GST_VIDEO_PLAYER_BACKEND_H
