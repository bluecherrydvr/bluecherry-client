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

#include <SDL2/SDL.h>


AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        qDebug() << "SDL initialization failed: " << SDL_GetError();
    }

    qDebug() << SDL_GetNumAudioDevices(0)  << " audio devices detected by SDL audio subsystem";
}

AudioPlayer::~AudioPlayer()
{
    SDL_Quit();
}

void AudioPlayer::play()
{

}

void AudioPlayer::stop()
{

}

void AudioPlayer::setSampleFormat()
{

}

void AudioPlayer::feedSamples(void *data, int samplesNum, int bytesNum)
{

}

