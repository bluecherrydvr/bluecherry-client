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

#ifndef GST_VIDEO_BUFFER_H
#define GST_VIDEO_BUFFER_H

#include <QObject>
#include <glib.h>

#include "video/VideoBuffer.h"

class VideoHttpBuffer;

typedef struct _GstAppSrc GstAppSrc;
typedef struct _GstElement GstElement;

class GstVideoBuffer : public VideoBuffer
{
    Q_OBJECT

public:
    explicit GstVideoBuffer(VideoHttpBuffer *buffer, QObject *parent = 0);
    virtual ~GstVideoBuffer();

    void startBuffering();
    virtual bool isBuffering() const;
    virtual bool isBufferingFinished() const;
    virtual int bufferedPercent() const;

    virtual unsigned int totalBytes() const;

    virtual QByteArray read(unsigned int bytes);
    virtual bool seek(unsigned int offset);

    void clearPlayback();

    /* Create and prepare a source element; the element will be added to the pipeline,
     * but not linked. */
    GstElement * setupSrcElement(GstElement *pipeline);

signals:
    void streamError(const QString &message);
    void bufferingStarted();
    void bufferingReady();
    void bufferingStopped();
    void bufferingFinished();
    void sizeChanged(unsigned size);

private:
    QScopedPointer<VideoHttpBuffer> m_buffer;
    GstElement *m_pipeline;
    GstAppSrc *m_element;

    static void needDataWrap(GstAppSrc *src, unsigned bytes, gpointer user_data);
    void needData(unsigned bytes);

    static int seekDataWrap(GstAppSrc *src, guint64 offset, gpointer user_data);

private slots:
    void streamErrorSlot(const QString &error);
    void sizeChangedSlot(unsigned size);

};

#endif // GST_VIDEO_BUFFER_H
