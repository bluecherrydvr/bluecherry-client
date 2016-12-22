#include "VaapiHWAccel.h"

#if defined(Q_OS_LINUX)

#include <QDebug>
#include <QProcessEnvironment>

extern "C"
{
#include <va/va.h>
#include <va/va_x11.h>
#include <va/va_drm.h>
#include <libavutil/hwcontext_vaapi.h>
#include <libavcodec/vaapi.h>
}

enum AVPixelFormat VaapiHWAccel::get_format(AVCodecContext *s, const enum AVPixelFormat *pix_fmts)
{
}

int VaapiHWAccel::get_buffer(AVCodecContext *s, AVFrame *frame, int flags)
{
}

VaapiHWAccel::VaapiHWAccel()
    : m_hw_device_ctx(0)
{
    int err;
    const char *device;
    QProcessEnvironment env;

    env = QProcessEnvironment::systemEnvironment();

    if (env.contains(QLatin1String("DISPLAY")))
    {
        device = env.value(QLatin1String("DISPLAY")).toAscii().constData();
    }
    else
        device = "/dev/dri/renderD128";

    err = av_hwdevice_ctx_create(&m_hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI,
                                 device, NULL, 0);
    if (err < 0)
        qDebug() << "Failed to create VAAPI device context";
}

VaapiHWAccel::~VaapiHWAccel()
{
}

#endif
