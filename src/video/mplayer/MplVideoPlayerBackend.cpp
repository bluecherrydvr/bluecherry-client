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

//EMIT ALL REQUIRED SIGNALS!!!
MplVideoPlayerBackend::~MplVideoPlayerBackend()
{
    clear();
}


MplVideoPlayerBackend::MplVideoPlayerBackend(QObject *parent)
    : VideoPlayerBackend(parent),
      m_videoBuffer(0), m_state(Stopped),
      m_playbackSpeed(1.0), m_mplayer(0)
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

    m_mplayer = new MplayerProcess(m_wid);
    connect(m_mplayer, SIGNAL(mplayerError(bool,QString)), this, SLOT(setError(bool,QString)));
    if (!m_mplayer->start())
        return false;


    /* Buffered HTTP source */
    setVideoBuffer(new VideoHttpBuffer(url));

    m_videoBuffer->startBuffering();

    /*
    m_mplayer->start("mplayer", QStringList() << "-slave" << "-idle"
                     << "-wid" << m_wid << "-quiet" << "-input"
                     << "nodefault-bindings:conf=/dev/null" << "-noconfig" << "all"
                     << "-nomouseinput" << "-zoom");

    if (!m_mplayer->waitForStarted())
    {
        QString errStr(tr("MPlayer process "));

        switch(m_mplayer->error())
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
    }*/

    m_playbackSpeed = 1.0;
    qDebug() << "MplVideoPlayerBackend::start mplayer process started\n";
    return true;
}

void MplVideoPlayerBackend::clear()
{
    /*
    if (m_mplayer && QProcess::NotRunning != m_mplayer->state())
    {
        m_mplayer->write("quit\n");
        if (!m_mplayer->waitForFinished(1000))
        {
            m_mplayer->kill();
        }

        m_mplayer->deleteLater();
        m_mplayer = 0;
    }*/

    if (m_mplayer)
    {
        m_mplayer->deleteLater();
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

bool MplVideoPlayerBackend::isSeekable() const
{
    return true;
}

void MplVideoPlayerBackend::playIfReady()
{
    if (!m_mplayer->isRunning())
        return;
    /*if (m_mplayer->state() != QProcess::Running)
        return;*/

    //m_mplayer->write((QString("pausing loadfile ") + m_videoBuffer->bufferFilePath() + "\n").toAscii().constData());
    m_mplayer->loadfile(m_videoBuffer->bufferFilePath());

    emit durationChanged(duration());

    play();
}

void MplVideoPlayerBackend::play()
{
    if (!m_mplayer->isRunning())
        return;
    /*if (m_mplayer->state() != QProcess::Running)
        return;*/

    //m_mplayer->write("pause\n");
    m_mplayer->play();

    emit playbackSpeedChanged(m_playbackSpeed);

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::pause()
{
    if (!m_mplayer->isRunning())
        return;
   /* if (m_mplayer->state() != QProcess::Running)
        return;*/

    //m_mplayer->write("pause\n");
    m_mplayer->pause();

    VideoState old = m_state;
    m_state = Paused;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::restart()
{
    if (!m_mplayer->isRunning())
        return;
    /*if (m_mplayer->state() != QProcess::Running)
        return;

    m_mplayer->write((QString("pausing loadfile ") + m_videoBuffer->bufferFilePath() + "\n").toAscii().constData());*/

    m_mplayer->loadfile(m_videoBuffer->bufferFilePath());

    VideoState old = m_state;
    m_state = Stopped;
    emit stateChanged(m_state, old);
}

void MplVideoPlayerBackend::mute(bool mute)
{
    if (!m_mplayer->isRunning())
        return;
    /*if (m_mplayer->state() != QProcess::Running)
        return;

    if (mute)
        m_mplayer->write("mute 0\n");
    else
        m_mplayer->write("mute 1\n");*/

    m_mplayer->mute(mute);
}

void MplVideoPlayerBackend::setVolume(double volume)
{
    if (!m_mplayer->isRunning())
        return;
    /*if (m_mplayer->state() != QProcess::Running)
        return;

    //mplayer volume range 0-100
    volume*=100.0;

    QString command = QString("set_property volume %1\n").arg(volume);

    m_mplayer->write(command.toAscii().constData());*/

    //mplayer volume range 0-100
    volume*=100.0;

    m_mplayer->setVolume(volume);
}

qint64 MplVideoPlayerBackend::duration() const
{
    if (!m_mplayer->isRunning())
        return -1;
    /*if (m_mplayer->state() != QProcess::Running)
        return -1;

    const_cast<MplVideoPlayerBackend*>(this)->m_mplayer->write("pausing get_property length\n");

    //result is in seconds, we return nanoseconds

    QRegExp rexp("ANS_length=([0-9\\.]+)");

    QByteArray reply = const_cast<MplVideoPlayerBackend*>(this)->m_mplayer->readAllStandardOutput();

    if (!QString::fromAscii(reply.constData()).contains(rexp))
    {
        return -1;
    }

    return (quint64)(rexp.cap(1).toDouble() * 1000000000.0);*/

    double secs = m_mplayer->duration();
    return secs < 0 ? -1 : secs  * 1000000000.0;
}

qint64 MplVideoPlayerBackend::position() const
{
    if (!m_mplayer->isRunning())
        return -1;
    /*
    if (m_mplayer->state() != QProcess::Running)
        return -1;

    const_cast<MplVideoPlayerBackend*>(this)->m_mplayer->write("pausing get_property time_pos\n");

    //result is in seconds, we return nanoseconds

    QRegExp rexp("ANS_time_pos=([0-9\\.]+)");

    QByteArray reply = const_cast<MplVideoPlayerBackend*>(this)->m_mplayer->readAllStandardOutput();

    if (!QString::fromAscii(reply.constData()).contains(rexp))
    {
        return -1;
    }

    return (quint64)(rexp.cap(1).toDouble() * 1000000000.0);*/

    double secs = m_mplayer->position();

    return secs > 0 ? secs * 1000000000.0 : -1;
}

void MplVideoPlayerBackend::setHardwareDecodingEnabled(bool enable)
{
    //implement later
}

bool MplVideoPlayerBackend::seek(qint64 position)
{
    if (!m_mplayer->isRunning())
        return false;
    /*if (m_mplayer->state() != QProcess::Running)
        return -1;

    if (state() != Playing && state() != Paused)
    {
        qDebug() << "Mplayer: Stream is not playing or paused, ignoring seek";
        return false;
    }

    QString command = QString("seek %1 2\n").arg((double)position/1000000000.0);

    if (state() == Paused)
        command = QString("pausing ") + command;

    m_mplayer->write(command.toAscii().constData());

    return true;*/

    return m_mplayer->seek((double)position/1000000000.0);
}

bool MplVideoPlayerBackend::setSpeed(double speed)
{
    if (!m_mplayer->isRunning())
        return false;
    /*if (m_mplayer->state() != QProcess::Running)
        return -1;*/

    if (speed == m_playbackSpeed)
        return true;

    if (speed == 0)
        return false;

    /*QString command = QString("set_property speed %1\n").arg(speed);

    if (state() == Paused)
        command = QString("pausing ") + command;

    m_mplayer->write(command.toAscii().constData());
    */
    m_mplayer->setSpeed(speed);

    m_playbackSpeed = speed;
    m_lastspeed = speed;
    emit playbackSpeedChanged(m_playbackSpeed);

    return speed;
}



