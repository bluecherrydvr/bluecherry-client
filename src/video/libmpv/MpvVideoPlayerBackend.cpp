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

#include "MpvVideoPlayerBackend.h"
#include "video/VideoHttpBuffer.h"
#include "mpv/qthelper.hpp"

#define DOWNLOADED_THRESHOLD 10

static void wakeup(void *ptr)
{
    MpvVideoPlayerBackend *backend = (MpvVideoPlayerBackend*) ptr;
    backend->emitEvents();
}

MpvVideoPlayerBackend::~MpvVideoPlayerBackend()
{
    clear();
}

MpvVideoPlayerBackend::MpvVideoPlayerBackend(QObject *parent)
    : VideoPlayerBackend(parent),
      m_videoBuffer(0), m_state(Stopped),
      m_errorMessage(QString()), m_playbackSpeed(1.0),
      m_mpv(0), m_id(0),
      m_playDuringDownload(false), m_pausedBySlowDownload(false),
      m_duration(-1), m_position(-1)
{
#ifndef Q_OS_WIN
    std::setlocale(LC_NUMERIC, "C");
#endif

    qDebug() << "MpvVideoPlayerBackend() this =" << this << "\n";
}

void MpvVideoPlayerBackend::setVideoBuffer(VideoHttpBuffer *videoHttpBuffer)
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

void MpvVideoPlayerBackend::checkDownloadAndPlayProgress()
{
    if (!m_playDuringDownload)
        return;

    if (m_videoBuffer->isBufferingFinished())
    {
        m_playDuringDownload = false;
        m_pausedBySlowDownload = false;
        disconnect(this, SIGNAL(currentPosition(double)), this, SLOT(checkDownloadAndPlayProgress()));

        return;
    }

    if (m_playDuringDownload && m_state == Playing)
    {
        if (m_videoBuffer->bufferedPercent() - qRound((m_position / m_duration) * 100) < DOWNLOADED_THRESHOLD)
        {
            pause();
            m_pausedBySlowDownload = true;
            qDebug() << "paused because download progress is slower than playback progress";
        }
    }

    if (m_pausedBySlowDownload && m_state == Paused)
    {
        if (m_videoBuffer->bufferedPercent() - qRound((m_position / m_duration) * 100) > DOWNLOADED_THRESHOLD)
        {
            m_pausedBySlowDownload = false;
            play();
            qDebug() << "continued playback after downloading portion of file";
        }
        else
            QTimer::singleShot(1000, this, SLOT(checkDownloadAndPlayProgress()));
    }
}

void MpvVideoPlayerBackend::playDuringDownloadTimerShot()
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
        connect(this, SIGNAL(currentPosition(double)), this, SLOT(checkDownloadAndPlayProgress()));
        playIfReady();
    }
    else
        QTimer::singleShot(1000, this, SLOT(playDuringDownloadTimerShot()));
}

void MpvVideoPlayerBackend::setWindowId(quint64 wid)
{
    m_id = wid;
}

bool MpvVideoPlayerBackend::createMpvProcess()
{
    m_mpv = mpv_create();

    if (!m_mpv)
    {
        qDebug() << "MpvVideoPlayerBackend: Can't create mpv instance!\n";
        return false;
    }

    mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &m_id);

    mpv_set_option_string(m_mpv, "input-default-bindings", "yes");
    mpv_set_option_string(m_mpv, "input-vo-keyboard", "yes");
    mpv_set_option_string(m_mpv, "input-cursor", "no");
    mpv_set_option_string(m_mpv, "cursor-autohide", "no");
    mpv_set_option_string(m_mpv, "hr-seek", "yes");

    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);

    mpv_request_log_messages(m_mpv, "error");  // no fatal error warn info v debug trace

    connect(this, SIGNAL(mpvEvents()), this, SLOT(receiveMpvEvents()), Qt::QueuedConnection);
    mpv_set_wakeup_callback(m_mpv, wakeup, this);

    if (mpv_initialize(m_mpv) < 0)
    {
        qDebug() << "MpvVideoPlayerBackend: mpv failed to initialize!\n";
        return false;
    }

    return true;
}

bool MpvVideoPlayerBackend::start(const QUrl &url)
{
    qDebug() << "MpvVideoPlayerBackend::start url =" << url << "\n";
    if (state() == PermanentError)
        return false;

    if (!m_id)
    {
        setError(false, tr("Window id is not set"));
        return false;
    }

    if (m_mpv)
    {
        int pause = 1;
        mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);
        mpv_terminate_destroy(m_mpv);
        m_mpv = NULL;
    }

    if (!createMpvProcess())
    {
        qDebug() << "MpvVideoPlayerBackend: start - failed to create MPV \n";
        return false;
    }

    /* Buffered HTTP source */
    setVideoBuffer(new VideoHttpBuffer(url));

    m_videoBuffer->startBuffering();

    QTimer::singleShot(1000, this, SLOT(playDuringDownloadTimerShot()));

    m_playbackSpeed = 1.0;
    qDebug() << "MpvVideoPlayerBackend:: MPV process started\n";
    return true;
}

