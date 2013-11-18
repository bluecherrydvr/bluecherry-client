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

#ifndef RTSP_STREAM_FRAME_FORMATTER_H
#define RTSP_STREAM_FRAME_FORMATTER_H

extern "C" {
#   include "libavutil/pixfmt.h"
}

class RtspStreamFrame;
struct AVFrame;
struct AVStream;

struct SwsContext;
 
class RtspStreamFrameFormatter
{
public:
    explicit RtspStreamFrameFormatter(AVStream *stream);
    ~RtspStreamFrameFormatter();

    void setAutoDeinterlacing(bool autoDeinterlacing);
    RtspStreamFrame * formatFrame(AVFrame *avFrame);

private:
    AVStream *m_stream;
    SwsContext *m_sws_context;
    PixelFormat m_pixelFormat;
    bool m_autoDeinterlacing;
    bool m_shouldTryDeinterlaceStream;

    bool shouldTryDeinterlaceStream();
    bool shouldTryDeinterlaceFrame(AVFrame *avFrame);
    void deinterlaceFrame(AVFrame *avFrame);
    AVFrame * scaleFrame(AVFrame *avFrame);
    void updateSWSContext();

};

#endif // RTSP_STREAM_FRAME_FORMATTER_H
