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

#ifndef RTSP_STREAM_FRAME_H
#define RTSP_STREAM_FRAME_H

#include <QtGlobal>

struct AVFrame;

class RtspStreamFrame
{
    Q_DISABLE_COPY(RtspStreamFrame);

public:
    explicit RtspStreamFrame(AVFrame *avFrame, int width, int height);
    ~RtspStreamFrame();

    AVFrame * avFrame() const;
    int width() { return m_streamWidth; }
    int height() { return m_streamHeight; }

private:
    AVFrame *m_avFrame;
    int m_streamWidth;
    int m_streamHeight;
};

#endif // RTSP_STREAM_FRAME_H
