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
#include <QThread>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QCoreApplication>
#include <QSettings>
#include "MplayerProcess.h"

#define MPL_PLAYMSGMAGIC "XPL32AFFC3DFEBB"

MplayerProcess::MplayerProcess(QString &wid, QObject *parent)
    : QObject(parent),
      m_wid(wid), m_process(0),
      m_duration(-1), m_position(-1),
      m_isreadytoplay(false),
      m_durreqsent(false),
      m_ispaused(false),
      m_speed(1.0)

{
    //qDebug() << this << "\n";

    Q_ASSERT(QThread::currentThread() == qApp->thread());

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readAvailableStdout()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readAvailableStderr()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
}

MplayerProcess::~MplayerProcess()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    //qDebug() << "~MplayerProcess()\n";

    if (m_process->state() == QProcess::Running)
    {
        quit();
        if (!m_process->waitForFinished(1000))
        {
            m_process->kill();
        }
    }
    delete m_process;
    m_process = 0;
}

bool MplayerProcess::saveScreenshot(QString &file)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!(isRunning() && m_isreadytoplay))
        return false;

    if (file.isEmpty())
        return false;

    //qDebug() << "MplayerProcess::saveScreenshot() " << file << "\n";

    m_dstscreenshotfile = file;

    sendCommand("screenshot 0");

    return true;
}

bool MplayerProcess::start(QString filename)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    m_process->setWorkingDirectory(QDir::tempPath());

    //qDebug() << this << "starting mplayer process, opening file " << filename << " ...\n";
#ifndef Q_OS_MAC
    QSettings settings;
    QString vo = settings.value(QLatin1String("eventPlayer/mplayer_vo"), QLatin1String("default")).toString();

    if (vo == QLatin1String("default"))
#ifdef Q_OS_WIN
        vo = QLatin1String("direct3d,directx:noaccel,");
#else
        vo = QLatin1String(" ,");
#endif

