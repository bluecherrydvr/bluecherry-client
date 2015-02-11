/*
 * Copyright 2010-2014 Bluecherry
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

#ifndef MPL_VIDEOWIDGET_H
#define MPL_VIDEOWIDGET_H

#include "video/VideoWidget.h"

#ifdef Q_OS_MAC
#include <QMutex>
#include <QImage>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

//class VideoRenderer;

struct VideoRendererWrapper;

#endif

class MplVideoWidget : public VideoWidget
{
    Q_OBJECT

public:
    explicit MplVideoWidget(QWidget *parent = 0);
    virtual ~MplVideoWidget();

    virtual void initVideo(VideoPlayerBackend *videoPlayerBackend);
    virtual void clearVideo();

    virtual QSize sizeHint() const;

    virtual void zoomIn();
    virtual void zoomOut();
    virtual double zoom() { return m_zoomFactor;}
    virtual void moveFrame(int dx, int dy);

    void setViewport(QWidget *viewport);

public slots:
    void setFullScreen(bool on);
    void toggleFullScreen() { setFullScreen(!isFullScreen()); }

    void setOverlayMessage(const QString &message);
    void clearOverlayMessage() { setOverlayMessage(QString()); }

protected:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void keyPressEvent(QKeyEvent *ev);
#ifdef Q_OS_MAC
    virtual bool eventFilter(QObject *, QEvent *);
#endif

private:
    QWidget *m_viewport;
    QString m_overlayMsg;
    int m_frameWidth, m_frameHeight;
    int m_normalFrameStyle;
    double m_zoomFactor;
    int m_originalWidth;
    int m_originalHeight;

    void setZoom(double z);

#ifdef Q_OS_MAC
    QString m_sharedBufferName;
    QMutex m_frameLock;
    int m_srcBufferSize;
    int m_dstBufferSize;
    AVPixelFormat m_pixelFormat;
    unsigned char *m_sharedBuffer;
    unsigned char *m_frontBuffer;
    unsigned char *m_backBuffer;
    int m_bpp;//bytes per pixel
    struct VideoRendererWrapper *m_renderer;

public:
    //methods for handling MPlayer OS X VO Protocol
    void initSharedMem(const char *bufferName, int width, int height, int bpp);
    void getFrame();
    void stop();
#endif
};

#endif //MPL_VIDEOWIDGET_H
