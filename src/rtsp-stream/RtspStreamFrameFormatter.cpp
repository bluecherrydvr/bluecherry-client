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

#include "RtspStreamFrameFormatter.h"
#include "RtspStreamFrame.h"
#include <QDebug>

extern "C" {
#   include "libavcodec/avcodec.h"
#   include "libavformat/avformat.h"
#   include "libswscale/swscale.h"
}

RtspStreamFrameFormatter::RtspStreamFrameFormatter(AVStream *stream) :
        m_stream(stream), m_sws_context(0), m_pixelFormat(PIX_FMT_BGRA),
        m_autoDeinterlacing(true), m_shouldTryDeinterlaceStream(shouldTryDeinterlaceStream())
{
}

RtspStreamFrameFormatter::~RtspStreamFrameFormatter()
{
    sws_freeContext(m_sws_context);
}

void RtspStreamFrameFormatter::setAutoDeinterlacing(bool autoDeinterlacing)
{
    m_autoDeinterlacing = autoDeinterlacing;
}

bool RtspStreamFrameFormatter::shouldTryDeinterlaceStream()
{
    /* Assume that H.264 D1-resolution video is interlaced, to work around a solo(?) bug
     * that results in interlaced_frame not being set for videos from solo6110. */

    if (m_stream->codec->codec_id != CODEC_ID_H264)
        return false;

    if (m_stream->codec->width == 704 && m_stream->codec->height == 480)
        return true;

    if (m_stream->codec->width == 720 && m_stream->codec->height == 576)
        return true;

    return false;
}

RtspStreamFrame * RtspStreamFrameFormatter::formatFrame(AVFrame* avFrame)
{
    if (shouldTryDeinterlaceFrame(avFrame))
        deinterlaceFrame(avFrame);

    return new RtspStreamFrame(scaleFrame(avFrame));
}

bool RtspStreamFrameFormatter::shouldTryDeinterlaceFrame(AVFrame *avFrame)
{
    if (!m_autoDeinterlacing)
        return false;

    if (avFrame->interlaced_frame)
        return true;

    return m_shouldTryDeinterlaceStream;
}

void RtspStreamFrameFormatter::deinterlaceFrame(AVFrame* avFrame)
{
    int ret = avpicture_deinterlace((AVPicture*)avFrame, (AVPicture*)avFrame,
                                    m_stream->codec->pix_fmt, m_stream->codec->width, m_stream->codec->height);
    if (ret < 0)
        qDebug("deinterlacing failed");
}

AVFrame * RtspStreamFrameFormatter::scaleFrame(AVFrame* avFrame)
{
    updateSWSContext();

    int bufSize  = avpicture_get_size(m_pixelFormat, m_stream->codec->width, m_stream->codec->height);
    uint8_t *buf = (uint8_t*) av_malloc(bufSize);

    AVFrame *result = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)result, buf, m_pixelFormat, m_stream->codec->width, m_stream->codec->height);
    sws_scale(m_sws_context, (const uint8_t**)avFrame->data, avFrame->linesize, 0, m_stream->codec->height,
              result->data, result->linesize);

    result->width = m_stream->codec->width;
    result->height = m_stream->codec->height;
    result->pts = avFrame->pkt_pts;

    return result;
}

void RtspStreamFrameFormatter::updateSWSContext()
{
    m_sws_context = sws_getCachedContext(m_sws_context,
                                         m_stream->codec->width, m_stream->codec->height,
                                         m_stream->codec->pix_fmt,
                                         m_stream->codec->width, m_stream->codec->height,
                                         m_pixelFormat,
                                         SWS_BICUBIC, NULL, NULL, NULL);
}
