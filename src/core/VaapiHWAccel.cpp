#include "VaapiHWAccel.h"

#if defined(Q_OS_LINUX)

#include <QDebug>
#include <QProcessEnvironment>
#include <QByteArray>

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
#if defined(Q_WS_X11)
    QProcessEnvironment env;
    QByteArray x11display;

    env = QProcessEnvironment::systemEnvironment();

    if (env.contains(QLatin1String("DISPLAY")))
    {
        x11display = env.value(QLatin1String("DISPLAY")).toAscii();
        qDebug() << "VAAPI - using X11 display " << x11display;
        device = x11display.constData();
    }
    else
#endif
        device = "/dev/dri/renderD128";

    qDebug() << "trying to create VAAPI device context \"" << device << "\"";

    err = av_hwdevice_ctx_create(&m_hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI,
                                 device, NULL, 0);
    if (err < 0)
        qDebug() << "Failed to create VAAPI device context";
}

VaapiHWAccel::~VaapiHWAccel()
{
}

#endif
