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

#include "MplVideoPlayerBackend.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QUrl>

MplVideoPlayerBackend::~MplVideoPlayerBackend()
{
    clear();
}


MplVideoPlayerBackend::MplVideoPlayerBackend(QObject *parent)
    : VideoPlayerBackend(parent),
      m_videoBuffer(0), m_state(Stopped),m_playbackSpeed(1.0)
{

}

void MplVideoPlayerBackend::setVideoBuffer(VideoHttpBuffer *videoHttpBuffer)
{
    if (m_videoBuffer)
    {
        disconnect(m_videoBuffer, 0, this, 0);
        m_videoBuffer->clearPlayback();
        m_videoBuffer->deleteLater();
    }

    m_videoBuffer = videoHttpBuffer;

    if (m_videoBuffer)
    {
        connect(m_videoBuffer, SIGNAL(bufferingStarted()), this, SIGNAL(bufferingStarted()));
        connect(m_videoBuffer, SIGNAL(bufferingStopped()), this, SIGNAL(bufferingStopped()));
        connect(m_videoBuffer, SIGNAL(bufferingReady()), SLOT(playIfReady()));
        connect(m_videoBuffer, SIGNAL(streamError(QString)), SLOT(streamError(QString)));
    }
}

bool MplVideoPlayerBackend::start(const QUrl &url)
{
    if (state() == PermanentError)
        return false;

    /* Buffered HTTP source */
    setVideoBuffer(new VideoHttpBuffer(url));

    m_videoBuffer->startBuffering();

    m_mplayer.start("mplayer", QStringList() << "-slave" << "-idle");

    if (!m_mplayer.waitForStarted())
    {
        QString errStr(tr("MPlayer process "));

        switch(m_mplayer.error())
        {
        case QProcess::FailedToStart:
            errStr.append(tr("failed to start"));
        break;

        case QProcess::Crashed:
            errStr.append(tr("crashed"));
        break;

        default:
            errStr.append(tr("- unable to start for unknown reason"));
        }

        setError(true, errStr);
        return false;
    }

    m_playbackSpeed = 1.0;

    return true;
}

void MplVideoPlayerBackend::clear()
{
    if (QProcess::NotRunning != m_mplayer.state())
    {
        m_mplayer.write("quit");
        if (!m_mplayer.waitForFinished(1000))
        {
            m_mplayer.kill();
        }
    }

    setVideoBuffer(0);

    m_state = Stopped;
    m_errorMessage.clear();
}

void MplVideoPlayerBackend::setError(bool permanent, const QString &message)
{
    VideoState old = m_state;
    m_state = permanent ? PermanentError : Error;
    m_errorMessage = message;
    emit stateChanged(m_state, old);
}

bool MplVideoPlayerBackend::isSeekable() const
{
    return true;
}

void MplVideoPlayerBackend::playIfReady()
{
    if (m_mplayer.state() != QProcess::Running)
    {
        return;
    }

    m_mplayer.write("pausing loadfile " + m_videoBuffer->/*Get temp file name*/);
}

