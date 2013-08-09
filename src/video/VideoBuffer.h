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

#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

#include <QObject>

class VideoBuffer : public QObject
{
    Q_OBJECT

public:
    explicit VideoBuffer(QObject *parent = 0);
    virtual ~VideoBuffer();

    virtual bool isBuffering() const = 0;
    virtual bool isBufferingFinished() const = 0;
    virtual int bufferedPercent() const = 0;

    virtual unsigned int totalBytes() const = 0;
    virtual bool isEndOfStream() const = 0;

    virtual QByteArray read(unsigned int bytes) = 0;
    virtual bool seek(unsigned int offset) = 0;

};

#endif // VIDEO_BUFFER_H
