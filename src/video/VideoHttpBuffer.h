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

#ifndef VIDEOHTTPBUFFER_H
#define VIDEOHTTPBUFFER_H

#include <QObject>
#include <QUrl>
#include "MediaDownload.h"
#include "video/VideoBuffer.h"

class QNetworkCookie;

class VideoHttpBuffer : public VideoBuffer
{
    Q_OBJECT

public:
    explicit VideoHttpBuffer(const QUrl &url, QObject *parent = 0);
    ~VideoHttpBuffer();

    virtual bool isBuffering() const { return m_media && !m_media->isFinished(); }
    virtual bool isBufferingFinished() const { return m_media && m_media->isFinished(); }
    virtual int bufferedPercent() const;

    virtual unsigned int totalBytes() const;
    virtual bool isEndOfStream() const;

    virtual QByteArray read(unsigned int bytes);
    virtual bool seek(unsigned int offset);

    qint64 bufferedSize() const { return m_media ? m_media->downloadedSize() : 0; }
    bool startBuffering();

    QUrl url() const { return m_url; }

private slots:
    void sendError(const QString &errorMessage);

private:
    QUrl m_url;
    MediaDownload *m_media;

};

inline int VideoHttpBuffer::bufferedPercent() const
{
    double file = totalBytes(), avail = bufferedSize();
    if (!file || !avail)
        return 0;
    return qMin(qRound((avail / file) * 100), 100);
}

#endif // VIDEOHTTPBUFFER_H