#ifdef Q_OS_LINUX
    m_process->start("bc-mplayer",
#else
    m_process->start("mplayer",
#endif

#else
    m_process->start(QCoreApplication::applicationDirPath() + QDir::separator() + "mplayer",
#endif
                    QStringList()
                    << "-slave"
#ifndef Q_OS_MAC
                    << "-wid" << m_wid
#endif
                    << "-quiet" << "-input"
                    << "nodefault-bindings:conf=/dev/null" << "-noconfig" << "all"
                    << "-playing-msg" << MPL_PLAYMSGMAGIC"\n"
                    << "-nomouseinput" << "-zoom" << "-nomsgcolor"
                    << "-vf" << "screenshot=bluecherryscrnshot"
                    << "-cache" << "512"
#ifdef WIN32
                    << "-nofs"
                    << "-priority" << "abovenormal"
                    << "-nodr"
                    << "-double"
                    << "-noslices"
                    << "-osdlevel" << "0"
                    << "-colorkey" << "0x020202"
                    //<< "-nokeepaspect"
#endif
#ifdef Q_OS_MAC
                    << "-vo" << "corevideo:shared_buffer:buffer_name=bceventmplayer" + m_wid
#else
                    << "-vo" << vo
#endif
                    << filename);

#ifdef Q_OS_MAC
    qDebug() << "mplayer started with \"" << "corevideo:shared_buffer:buffer_name=bceventmplayer" + m_wid << "\"\n";
#endif

    if (!m_process->waitForStarted())
    {
        QString errStr(tr("MPlayer process "));

        switch(m_process->error())
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

        emit mplayerError(true, errStr);
        return false;
    }

    //qDebug() << this << "mplayer process started\n";

    return true;
}

void MplayerProcess::checkScreenshot(QByteArray &a)
{
    if (m_dstscreenshotfile.isEmpty())
        return;

    QRegExp rx_screenshot("^\\*\\*\\* screenshot '(.*)'");

    if (QString::fromAscii(a.constData()).contains(rx_screenshot))
    {
        QString s = rx_screenshot.cap(1);

        m_srcscreenshotfile = m_process->workingDirectory() + '/' + s;

        moveScreenshot();
    }
}

void MplayerProcess::moveScreenshot()
{
    if (!QFile::exists(m_srcscreenshotfile))
    {
        QTimer::singleShot(300, this, SLOT(moveScreenshot()));
        return;
    }

    if (!QFile::copy(m_srcscreenshotfile, m_dstscreenshotfile))
        emit mplayerError(false,
                          tr("failed to copy screenshot file from %1 to %2").
                          arg(m_srcscreenshotfile).arg(m_dstscreenshotfile));
        //qDebug() << "failed to copy screenshot file from " << m_srcscreenshotfile << " to " << m_dstscreenshotfile << "\n";

    m_dstscreenshotfile.clear();
    QFile::remove(m_srcscreenshotfile);
    m_srcscreenshotfile.clear();
}

void MplayerProcess::checkEof(QByteArray &a)
{
    QRegExp rx_endoffile("^Exiting... \\(End of file\\)|^ID_EXIT=EOF");

    if (QString::fromAscii(a.constData()).contains(rx_endoffile))
        emit eof();
}



void MplayerProcess::checkPlayingMsgMagic(QByteArray &a)
{
    if (m_isreadytoplay)
        return;

    if (a.contains(MPL_PLAYMSGMAGIC))
    {
        m_isreadytoplay = true;
        emit readyToPlay();
        duration();
        setSpeed(m_speed);
    }
}

void MplayerProcess::checkPositionAnswer(QByteArray &a)
{
    QRegExp rexp(QString("ANS_time_pos=(\\S+)"));

    if (QString::fromAscii(a.constData()).contains(rexp))
    {
        m_position = rexp.cap(1).toDouble();
        emit currentPosition(m_position);
    }
}

void MplayerProcess::checkDurationAnswer(QByteArray &a)
{
    if (!m_durreqsent)
        return;

    QRegExp rexp(QString("ANS_length=(\\S+)"));

    if (QString::fromAscii(a.constData()).contains(rexp))
    {
        double old_duration = m_duration;
        m_duration = rexp.cap(1).toDouble();

        if (old_duration != m_duration)
            emit durationChanged();

        m_durreqsent = false;
    }
}

void MplayerProcess::checkVOError(QByteArray &a)
{
    QRegExp rx_voerror(QString("^Error opening/initializing the selected video_out \\(\\-vo\\) device"));

    if (QString::fromAscii(a.constData()).contains(rx_voerror))
    {
#ifndef Q_OS_MAC
        QSettings settings;
        QString vo = settings.value(QLatin1String("eventPlayer/mplayer_vo"), QLatin1String("default")).toString();

        emit mplayerError(true,
                          tr("Failed to initialize video output '%1'. Please choose another video output driver for MPlayer in the program options.").arg(vo));
#else
        emit mplayerError(true, rx_voerror.cap());
#endif
    }
}

void MplayerProcess::readAvailableStderr()
{
    m_process->setReadChannel(QProcess::StandardError);

    while(m_process->canReadLine())
    {
        QByteArray l = m_process->readLine();
        checkVOError(l);

        qDebug() << "MPLAYER STDERR:" << l;
    }
}

void MplayerProcess::readAvailableStdout()
{
    m_process->setReadChannel(QProcess::StandardOutput);

    while(m_process->canReadLine())
    {
        QByteArray l = m_process->readLine();
        checkEof(l);
        checkPlayingMsgMagic(l);
        checkDurationAnswer(l);
        checkPositionAnswer(l);
        checkScreenshot(l);
        //qDebug() << "MPLAYER STDOUT:" << l << "\n";
    }
}

void MplayerProcess::sendCommand(QString cmd)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!isRunning())
        return;

    m_process->write(QString("%1 %2\n").arg(m_ispaused ? "pausing_keep" : "").arg(cmd).toAscii().constData());//

    //qDebug() << this << " sending command " << QString("pausing_keep %1\n").arg(cmd);
}

