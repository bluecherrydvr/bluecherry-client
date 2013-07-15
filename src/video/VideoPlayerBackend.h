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

#ifndef VIDEO_PLAYER_BACKEND_H
#define VIDEO_PLAYER_BACKEND_H

#include <QObject>

class VideoPlayerBackend : public QObject
{
    Q_OBJECT

public:
    explicit VideoPlayerBackend(QObject *parent = 0);
    virtual ~VideoPlayerBackend();

};

#endif // VIDEO_PLAYER_BACKEND_H
