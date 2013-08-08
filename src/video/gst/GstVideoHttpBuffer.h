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

#ifndef GST_VIDEO_HTTP_BUFFER_H
#define GST_VIDEO_HTTP_BUFFER_H

#include <QObject>

#include "video/VideoHttpBuffer.h"

class GstVideoHttpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit GstVideoHttpBuffer(VideoHttpBuffer *buffer, QObject *parent = 0);
    virtual ~GstVideoHttpBuffer();

    void startBuffering();
    bool isBuffering() const;
    bool isBufferingFinished() const;
    int bufferedPercent() const;

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

private:
    QScopedPointer<VideoHttpBuffer> m_buffer;

};

#endif // GST_VIDEO_HTTP_BUFFER_H
