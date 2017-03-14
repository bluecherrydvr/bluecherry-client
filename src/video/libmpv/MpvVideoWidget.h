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

#ifndef MPV_VIDEOWIDGET_H
#define MPV_VIDEOWIDGET_H

#include "video/VideoWidget.h"

class MpvVideoWidget : public VideoWidget
{
    Q_OBJECT

public:
    explicit MpvVideoWidget(QWidget *parent = 0);
    virtual ~MpvVideoWidget();

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

private:
    QWidget *m_viewport;
    QString m_overlayMsg;
    int m_frameWidth, m_frameHeight;
    int m_normalFrameStyle;
    double m_zoomFactor;
    int m_originalWidth;
    int m_originalHeight;

    void setZoom(double z);
};

#endif //MPV_VIDEOWIDGET_H