void MpvVideoPlayerBackend::clear()
{
    if (m_mpv)
        mpv_terminate_destroy(m_mpv);

    m_mpv = NULL;

    setVideoBuffer(0);

    m_state = Stopped;
    m_errorMessage.clear();
}

void MpvVideoPlayerBackend::setError(bool permanent, const QString message)
{
    VideoState old = m_state;
    m_state = permanent ? PermanentError : Error;
    m_errorMessage = message;
    emit stateChanged(m_state, old);
}

void MpvVideoPlayerBackend::streamError(const QString &message)
{
    qDebug() << "MpvVideoPlayerBackend: stopping stream due to error:" << message;

    setError(true, message);
}

bool MpvVideoPlayerBackend::saveScreenshot(QString &file)
{
    if (!m_mpv)
        return false;

    const char *cmd[] = { "screenshot-to-file", file.toUtf8(), "video", NULL };
    mpv_command(m_mpv, cmd);

    return true;
}


void MpvVideoPlayerBackend::handleEof()
{
    qDebug() << "EOF\n";
    VideoState old = m_state;
    m_state = Done;
    emit stateChanged(m_state, old);
    emit endOfStream();
}

void MpvVideoPlayerBackend::mpvPlayerReady()
{
    qDebug() << this << "MPV is ready to play\n";

    emit streamsInitialized(true);

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);

    setSpeed(m_playbackSpeed);
}

void MpvVideoPlayerBackend::durationIsKnown()
{
    emit durationChanged(duration());
}

bool MpvVideoPlayerBackend::isSeekable() const
{
    return true;
}

void MpvVideoPlayerBackend::playIfReady()
{
    if (m_pausedBySlowDownload)
    {
        play();
        return;
    }

    if (!m_mpv || m_state == Playing ||
            m_state == Forward || m_state == Backward)
        return;

    const QByteArray filename = m_videoBuffer->bufferFilePath().toUtf8();
    const char *args[] = { "loadfile", filename.data(), NULL };
    mpv_command(m_mpv, args);

    mpvPlayerReady();

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MpvVideoPlayerBackend::play()
{
    if (!m_mpv || m_state == Playing)
        return;

    if (m_pausedBySlowDownload)
    {
        if (m_videoBuffer->bufferedPercent() - qRound((m_position / m_duration) * 100) > DOWNLOADED_THRESHOLD)
            m_pausedBySlowDownload = false;
        else
            return;
    }

    int pause = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);

    emit playbackSpeedChanged(m_playbackSpeed);

    VideoState old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MpvVideoPlayerBackend::pause()
{
    if (!m_mpv)
        return;

    m_pausedBySlowDownload = false;

    int pause = 1;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);

    VideoState old = m_state;
    m_state = Paused;
    emit stateChanged(m_state, old);
}

void MpvVideoPlayerBackend::restart()
{
    if (!m_mpv)
        return;

    const char *arg[] = { "stop", NULL };
    mpv_command(m_mpv, arg);

    VideoState old = m_state;
    m_state = Stopped;
    emit stateChanged(m_state, old);

    const QByteArray filename = m_videoBuffer->bufferFilePath().toUtf8();
    const char *args[] = { "loadfile", filename.data(), NULL };
    mpv_command(m_mpv, args);

    old = m_state;
    m_state = Playing;
    emit stateChanged(m_state, old);
}

void MpvVideoPlayerBackend::playForward()
{
    if (m_state != Forward)
    {
        VideoState old = m_state;
        m_state = Forward;
        emit stateChanged(m_state, old);
    }

    if (m_position >= m_duration)
    {
        pause();
        return;
    }

    const char *cmd[] = { "frame-step", NULL };

    if (mpv_command(m_mpv, cmd) < 0)
        qDebug() << "MpvVideoPlayerBackend: frame-step error";
}

void MpvVideoPlayerBackend::playBackward()
{
    if (m_state != Backward)
    {
        VideoState old = m_state;
        m_state = Backward;
        emit stateChanged(m_state, old);
    }

    if (m_position <= 0.0)
    {
        pause();
        return;
    }

    const char *cmd[] = { "frame-back-step", NULL };

    if (mpv_command(m_mpv, cmd) < 0)
        qDebug() << "MpvVideoPlayerBackend: frame-back-step error";
}

void MpvVideoPlayerBackend::mute(bool mute)
{
    if (!m_mpv)
        return;

    int mut = (mute ? 1 : 0);
    mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &mut);
}

