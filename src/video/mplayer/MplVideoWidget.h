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

class MplVideoWidget : public VideoWidget
{
    Q_OBJECT

public:
    explicit MplVideoWidget(QWidget *parent = 0);
    virtual ~MplVideoWidget();

    virtual void initVideo(VideoPlayerBackend *videoPlayerBackend);
    virtual void clearVideo();

    virtual QImage currentFrame();

    virtual QSize sizeHint() const;

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
    virtual bool eventFilter(QObject *, QEvent *);

private:
    QWidget *m_viewport;
    QString m_overlayMsg;
    int m_frameWidth, m_frameHeight;
};

#endif //MPL_VIDEOWIDGET_H
