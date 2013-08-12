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
class GstVideoBuffer;
class VideoBuffer;

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

    virtual void setVideoBuffer(VideoBuffer *videoBuffer);
    virtual VideoBuffer * videoBuffer() const;

    virtual bool start();
    virtual void clear();

public slots:
    virtual void play();
    virtual void playIfReady();
    virtual void pause();
    virtual bool seek(qint64 position);
    virtual bool setSpeed(double speed);
    virtual void restart();

private slots:
    void bufferingError(const QString &bufferingErrorMessage);

private:
    QThread *m_controlThread;
    QMutex m_mutex;
    GstElement *m_pipeline, *m_videoLink, *m_sink;
    GstVideoBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
    double m_playbackSpeed;

    void setErrorMessage(bool permanent, const QString &errorMessage);
    void setGstVideoBuffer(GstVideoBuffer *gstVideoBuffer);

    GstBusSyncReply busHandler(GstBus *bus, GstMessage *msg);
    void decodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast);

    static GstBusSyncReply staticBusHandler(GstBus *bus, GstMessage *msg, gpointer data);
    static void staticDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data);
};

#endif // GST_VIDEO_PLAYER_BACKEND_H
