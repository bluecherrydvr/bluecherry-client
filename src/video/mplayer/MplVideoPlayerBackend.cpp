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

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QUrl>
#include <QRegExp>
#include <QByteArray>
#include <QDebug>

#include "MplVideoPlayerBackend.h"
#include "video/VideoHttpBuffer.h"

MplVideoPlayerBackend::~MplVideoPlayerBackend()
{
    clear();
}

MplVideoPlayerBackend::MplVideoPlayerBackend(QObject *parent)
    : VideoPlayerBackend(parent),
      m_videoBuffer(0), m_state(Stopped),
      m_playbackSpeed(1.0), m_mplayer(0),
      m_wid(QString()), m_errorMessage(QString())
{
    qDebug() << "MplVideoPlayerBackend() this =" << this << "\n";
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
        connect(m_videoBuffer, SIGNAL(bufferingFinished()), SLOT(playIfReady()));
        connect(m_videoBuffer, SIGNAL(streamError(QString)), SLOT(streamError(QString)));
    }
}

void MplVideoPlayerBackend::setWindowId(quint64 wid)
{
    m_wid = QString::number(wid);
}

bool MplVideoPlayerBackend::start(const QUrl &url)
{
    qDebug() << "MplVideoPlayerBackend::start url =" << url << "\n";
    if (state() == PermanentError)
        return false;

    if (m_wid.isNull() || m_wid.isEmpty())
    {
        setError(false, tr("Window id is not set"));
        return false;
    }

    if (m_mplayer)
        m_mplayer->deleteLater();

    m_mplayer = new MplayerProcess(m_wid, this);
    connect(m_mplayer, SIGNAL(mplayerError(bool,QString)), this, SLOT(setError(bool,QString)));
    connect(m_mplayer, SIGNAL(eof()), this, SLOT(handleEof()));
    connect(m_mplayer, SIGNAL(readyToPlay()), this, SLOT(mplayerReady()));
    connect(m_mplayer, SIGNAL(durationChanged()), this, SLOT(durationIsKnown()));


    /* Buffered HTTP source */
    setVideoBuffer(new VideoHttpBuffer(url));

    m_videoBuffer->startBuffering();

    m_playbackSpeed = 1.0;
    qDebug() << "MplVideoPlayerBackend::start mplayer process started\n";
    return true;
}

void MplVideoPlayerBackend::clear()
{
    if (m_mplayer)
    {
#ifdef Q_OS_MAC
        delete m_mplayer;
#else
        m_mplayer->deleteLater();
#endif
        m_mplayer = 0;
    }

    setVideoBuffer(0);

    m_state = Stopped;
    m_errorMessage.clear();
}

void MplVideoPlayerBackend::setError(bool permanent, const QString message)
{
    VideoState old = m_state;
    m_state = permanent ? PermanentError : Error;
    m_errorMessage = message;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::streamError(const QString &message)
{
    qDebug() << "MplVideoPlayerBackend: stopping stream due to error:" << message;

    //close mplayer process?

    setError(true, message);
}

bool MplVideoPlayerBackend::saveScreenshot(QString &file)
{
    if (!m_mplayer || !m_mplayer->isRunning() || !m_mplayer->isReadyToPlay())
        return false;

    return m_mplayer->saveScreenshot(file);
}


void MplVideoPlayerBackend::handleEof()
{
    qDebug() << "EOF\n";
    VideoState old = m_state;
    m_state = Done;
    emit stateChanged(m_state, old);
    emit endOfStream();
}

void MplVideoPlayerBackend::mplayerReady()
{
    qDebug() << this << "mplayer is ready to play\n";

    emit streamsInitialized(true);

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::durationIsKnown()
{
    emit durationChanged(duration());
}

bool MplVideoPlayerBackend::isSeekable() const
{
    return true;
}

void MplVideoPlayerBackend::playIfReady()
{
    if (!m_mplayer || !m_mplayer->start(m_videoBuffer->bufferFilePath()))
        return;
}

void MplVideoPlayerBackend::play()
{
    if (!m_mplayer || !m_mplayer->isRunning() || !m_mplayer->isReadyToPlay())
        return;

    m_mplayer->play();

    emit playbackSpeedChanged(m_playbackSpeed);

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::pause()
{
    if (!m_mplayer || !m_mplayer->isRunning() || !m_mplayer->isReadyToPlay())
        return;

    m_mplayer->pause();

    VideoState old = m_state;
    m_state = Paused;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::restart()
{
    if (!m_mplayer)
        return;

    m_mplayer->deleteLater();

    m_mplayer = new MplayerProcess(m_wid, this);
    connect(m_mplayer, SIGNAL(mplayerError(bool,QString)), this, SLOT(setError(bool,QString)));
    connect(m_mplayer, SIGNAL(eof()), this, SLOT(handleEof()));
    connect(m_mplayer,SIGNAL(readyToPlay()), this, SLOT(mplayerReady()));
    connect(m_mplayer, SIGNAL(durationChanged()), this, SLOT(durationIsKnown()));

    m_mplayer->start(m_videoBuffer->bufferFilePath());

    VideoState old = m_state;
    m_state = Stopped;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::mute(bool mute)
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return;

    m_mplayer->mute(mute);
}

void MplVideoPlayerBackend::setVolume(double volume)
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return;

    volume*=100.0;

    m_mplayer->setVolume(volume);
}

int MplVideoPlayerBackend::duration() const
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return -1;

    double secs = m_mplayer->duration();
    return secs < 0 ? -1 : secs  * 1000.0;
}

int MplVideoPlayerBackend::position() const
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return -1;

    double secs = m_mplayer->position();

    return secs > 0 ? secs * 1000.0 : -1;
}

void MplVideoPlayerBackend::setHardwareDecodingEnabled(bool enable)
{
    //implement later
}

bool MplVideoPlayerBackend::seek(int position)
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return false;

    return m_mplayer->seek((double)position/1000.0);
}

bool MplVideoPlayerBackend::setSpeed(double speed)
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return false;

    if (speed == m_playbackSpeed)
        return true;

    if (speed == 0)
        return false;

    m_mplayer->setSpeed(speed);

    m_playbackSpeed = speed;
    m_lastspeed = speed;
    emit playbackSpeedChanged(m_playbackSpeed);

    return speed;
}



