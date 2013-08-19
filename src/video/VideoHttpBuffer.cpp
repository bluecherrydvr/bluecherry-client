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

VideoHttpBuffer::VideoHttpBuffer(const QUrl &url, QObject *parent) :
        VideoBuffer(parent), m_url(url), m_media(0)
{
}

VideoHttpBuffer::~VideoHttpBuffer()
{
    if (m_media)
    {
        bcApp->mediaDownloadManager()->releaseMediaDownload(m_url);
        m_media = 0;
    }
}

void VideoHttpBuffer::startBuffering()
{
    Q_ASSERT(!m_media);

    m_media = bcApp->mediaDownloadManager()->acquireMediaDownload(m_url);
    connect(m_media, SIGNAL(fileSizeChanged(uint)), this, SIGNAL(totalBytesChanged(uint)));
    connect(m_media, SIGNAL(finished()), SIGNAL(bufferingFinished()));
    connect(m_media, SIGNAL(stopped()), SIGNAL(bufferingStopped()));
    connect(m_media, SIGNAL(error(QString)), SLOT(sendError(QString)));

    m_media->start();

    qDebug("VideoHttpBuffer: started");
    emit bufferingStarted();
}

bool VideoHttpBuffer::isBuffering() const
{
    return m_media && !m_media->isFinished();
}

bool VideoHttpBuffer::isBufferingFinished() const
{
    return m_media && m_media->isFinished();
}

int VideoHttpBuffer::bufferedPercent() const
{
    if (!m_media)
        return 0;

    unsigned int file = totalBytes();
    qint64 avail = m_media->downloadedSize();
    if (!file || !avail)
        return 0;
    return qMin(qRound(((float)avail / file) * 100), 100);
}

unsigned int VideoHttpBuffer::totalBytes() const
{
    return m_media ? m_media->fileSize() : 0;
}

bool VideoHttpBuffer::isEndOfStream() const
{
    return m_media ? (m_media->readPosition() >= m_media->fileSize() && m_media->isFinished()) : false;
}

QByteArray VideoHttpBuffer::read(unsigned int bytes)
{
    return m_media ? m_media->read(m_media->readPosition(), bytes) : QByteArray();
}

bool VideoHttpBuffer::seek(unsigned int offset)
{
    Q_ASSERT(m_media);

    return m_media->seek(offset);
}

void VideoHttpBuffer::sendError(const QString &errorMessage)
{
    emit error(errorMessage);
    emit bufferingStopped();
}
