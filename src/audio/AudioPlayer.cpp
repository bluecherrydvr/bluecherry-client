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

#include "AudioPlayer.h"

#include <QDebug>

#if defined(__APPLE__)

#if defined(MAC_OS_X_VERSION_MIN_REQUIRED)
#undef MAC_OS_X_VERSION_MIN_REQUIRED
#endif

#define MAC_OS_X_VERSION_MIN_REQUIRED 1050

#endif
#include <SDL2/SDL.h>

extern "C"
{
#include <libavutil/samplefmt.h>
}


AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent),
      m_isDeviceOpened(false), m_isPlaying(false),
      m_deviceID(0)
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        qDebug() << "SDL initialization failed: " << SDL_GetError();
    }

    qDebug() << SDL_GetNumAudioDevices(0)  << " audio devices detected by SDL audio subsystem";

    setAudioFormat(AV_SAMPLE_FMT_S16, 2, 44100);
}

AudioPlayer::~AudioPlayer()
{
    stop();
    SDL_Quit();
}

/*
void AudioPlayer::SDL_AudioCallback(void*  userdata, quint8 *stream, int len)
{

}
*/

void AudioPlayer::play()
{
    if (!m_isDeviceOpened)
        return;

    SDL_PauseAudioDevice(m_deviceID, 0);
    m_isPlaying = true;

    m_sampleClock.start();
}

void AudioPlayer::stop()
{
    if (m_isDeviceOpened)
    {
        SDL_CloseAudioDevice(m_deviceID);
        m_isDeviceOpened = false;
        m_isPlaying = false;
    }
}

void AudioPlayer::setAudioFormat(enum AVSampleFormat fmt, int channelsNum, int sampleRate)
{
    qDebug() << "AudioPlayer: setting sample format to " << av_get_sample_fmt_name(fmt)
             << " channels: " << channelsNum << "sample rate: " << sampleRate;

    AudioPlayer::stop();

    SDL_AudioSpec spec;
    SDL_AudioFormat sdlFmt;

    SDL_memset(&spec, 0, sizeof(spec));

    spec.freq = sampleRate;
    spec.channels = channelsNum;
    spec.samples = 8192;
    spec.callback = NULL;

    switch(fmt)
    {
    case AV_SAMPLE_FMT_U8:
        sdlFmt = AUDIO_U8;
        break;

    case AV_SAMPLE_FMT_S16:
        sdlFmt = AUDIO_S16SYS;
        break;

    case AV_SAMPLE_FMT_S32:
        sdlFmt = AUDIO_S32SYS;
        break;

    case AV_SAMPLE_FMT_FLT:
        sdlFmt = AUDIO_F32SYS;
        break;

    case AV_SAMPLE_FMT_FLTP:
        /*
        Planar formats are not supported by SDL (2.0.4 and older),
        so we use only single plane and play one channel as a workaround
        */
        sdlFmt = AUDIO_F32SYS;
        spec.channels = 1;
        break;

    default:
        qDebug() << "AudioPlayer: sample format " << av_get_sample_fmt_name(fmt) << " is not supported by SDL";
        return;
    }

    spec.format = sdlFmt;

    m_deviceID = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    if (m_deviceID == 0)
    {
        qDebug() << "AudioPlayer: failed to open audio device - " << SDL_GetError();
        return;
    }

    m_isDeviceOpened = true;

}

void AudioPlayer::feedSamples(void *data, int samplesNum, int bytesNum)
{
    if (!m_isDeviceOpened)
        return;

    //qDebug() << "AudioPlayer: got " << samplesNum << " samples, " << bytesNum << " bytes, time elapsed: " << m_sampleClock.elapsed() << "ms";


    int ret = SDL_QueueAudio(m_deviceID, data, bytesNum);

    if (ret < 0)
    {
        qDebug() << "AudioPlayer: SDL_QueueAudio() failed - " << SDL_GetError();
    }

}

