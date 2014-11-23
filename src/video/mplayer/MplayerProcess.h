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

class MplayerProcess : public QObject
{
Q_OBJECT

public:
    MplayerProcess(QString &wid, QObject *parent = 0);
    ~MplayerProcess();

public slots:
    bool start();
    void play();
    void pause();
    void quit();
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

    void loadfile(QString file);

    void setProperty(QString prop, QString val);
    QString getProperty(QString prop);
    void sendCommand(QString cmd);

    bool isRunning();
signals:
    void mplayerError(bool permanent, const QString message);

private slots:
    void readAvailableStdout();
    void readAvailableStderr();
    void processError(QProcess::ProcessError error);

private:
    QProcess m_process;
    QString m_wid;
    bool m_expectData;
    bool m_loaded;

    void consumeStdOut();
};

#endif
