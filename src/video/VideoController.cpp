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

#include "VideoController.h"

VideoController::VideoController(QObject *parent) :
        QObject(parent)
{
}

VideoController::~VideoController()
{
    setVideoPlayerBackend(0);
}

void VideoController::setVideoPlayerBackend(VideoPlayerBackend *videoPlayerBackend)
{
    disconnectBackend();
    if (m_videoPlayerBackend)
        m_videoPlayerBackend.data()->deleteLater();
    m_videoPlayerBackend = videoPlayerBackend;
    connectBackend();
}

VideoPlayerBackend * VideoController::videoPlayerBackend() const
{
    return m_videoPlayerBackend.data();
}

void VideoController::disconnectBackend()
{
    if (!m_videoPlayerBackend)
        return;

    disconnect(m_videoPlayerBackend.data(), 0, this, 0);
}

void VideoController::connectBackend()
{
    if (!m_videoPlayerBackend)
        return;

    connect(this, SIGNAL(startRequested()), m_videoPlayerBackend.data(), SLOT(start()));
    connect(this, SIGNAL(clearRequested()), m_videoPlayerBackend.data(), SLOT(clear()));
    connect(this, SIGNAL(playRequested()), m_videoPlayerBackend.data(), SLOT(play()));
    connect(this, SIGNAL(pauseRequested()), m_videoPlayerBackend.data(), SLOT(pause()));
    connect(this, SIGNAL(restartRequested()), m_videoPlayerBackend.data(), SLOT(restart()));
    connect(this, SIGNAL(seekRequested(qint64)), m_videoPlayerBackend.data(), SLOT(seek(qint64)));
    connect(this, SIGNAL(setSpeedRequested(double)), m_videoPlayerBackend.data(), SLOT(setSpeed(double)));
}

void VideoController::start()
{
    emit startRequested();
}

void VideoController::clear()
{
    emit clearRequested();
}

void VideoController::playPause()
{
    if (m_videoPlayerBackend.data()->state() != VideoPlayerBackend::Playing)
    {
        if (m_videoPlayerBackend.data()->atEnd())
            restart();
        else
            emit playRequested();
    }
    else
        emit pauseRequested();
}

void VideoController::restart()
{
    emit restartRequested();
    emit playRequested();
}

void VideoController::seek(qint64 position)
{
    emit seekRequested(position);
}

void VideoController::setSpeed(double speed)
{
    emit setSpeedRequested(speed);
}
