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

#include "GstVideoBuffer.h"
#include "video/VideoHttpBuffer.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

gboolean GstVideoBuffer::seekDataWrap(GstAppSrc *src, guint64 offset, gpointer user_data)
{
    Q_UNUSED(src);

    return static_cast<GstVideoBuffer *>(user_data)->seek(offset);
}

void GstVideoBuffer::needDataWrap(GstAppSrc *src, unsigned size, gpointer user_data)
{
    Q_UNUSED(src);

    return static_cast<GstVideoBuffer *>(user_data)->needData(size);
}

GstVideoBuffer::GstVideoBuffer(VideoBuffer *buffer, QObject *parent) :
        VideoBuffer(parent), m_buffer(buffer), m_pipeline(0), m_element(0)
{
    connect(buffer, SIGNAL(error(QString)), this, SLOT(errorSlot(QString)));
    connect(buffer, SIGNAL(totalBytesChanged(uint)), this, SLOT(totalBytesChangedSlot(uint)));
    connect(buffer, SIGNAL(bufferingStarted()), this, SIGNAL(bufferingStarted()));
    connect(buffer, SIGNAL(bufferingStopped()), this, SIGNAL(bufferingStopped()));
    connect(buffer, SIGNAL(bufferingFinished()), this, SIGNAL(bufferingFinished()));
}

GstVideoBuffer::~GstVideoBuffer()
{
}

void GstVideoBuffer::startBuffering()
{
    m_buffer.data()->startBuffering();
}

bool GstVideoBuffer::isBuffering() const
{
    return m_buffer.data()->isBuffering();
}

bool GstVideoBuffer::isBufferingFinished() const
{
    return m_buffer.data()->isBufferingFinished();
}

int GstVideoBuffer::bufferedPercent() const
{
    return m_buffer.data()->bufferedPercent();
}

unsigned int GstVideoBuffer::totalBytes() const
{
    return m_buffer.data()->totalBytes();
}

bool GstVideoBuffer::isEndOfStream() const
{
    return m_buffer.data()->isEndOfStream();
}

QByteArray GstVideoBuffer::read(unsigned int bytes)
{
    return m_buffer.data()->read(bytes);
}

bool GstVideoBuffer::seek(unsigned int offset)
{
    return m_buffer.data()->seek(offset);
}

GstElement * GstVideoBuffer::setupSrcElement(GstElement *pipeline)
{
    Q_ASSERT(!m_element && !m_pipeline);
    if (m_element || m_pipeline)
    {
        if (m_pipeline == pipeline)
            return GST_ELEMENT(m_element);
        return 0;
    }

    m_element = GST_APP_SRC(gst_element_factory_make("appsrc", "source"));
    if (!m_element)
        return 0;

    g_object_ref(m_element);

    m_pipeline = pipeline;

    gst_app_src_set_max_bytes(m_element, 0);
    gst_app_src_set_stream_type(m_element, GST_APP_STREAM_TYPE_RANDOM_ACCESS);

    GstAppSrcCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.need_data = needDataWrap;
    callbacks.seek_data = seekDataWrap;
    gst_app_src_set_callbacks(m_element, &callbacks, this, 0);

    if (totalBytes())
        gst_app_src_set_size(m_element, totalBytes());

    gst_bin_add(GST_BIN(m_pipeline), GST_ELEMENT(m_element));
    return GST_ELEMENT(m_element);
}

void GstVideoBuffer::clearPlayback()
{
    if (m_element)
    {
        qDebug("gstreamer: Destroying source element");

        GstAppSrcCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        gst_app_src_set_callbacks(m_element, &callbacks, 0, 0);

        GstStateChangeReturn re = gst_element_set_state(GST_ELEMENT(m_element), GST_STATE_NULL);
        Q_UNUSED(re);
        Q_ASSERT(re == GST_STATE_CHANGE_SUCCESS);

       g_object_unref(m_element);
    }

    m_element = 0;
    m_pipeline = 0;
}

void GstVideoBuffer::needData(unsigned int bytes)
{
    QByteArray data = read(bytes);
    tryPushBuffer(data);
}

void GstVideoBuffer::tryPushBuffer(const QByteArray &buffer)
{

    if (isBufferValid(buffer))
        pushBuffer(buffer);
    else if (isEndOfStream())
    {
        qDebug() << "GstVideoBuffer: end of stream";
        gst_app_src_end_of_stream(m_element);
    }
    else
        qDebug() << "GstVideoBuffer: read aborted";
}

bool GstVideoBuffer::isBufferValid(const QByteArray &buffer) const
{
    if (!buffer.isEmpty())
        return true;

    if (buffer.isNull())
    {
        /* Error reporting is handled by MediaDownload for this case */
        qDebug() << "GstVideoBuffer: read error";
        return false;
    }

    return false;
}

void GstVideoBuffer::pushBuffer(const QByteArray& buffer)
{
    GstBuffer *gstBuffer = gst_buffer_new();
    GST_BUFFER_SIZE(gstBuffer) = buffer.size();
    GST_BUFFER_MALLOCDATA(gstBuffer) = (guint8 *)g_malloc(buffer.size());
    GST_BUFFER_DATA(gstBuffer) = GST_BUFFER_MALLOCDATA(gstBuffer);
    memcpy(GST_BUFFER_DATA(gstBuffer), buffer.constData(), buffer.size());

    GstFlowReturn flow = gst_app_src_push_buffer(m_element, gstBuffer);

    if (flow != GST_FLOW_OK)
        qDebug() << "GstVideoBuffer: Push result is" << flow;
}

void GstVideoBuffer::errorSlot(const QString &errorMessage)
{
    gst_element_set_state(GST_ELEMENT(m_element), GST_STATE_NULL);
    emit error(errorMessage);
}

void GstVideoBuffer::totalBytesChangedSlot(unsigned size)
{
    if (!size)
        qDebug() << "VideoHttpBuffer: fileSize is 0, may cause problems!";

    bool firstTime = gst_app_src_get_size(m_element) <= 0;

    if (m_element)
        gst_app_src_set_size(m_element, size);

    if (firstTime && size)
        emit bufferingReady();

    emit totalBytesChanged(size);
}
