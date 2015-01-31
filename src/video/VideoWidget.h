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

#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QFrame>

class VideoPlayerBackend;

class VideoWidget : public QFrame
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

    virtual void initVideo(VideoPlayerBackend *videoPlayerBackend) = 0;
    virtual void clearVideo() = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void moveFrame(int dx, int dy) = 0;

};

#endif // VIDEO_WIDGET_H
