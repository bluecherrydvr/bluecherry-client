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

#include "VideoHttpBuffer.h"
#include "core/BluecherryApp.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QNetworkCookieJar>
#include <QApplication>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

void VideoHttpBuffer::needDataWrap(GstAppSrc *src, guint length, gpointer user_data)
{
    Q_UNUSED(src);
    Q_ASSERT(length < INT_MAX);
    static_cast<VideoHttpBuffer*>(user_data)->needData(int(length));
}

gboolean VideoHttpBuffer::seekDataWrap(GstAppSrc *src, quint64 offset, gpointer user_data)
{
    Q_UNUSED(src);
    return static_cast<VideoHttpBuffer*>(user_data)->seekData(offset) ? TRUE : FALSE;
}

VideoHttpBuffer::VideoHttpBuffer(QObject *parent)
    : QObject(parent), media(0), m_element(0), m_pipeline(0)
{
}

VideoHttpBuffer::~VideoHttpBuffer()
{
    clearPlayback();
    if (media)
        media->deref();
}

GstElement *VideoHttpBuffer::setupSrcElement(GstElement *pipeline)
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
    callbacks.seek_data = (gboolean (*)(GstAppSrc*,guint64,void*))seekDataWrap;
    gst_app_src_set_callbacks(m_element, &callbacks, this, 0);

    if (media && media->fileSize())
        gst_app_src_set_size(m_element, media->fileSize());

    gst_bin_add(GST_BIN(m_pipeline), GST_ELEMENT(m_element));
    return GST_ELEMENT(m_element);
}

void VideoHttpBuffer::setCookies(const QList<QNetworkCookie> &cookies)
{
    m_cookies = cookies;
}

bool VideoHttpBuffer::startBuffering(const QUrl &url)
{
    Q_ASSERT(!media);

    media = new MediaDownload;
    media->ref();
    connect(media, SIGNAL(fileSizeChanged(uint)), SLOT(fileSizeChanged(uint)), Qt::DirectConnection);
    connect(media, SIGNAL(finished()), SIGNAL(bufferingFinished()));
    connect(media, SIGNAL(stopped()), SIGNAL(bufferingStopped()));
    connect(media, SIGNAL(error(QString)), SLOT(sendStreamError(QString)));

    media->start(url, m_cookies);

    qDebug("VideoHttpBuffer: started");
    emit bufferingStarted();

    return true;
}

void VideoHttpBuffer::clearPlayback()
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

void VideoHttpBuffer::sendStreamError(const QString &message)
{
    emit streamError(message);
    emit bufferingStopped();
    gst_element_set_state(GST_ELEMENT(m_element), GST_STATE_NULL);
}

void VideoHttpBuffer::fileSizeChanged(unsigned fileSize)
{
    if (!fileSize)
        qDebug() << "VideoHttpBuffer: fileSize is 0, may cause problems!";

    bool firstTime = gst_app_src_get_size(m_element) <= 0;

    if (m_element)
        gst_app_src_set_size(m_element, fileSize);

    if (firstTime && fileSize)
        emit bufferingReady();
}

void VideoHttpBuffer::needData(int size)
{
    Q_ASSERT(media);

    /* Refactor to use gst_pad_alloc_buffer? Probably wouldn't provide any benefit. */
    GstBuffer *buffer = gst_buffer_new_and_alloc(size);

    int re = media->read(media->readPosition(), (char*)GST_BUFFER_DATA(buffer), size);
    if (re < 0)
    {
        /* Error reporting is handled by MediaDownload for this case */
        qDebug() << "VideoHttpBuffer: read error";
        return;
    }
    else if (re == 0)
    {
        if (media->readPosition() >= media->fileSize() && media->isFinished())
        {
            qDebug() << "VideoHttpBuffer: end of stream";
            gst_app_src_end_of_stream(m_element);
        }
        else
            qDebug() << "VideoHttpBuffer: read aborted";
        return;
    }

    GST_BUFFER_SIZE(buffer) = re;

    GstFlowReturn flow = gst_app_src_push_buffer(m_element, buffer);
    if (flow != GST_FLOW_OK)
        qDebug() << "VideoHttpBuffer: Push result is" << flow;
}

bool VideoHttpBuffer::seekData(qint64 offset)
{
    Q_ASSERT(media);
    return media->seek((unsigned)offset);
}
