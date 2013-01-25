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

typedef struct _GstAppSrc GstAppSrc;
typedef struct _GstElement GstElement;
class QNetworkCookie;

class VideoHttpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit VideoHttpBuffer(QObject *parent = 0);
    ~VideoHttpBuffer();

    /* Create and prepare a source element; the element will be added to the pipeline,
     * but not linked. */
    GstElement *setupSrcElement(GstElement *pipeline);

    MediaDownload *mediaDownload() const { return media; }

    bool isBuffering() const { return media && !media->isFinished(); }

    qint64 fileSize() const { return media ? media->fileSize() : 0; }
    qint64 bufferedSize() const { return media ? media->downloadedSize() : 0; }
    int bufferedPercent() const;
    bool isBufferingFinished() const { return media && media->isFinished(); }

    void setCookies(const QList<QNetworkCookie> &cookies);

public slots:
    bool startBuffering(const QUrl &url);
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
    MediaDownload *media;
    GstAppSrc *m_element;
    GstElement *m_pipeline;
    QList<QNetworkCookie> m_cookies;

    static void needDataWrap(GstAppSrc *, unsigned, void*);
    static int seekDataWrap(GstAppSrc *, quint64, void*);

    void needData(int size);
    bool seekData(qint64 offset);
};

inline int VideoHttpBuffer::bufferedPercent() const
{
    double file = fileSize(), avail = bufferedSize();
    if (!file || !avail)
        return 0;
    return qMin(qRound((avail / file) * 100), 100);
}

#endif // VIDEOHTTPBUFFER_H
