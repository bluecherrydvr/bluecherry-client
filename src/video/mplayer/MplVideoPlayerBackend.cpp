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
#include <QTimer>
#include <QDebug>

#include "MplVideoPlayerBackend.h"
#include "video/VideoHttpBuffer.h"

#define DOWNLOADED_THRESHOLD 10

MplVideoPlayerBackend::~MplVideoPlayerBackend()
{
    clear();
}

MplVideoPlayerBackend::MplVideoPlayerBackend(QObject *parent)
    : VideoPlayerBackend(parent),
      m_videoBuffer(0), m_state(Stopped),
      m_errorMessage(QString()), m_playbackSpeed(1.0),
      m_mplayer(0), m_wid(QString()),
      m_playDuringDownload(false), m_pausedBySlowDownload(false)
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

void MplVideoPlayerBackend::checkDownloadAndPlayProgress(double position)
{
    if (!m_playDuringDownload)
        return;

    if (m_videoBuffer->isBufferingFinished())
    {
        m_playDuringDownload = false;
        m_pausedBySlowDownload = false;
        disconnect(m_mplayer, SIGNAL(currentPosition(double)), this, SLOT(checkDownloadAndPlayProgress(double)));

        return;
    }

    if (m_playDuringDownload && m_state == Playing)
    {
        if (m_videoBuffer->bufferedPercent() - qRound((position / m_mplayer->duration()) * 100) < DOWNLOADED_THRESHOLD)
        {
            pause();
            m_pausedBySlowDownload = true;
            qDebug() << "paused because download progress is slower than playback progress";
        }
    }

    if (m_pausedBySlowDownload && m_state == Paused)
    {
        if (m_videoBuffer->bufferedPercent() - qRound((position / m_mplayer->duration()) * 100) > DOWNLOADED_THRESHOLD)
        {
            m_pausedBySlowDownload = false;
            play();
            qDebug() << "continued playback after downloading portion of file";
        }
    }
}

void MplVideoPlayerBackend::playDuringDownloadTimerShot()
{
    if (m_videoBuffer->isBufferingFinished())
    {
        m_playDuringDownload = false;
        m_pausedBySlowDownload = false;
        return;
    }

    if (m_videoBuffer->bufferedPercent() > DOWNLOADED_THRESHOLD)
    {
        m_playDuringDownload = true;

        qDebug() << "started playback while download is in progress";
        connect(m_mplayer, SIGNAL(currentPosition(double)), this, SLOT(checkDownloadAndPlayProgress(double)));
        playIfReady();
    }
    else
        QTimer::singleShot(1000, this, SLOT(playDuringDownloadTimerShot()));
}

void MplVideoPlayerBackend::setWindowId(quint64 wid)
{
    m_wid = QString::number(wid);
}

void MplVideoPlayerBackend::createMplayerProcess()
{
    m_mplayer = new MplayerProcess(m_wid, this);
    connect(m_mplayer, SIGNAL(mplayerError(bool,QString)), this, SLOT(setError(bool,QString)));
    connect(m_mplayer, SIGNAL(eof()), this, SLOT(handleEof()));
    connect(m_mplayer, SIGNAL(readyToPlay()), this, SLOT(mplayerReady()));
    connect(m_mplayer, SIGNAL(durationChanged()), this, SLOT(durationIsKnown()));
    connect(m_mplayer, SIGNAL(currentPosition(double))
            ,this, SIGNAL(currentPosition(double)));
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
    {
        m_mplayer->disconnect();
        m_mplayer->deleteLater();
    }

    createMplayerProcess();


    /* Buffered HTTP source */
    setVideoBuffer(new VideoHttpBuffer(url));

    m_videoBuffer->startBuffering();

    QTimer::singleShot(1000, this, SLOT(playDuringDownloadTimerShot()));

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

    m_mplayer->setSpeed(m_playbackSpeed);
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
    if (m_pausedBySlowDownload)
    {
        play();
        return;
    }

    if (!m_mplayer || m_mplayer->isRunning() || !m_mplayer->start(m_videoBuffer->bufferFilePath()))
        return;
}

void MplVideoPlayerBackend::play()
{
    if (!m_mplayer || !m_mplayer->isRunning() || !m_mplayer->isReadyToPlay())
        return;

    if (m_pausedBySlowDownload)
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

    m_pausedBySlowDownload = false;
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

    createMplayerProcess();

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

void MplVideoPlayerBackend::queryPosition() const
{
    if (!m_mplayer || !m_mplayer->isRunning())
    {
        return;
    }

    m_mplayer->queryPosition();
}

void MplVideoPlayerBackend::setHardwareDecodingEnabled(bool enable)
{
    //implement later
}

bool MplVideoPlayerBackend::seek(int position)
{
    if (!m_mplayer || !m_mplayer->isRunning())
        return false;

    if (m_playDuringDownload && (position/duration()*100) > m_videoBuffer->bufferedPercent())
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