void MpvVideoPlayerBackend::setVolume(double volume)
{
    if (!m_mpv)
        return;

    double vol = volume * 100.0;
    mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
}

int MpvVideoPlayerBackend::duration() const
{
    if (!m_mpv)
        return -1;

    return m_duration < 0 ? -1 : m_duration  * 1000.0;
}

int MpvVideoPlayerBackend::position() const
{
    if (!m_mpv)
        return -1;

    return m_position > 0 ? m_position * 1000.0 : -1;
}

void MpvVideoPlayerBackend::setHardwareDecodingEnabled(bool enable)
{
    //implement later
}

bool MpvVideoPlayerBackend::seek(int position)
{
    if (!m_mpv)
        return false;

    if (m_playDuringDownload &&
            m_videoBuffer->bufferedPercent() - qRound(position * 100 / duration()) < DOWNLOADED_THRESHOLD + 1)
        return false;

    char num[32];
    double pos = double (position);
    pos /= 1000;
    sprintf(num, "%.3f", pos);

    const char *cmd[] = { "seek", num, "absolute", NULL };
    mpv_command(m_mpv, cmd);

    return true;
}

bool MpvVideoPlayerBackend::setSpeed(double speed)
{
    if (!m_mpv)
        return false;

    if (speed == m_playbackSpeed)
        return true;

    mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &speed);

    m_playbackSpeed = speed;
    m_lastspeed = speed;
    emit playbackSpeedChanged(m_playbackSpeed);

    return speed;
}

void MpvVideoPlayerBackend::setBrightness(int brightness)
{
    if (!m_mpv)
        return;

    int64_t bri = (brightness - 8) * 5;

    mpv_set_property(m_mpv, "brightness", MPV_FORMAT_INT64, &bri);
}

void MpvVideoPlayerBackend::setContrast(int contrast)
{
    if (!m_mpv)
        return;

    int64_t con = (contrast - 8) * 8;

    mpv_set_property(m_mpv, "contrast", MPV_FORMAT_INT64, &con);
}

void MpvVideoPlayerBackend::setColor(int balance)
{
    if (!m_mpv)
        return;

    int64_t bal = (balance - 8) * 8;

    mpv_set_property(m_mpv, "saturation", MPV_FORMAT_INT64, &bal);
}

void MpvVideoPlayerBackend::queryPosition()
{
    emit currentPosition(m_position);
}

void MpvVideoPlayerBackend::emitEvents()
{
    if (m_mpv)
        emit mpvEvents();
}

void MpvVideoPlayerBackend::receiveMpvEvents()
{
    // This slot is invoked by wakeup() (through the mpv_events signal).
    while (m_mpv)
    {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handleMpvEvent(event);
    }
}

void MpvVideoPlayerBackend::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id)
    {
        case MPV_EVENT_PROPERTY_CHANGE:
        {
            mpv_event_property *prop = (mpv_event_property *)event->data;
            if (strcmp(prop->name, "time-pos") == 0)
            {
                if (prop->format == MPV_FORMAT_DOUBLE)
                {
                    m_position = *(double *)prop->data;

                    if (m_state == Backward)
                        playBackward();
                    else if (m_state == Forward)
                        playForward();

                    emit currentPosition(m_position);
                }
            }
            else if (strcmp(prop->name, "duration") == 0)
            {
                if (prop->format == MPV_FORMAT_DOUBLE)
                {
                    double duration = *(double *)prop->data;
                    if (m_duration != duration)
                    {
                        m_duration = duration;
                        durationIsKnown();
                    }
                }
            }

            break;
        }
        case MPV_EVENT_SEEK:
        {
            break;
        }
        case MPV_EVENT_END_FILE:
        {
            mpv_event_end_file *eef = (mpv_event_end_file*) event->data;
            switch (eef->reason)
            {
                case MPV_END_FILE_REASON_EOF:
                {
                    handleEof();
                    break;
                }
                case MPV_END_FILE_REASON_STOP:
                case MPV_END_FILE_REASON_QUIT:
                case MPV_END_FILE_REASON_ERROR:
                case MPV_END_FILE_REASON_REDIRECT:
                default: m_position = -1;
            }

            break;
        }
        case MPV_EVENT_LOG_MESSAGE:
        {
            mpv_event_log_message *msg = (mpv_event_log_message*) event->data;

            qDebug() << "MpvVideoPlayerBackend: [" << msg->prefix << "] "
                     << msg->level << ": " << msg->text;
            break;
        }
        case MPV_EVENT_SHUTDOWN:
        {
            qDebug() << "MpvVideoPlayerBackend: MPV shutdown event";

            mpv_terminate_destroy(m_mpv);
            m_mpv = NULL;
            break;
        }
        default: ; // Ignore uninteresting or unknown events.
    }
}

