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

#ifndef MPL_MPLAYER_PROCESS_H
#define MPL_MPLAYER_PROCESS_H

#include <QObject>
#include <QProcess>
#include <QMutex>

class MplayerProcess : public QObject
{
Q_OBJECT

public:
    MplayerProcess(QString &wid, QObject *parent = 0);
    ~MplayerProcess();

    //seek to pos second
    bool seek(double pos);

    //normal speed - 1.0
    void setSpeed(double speed);
    //volume range 0..100
    void setVolume(double vol);
    //true - disable sound
    void mute(bool yes);

    //returns duration in seconds
    double duration();
    double position();
    void queryPosition();

    void setProperty(QString prop, QString val);
    //QString getProperty(QString prop);
    void sendCommand(QString cmd);

    bool isRunning();
    bool isReadyToPlay() {return m_isreadytoplay;};

    bool saveScreenshot(QString &file);

public slots:
    bool start(QString filename);
    void play();
    void pause();
    void quit();

signals:
    void mplayerError(bool permanent, const QString message);
    void eof();
    void readyToPlay();
    void durationChanged();
    void currentPosition(double position);

private slots:
    void readAvailableStdout();
    void readAvailableStderr();
    void processError(QProcess::ProcessError error);
    void moveScreenshot();

private:
    QProcess *m_process;
    QString m_wid;
    //bool m_expectData;
    double m_duration;
    double m_position;

    bool m_isreadytoplay;
    bool m_durreqsent;
    bool m_ispaused;

    double m_speed;

    QString m_dstscreenshotfile;
    QString m_srcscreenshotfile;

    void checkEof(QByteArray &a);
    void checkPositionAnswer(QByteArray &a);
    void checkDurationAnswer(QByteArray &a);
    void checkPlayingMsgMagic(QByteArray &a);
    void checkScreenshot(QByteArray &a);
    void checkVOError(QByteArray &a);
};

#endif