void MplayerProcess::quit()
{
    sendCommand("quit");
}

void MplayerProcess::setProperty(QString prop, QString val)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    sendCommand(QString("set_property %1 %2").arg(prop).arg(val));
}
/*
QString MplayerProcess::getProperty(QString prop)
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    QString ret;

    if (!isRunning())
        return ret;

    //m_expectData = true;

    QRegExp rexp(QString("ANS_%1=(\\S+)").arg(prop));

    m_process->disconnect(SIGNAL(readyReadStandardOutput()));
    m_process->readAllStandardOutput();

    sendCommand(QString("get_property %1").arg(prop));

    m_process->waitForReadyRead(100);

    while(m_process->canReadLine())
    {
          QByteArray line = m_process->readLine(1024);

          if (line.contains("ANS_ERROR=PROPERTY_UNAVAILABLE"))
                break;

          checkEof(line);

          if (QString::fromAscii(line.constData()).contains(rexp))
                {
                    ret = rexp.cap(1);
                    break;
                }
    }

    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readAvailableStdout()));

    return ret;
}*/

void MplayerProcess::play()
{
    if (!(isRunning() && m_isreadytoplay))
        return;

    //qDebug() << "MplayerProcess::play()\n";

    m_process->write("pause\n");
    m_ispaused = false;

    if (m_speed != 1.0)
        setProperty("speed", QString::number(m_speed, 'f', 2));
}

void MplayerProcess::pause()
{
    if (!(isRunning() && m_isreadytoplay))
        return;

    //qDebug() << "MplayerProcess::pause()\n";

    m_process->write("pause\n");
    m_ispaused = true;
}

bool MplayerProcess::seek(double pos)
{
    if (!(isRunning() && m_isreadytoplay))
        return false;

    sendCommand(QString("seek %1 2").arg(pos, 0, 'f', 2));

    return true;
}

double MplayerProcess::duration()
{
    if (!(isRunning() && m_isreadytoplay))
        return -1;

    if (!m_durreqsent)
    {
        m_durreqsent = true;
        sendCommand(QString("get_property length"));
    }

    return m_duration;
}

double MplayerProcess::position()
{
    if (!(isRunning() && m_isreadytoplay))
        return -1;

    sendCommand(QString("get_property time_pos"));

    return m_position;
}

void MplayerProcess::queryPosition()
{
    if (!(isRunning() && m_isreadytoplay))
    {
        return;
    }

    sendCommand(QString("get_property time_pos"));
}

void MplayerProcess::setSpeed(double speed)
{
    m_speed = speed;

    if (!(isRunning() && m_isreadytoplay))
        return;

    setProperty("speed", QString::number(speed, 'f', 2));
}

void MplayerProcess::setVolume(double vol)
{
    if (!(isRunning() && m_isreadytoplay))
        return;

    setProperty("volume", QString::number(vol, 'f', 2));
}

void MplayerProcess::mute(bool yes)
{
    if (!(isRunning() && m_isreadytoplay))
        return;

    if (yes)
        setProperty("mute", "1");
    else
        setProperty("mute", "0");
}


bool MplayerProcess::isRunning()
{
    if (!m_process)
        return false;

    return m_process->state() == QProcess::Running;
}

void MplayerProcess::processError(QProcess::ProcessError error)
{
    QString errStr("Received mplayer process error:");
    switch(error)
    {
    case QProcess::FailedToStart:
        errStr.append(tr("failed to start"));
    break;

    case QProcess::Crashed:
        errStr.append(tr("crashed"));
    break;

    default:
        return;
    }

    //qDebug() << this << errStr;

    emit mplayerError(true, errStr);
}
