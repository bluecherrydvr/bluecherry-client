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

#include "LiveStreamFrameFormatter.h"
#include "LiveStreamFrame.h"
#include <QDebug>

extern "C" {
#   include "libavformat/avformat.h"
#   include "libswscale/swscale.h"
}

LiveStreamFrameFormatter::LiveStreamFrameFormatter(AVStream *stream) :
        m_stream(stream), m_sws_context(0), m_autoDeinterlacing(true)
{
}

LiveStreamFrameFormatter::~LiveStreamFrameFormatter()
{
    sws_freeContext(m_sws_context);
}

void LiveStreamFrameFormatter::setAutoDeinterlacing(bool autoDeinterlacing)
{
    m_autoDeinterlacing = autoDeinterlacing;
}

LiveStreamFrame * LiveStreamFrameFormatter::formatFrame(AVFrame* avFrame)
{
    const PixelFormat fmt = PIX_FMT_BGRA;

    /* Assume that H.264 D1-resolution video is interlaced, to work around a solo(?) bug
     * that results in interlaced_frame not being set for videos from solo6110. */
    if (m_autoDeinterlacing && (avFrame->interlaced_frame ||
                              (m_stream->codec->codec_id == CODEC_ID_H264 &&
                               ((m_stream->codec->width == 704 && m_stream->codec->height == 480) ||
                                (m_stream->codec->width == 720 && m_stream->codec->height == 576)))))
    {
        if (avpicture_deinterlace((AVPicture*)avFrame, (AVPicture*)avFrame, m_stream->codec->pix_fmt,
                                  m_stream->codec->width, m_stream->codec->height) < 0)
        {
            qDebug("deinterlacing failed");
        }
    }

    m_sws_context = sws_getCachedContext(m_sws_context, m_stream->codec->width, m_stream->codec->height, m_stream->codec->pix_fmt,
                               m_stream->codec->width, m_stream->codec->height, fmt, SWS_BICUBIC,
                               NULL, NULL, NULL);

    int bufSize  = avpicture_get_size(fmt, m_stream->codec->width, m_stream->codec->height);
    uint8_t *buf = (uint8_t*) av_malloc(bufSize);

    AVFrame *frame = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)frame, buf, fmt, m_stream->codec->width, m_stream->codec->height);
    sws_scale(m_sws_context, (const uint8_t**)avFrame->data, avFrame->linesize, 0, m_stream->codec->height,
              frame->data, frame->linesize);

    frame->width = m_stream->codec->width;
    frame->height = m_stream->codec->height;
    frame->pts = avFrame->pkt_pts;

    return new LiveStreamFrame(frame);
}
