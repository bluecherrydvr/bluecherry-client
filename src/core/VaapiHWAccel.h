/*
 * Copyright 2010-2016 Bluecherry
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

#ifndef VAAPIHWACCEL_H
#define VAAPIHWACCEL_H

#include <QtGlobal>

#if defined(Q_OS_LINUX)

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/pixfmt.h>
#include <libavutil/buffer.h>
}

class VaapiHWAccel
{

public:
    VaapiHWAccel();
    ~VaapiHWAccel();

    bool isAvailable() {return m_available;}

    //AVCodecContext callbacks
    static enum AVPixelFormat get_format(AVCodecContext *s, const enum AVPixelFormat *pix_fmts);
    static int get_buffer(AVCodecContext *s, AVFrame *frame, int flags);
    static int decoderInit(AVCodecContext *s);
    static int retrieveData(AVCodecContext *s, AVFrame *input);
private:
    static VaapiHWAccel *m_instance;

    bool m_available;
    AVBufferRef *m_hw_device_ctx;

};
#endif

#endif // VAAPIHWACCEL_H
