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

#include "MplayerProcess.h"

MplayerProcess::MplayerProcess(QString &wid, QObject *parent)
    : QObject(parent),
      m_wid(wid), m_expectData(false), m_loaded(false)
{
    qDebug() << this << "\n";

    //connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readAvailableStdout()));
    connect(&m_process, SIGNAL(readyReadStandardError()), this, SLOT(readAvailableStderr()));
    connect(&m_process, SIGNAL(error()), this, SLOT(processError(QProcess::ProcessError)));
}

MplayerProcess::~MplayerProcess()
{
    qDebug() << "~MplayerProcess()\n";

    if (m_process.state() == QProcess::Running)
    {
        quit();
        if (!m_process.waitForFinished(1000))
        {
            m_process.kill();
        }
    }
}

bool MplayerProcess::start()
{
    qDebug() << this << "starting mplayer process...\n";

    m_process.start("mplayer", QStringList() << "-slave" << "-idle"
                    << "-wid" << m_wid << "-quiet" << "-input"
                    << "nodefault-bindings:conf=/dev/null" << "-noconfig" << "all"
                    << "-nomouseinput" << "-zoom" << "-nomsgcolor");

    if (!m_process.waitForStarted())
    {
        QString errStr(tr("MPlayer process "));

        switch(m_process.error())
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

    qDebug() << this << "mplayer process started\n";


    consumeStdOut();

    return true;
}

void MplayerProcess::consumeStdOut()
{
    int i = 0;

    while(m_process.canReadLine() && i < 10)
    {
        readAvailableStdout();
        m_process.waitForReadyRead(++i * 100);
    }
}

void MplayerProcess::readAvailableStderr()
{
    qDebug() << this << " stderr from mplayer:\n====\n"
             << m_process.readAllStandardError().constData()
             << "\n====\n";
}

void MplayerProcess::readAvailableStdout()
{
    if (m_expectData)
        return;

    qDebug() << this << " stdout from mplayer:\n====\n"
             << m_process.readAllStandardOutput().constData()
             << "\n====\n";

}

void MplayerProcess::sendCommand(QString cmd)
{
    if (!isRunning())
        return;

    m_process.write(QString("pausing_keep %1\n").arg(cmd).toAscii().constData());

    qDebug() << this << " sending command " << QString("pausing_keep %1\n").arg(cmd);
}

void MplayerProcess::quit()
{
    sendCommand("quit");
}

void MplayerProcess::setProperty(QString prop, QString val)
{
    sendCommand(QString("set_property %1 %2").arg(prop).arg(val));
}

QString MplayerProcess::getProperty(QString prop)
{
    if (!isRunning())
        return QString();

    m_expectData = true;

    QByteArray rbuf;
    QRegExp rexp(QString("ANS_%1=(\\S+)").arg(prop));

    sendCommand(QString("get_property %1").arg(prop));

    int i = 0;
    do
    {
        qDebug() << "getProperty iteration #" << i << "\b";
        rbuf = rbuf + m_process.readAllStandardOutput();

        qDebug() << "buffer content: " << rbuf.constData() << "\n";

        if (QString::fromAscii(rbuf.constData()).contains(rexp))
        {
            m_expectData = false;

            qDebug() << this << "returning property value " << rexp.cap(1) << "\n";
            return rexp.cap(1);
        }

        i++;
        m_process.waitForReadyRead(i * 10);
    }
    while(i < 12);

    //emit mplayerError(false, QString(tr("Failed to read property value from mplayer process!")));

    m_expectData = false;
    return QString();
}

void MplayerProcess::play()
{
    if (!isRunning())
        return;

    m_process.write("pause\n");
}

void MplayerProcess::pause()
{
    if (!isRunning())
        return;

    m_process.write("pause\n");
}

bool MplayerProcess::seek(double pos)
{
    if (!isRunning())
        return false;

    if (!m_loaded)
            return false;

    sendCommand(QString("seek %1 2").arg(pos, 0, 'f', 2));

    return true;
}

double MplayerProcess::duration()
{
    if (!isRunning())
        return -1;

    if (!m_loaded)
            return -1;

    QString ret = getProperty("length");

    if (ret.isEmpty())
        return -1;

    return ret.toDouble();
}

double MplayerProcess::position()
{
    if (!isRunning())
        return -1;

    if (!m_loaded)
            return -1;

    QString ret = getProperty("time_pos");

    if (ret.isEmpty())
        return -1;

    return ret.toDouble();
}

void MplayerProcess::setSpeed(double speed)
{
    if (!isRunning())
        return;

    setProperty("speed", QString::number(speed, 'f', 2));
}

void MplayerProcess::setVolume(double vol)
{
    if (!isRunning())
        return;

    setProperty("volume", QString::number(vol, 'f', 2));
}

void MplayerProcess::mute(bool yes)
{
    if (!isRunning())
        return;

    if (yes)
        setProperty("mute", "0");
    else
        setProperty("mute", "1");
}

void MplayerProcess::loadfile(QString file)
{
    qDebug() << this << "loading file " << file << "\n";

    if (!isRunning())
        return;

    m_process.write((QString("pausing loadfile ") + file + "\n").toAscii().constData());

    consumeStdOut();

    m_loaded = true;
}

bool MplayerProcess::isRunning()
{
    return m_process.state() == QProcess::Running;
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
        errStr.append(tr("- unable to start for unknown reason"));
    }

    qDebug() << this << errStr;

    emit mplayerError(true, errStr);
}
