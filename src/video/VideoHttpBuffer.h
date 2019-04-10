/*
 * Copyright 2010-2019 Bluecherry, LLC
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

class QNetworkCookie;

class VideoHttpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit VideoHttpBuffer(const QUrl &url, QObject *parent = 0);
    ~VideoHttpBuffer();


    bool isBuffering() const { return media && !media->isFinished(); }

    qint64 fileSize() const { return media ? media->fileSize() : 0; }
    qint64 bufferedSize() const { return media ? media->downloadedSize() : 0; }
    int bufferedPercent() const;
    bool isBufferingFinished() const { return media && media->isFinished(); }
    bool startBuffering();

    QUrl url() const { return m_url; }

    QString bufferFilePath() const;

public slots:
    void clearPlayback();

signals:
    void streamError(const QString &message);

    /* Emitted when buffering starts, i.e. upon start() */
    void bufferingStarted();
    /* Emitted when there is data ready in the buffer for the first time */
    void bufferingReady();
    /* Emitted when buffering stops for any reason, including errors */
    void bufferingStopped();
    /* Emitted when buffering is finished, and the entire file is cached locally */
    void bufferingFinished();

private slots:
    void fileSizeChanged(unsigned fileSize);
    void sendStreamError(const QString &message);

private:
    QUrl m_url;
    MediaDownload *media;
};

inline int VideoHttpBuffer::bufferedPercent() const
{
    double file = fileSize(), avail = bufferedSize();
    if (!file || !avail)
        return 0;
    return qMin(qRound((avail / file) * 100), 100);
}

#endif // VIDEOHTTPBUFFER_H
