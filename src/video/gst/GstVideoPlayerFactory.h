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

#ifndef GST_VIDEO_PLAYER_FACTORY_H
#define GST_VIDEO_PLAYER_FACTORY_H

#include "video/VideoPlayerFactory.h"

class GstVideoPlayerFactory : public VideoPlayerFactory
{

public:
    virtual ~GstVideoPlayerFactory();

    virtual VideoWidget * createWidget(QWidget *parent = 0);
    virtual VideoPlayerBackend * createBackend(QObject *parent = 0);

};

#endif // GST_VIDEO_PLAYER_FACTORY_H
