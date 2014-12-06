/*
 * Copyright 2010-2013 Bluecherry
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

#include "VideoHttpBuffer.h"
#include "core/BluecherryApp.h"
#include "network/MediaDownloadManager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QApplication>


VideoHttpBuffer::VideoHttpBuffer(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url), media(0)
{
}

VideoHttpBuffer::~VideoHttpBuffer()
{
    clearPlayback();
    if (media)
    {
        bcApp->mediaDownloadManager()->releaseMediaDownload(m_url);
        media = 0;
    }
}

bool VideoHttpBuffer::startBuffering()
{
    Q_ASSERT(!media);

    media = bcApp->mediaDownloadManager()->acquireMediaDownload(m_url);
    connect(media, SIGNAL(fileSizeChanged(uint)), SLOT(fileSizeChanged(uint)), Qt::DirectConnection);
    connect(media, SIGNAL(finished()), SIGNAL(bufferingFinished()));
    connect(media, SIGNAL(stopped()), SIGNAL(bufferingStopped()));
    connect(media, SIGNAL(error(QString)), SLOT(sendStreamError(QString)));

    media->start();

    qDebug("VideoHttpBuffer: started");
    emit bufferingStarted();

    return true;
}

void VideoHttpBuffer::clearPlayback()
{

}

void VideoHttpBuffer::sendStreamError(const QString &message)
{
    emit streamError(message);
    emit bufferingStopped();
}

void VideoHttpBuffer::fileSizeChanged(unsigned fileSize)
{
    if (!fileSize)
        qDebug() << "VideoHttpBuffer: fileSize is 0, may cause problems!";

}


QString VideoHttpBuffer::bufferFilePath() const
{
    return media->bufferFilePath();
}
