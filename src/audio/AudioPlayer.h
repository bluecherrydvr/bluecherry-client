/*
 * Copyright 2010-2016 Bluecherry
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

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QElapsedTimer>

extern "C"
{
#include <libavutil/samplefmt.h>
}

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = 0);
    ~AudioPlayer();

public slots:
    void play();
    void stop();
    void setAudioFormat(enum AVSampleFormat fmt, int channelsNum, int sampleRate);
    void feedSamples(void *data, int samplesNum, int bytesNum);

private:

    bool m_isDeviceOpened;
    bool m_isPlaying;
    int m_deviceID;

    QElapsedTimer m_sampleClock;

    //static void SDL_AudioCallback(void*  userdata, quint8* stream, int len);



};

#endif

