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

#include "ThumbnailManager.h"
#include "network/MediaDownloadManager.h"
#include "video/MediaDownload.h"
#include "core/EventData.h"
#include "core/BluecherryApp.h"
#include "utils/FileUtils.h"

#include <QFile>
#include <QString>
#include <QUrl>
#include <QDir>

#include <QDebug>

struct ThumbnailData
{
    QString fileName;
    MediaDownload *md;
    ThumbnailManager::Status status;
};

ThumbnailManager::ThumbnailManager(QObject *parent)
    : QObject(parent)
{

}

ThumbnailManager::~ThumbnailManager()
{
    //clear cache
    //qDebug() << "~ThumbnailManager()";

    QMap<QString, ThumbnailData*>::const_iterator i = m_thumbnails.constBegin();
    while (i != m_thumbnails.constEnd())
    {
        if (i.value()->status == Available)
        {
            //qDebug() << "removing thumbnail file " << thumbnailFilePath(i.key());
            QFile::remove(thumbnailFilePath(i.key()));
        }

        delete i.value();

        ++i;
    }
}

QString ThumbnailManager::thumbnailFilePath(const QString &keyStr)
{
    return QDir::tempPath() + '/' + QLatin1String("bc_tmb_") + keyStr;
}

ThumbnailManager::Status ThumbnailManager::getThumbnail(const EventData *event, QString &imgPath)
{
    QString keyStr;
    Status result = Unknown;

    keyStr = sanitizeFilename(QString::fromLatin1("%1.%2.%3")
                              .arg(event->uiServer())
                              .arg(event->uiLocation())
                              .arg(event->mediaId()));

    //qDebug() << "searching for thumbnail " << keyStr;

    if (m_thumbnails.contains(keyStr))
    {
        ThumbnailData* td = m_thumbnails.value(keyStr);

        switch (td->status)
        {
        case Loading:

            if (td->md->isFinished())
            {
                if (td->md->fileSize() < 50) //handle case when error string is returned instead of picture by server version <=2.7.4
                {
                    td->status = NotFound;
                    bcApp->mediaDownloadManager()->releaseMediaDownload(td->md->url());
                    break;
                }
                //copy file, resize
                QFile::copy(td->md->bufferFilePath(), thumbnailFilePath(keyStr));
                td->status = Available;
                bcApp->mediaDownloadManager()->releaseMediaDownload(td->md->url());
            }

            if (td->md->hasError())
            {
                //qDebug() << "failed to download thumbnail" << keyStr;
                td->status = NotFound;
                bcApp->mediaDownloadManager()->releaseMediaDownload(td->md->url());
            }

            break;

        case Available:
            break;

        case NotFound:
        case Unknown:
        default:
            break;
        }

        result = td->status;
    }
    else
    {
        ThumbnailData* td = new ThumbnailData;
        QUrl url = event->server()->url().resolved(QUrl(QLatin1String("/media/request")));
        url.addQueryItem(QLatin1String("id"), QString::number(event->mediaId()));
        url.addQueryItem(QLatin1String("mode"), QLatin1String("screenshot"));

        //qDebug() << "requesting thumbnail using URL " << url.toString();

        td->status = Loading;
        td->md = bcApp->mediaDownloadManager()->acquireMediaDownload(url);
        td->fileName = keyStr;

        td->md->start();

        m_thumbnails[keyStr] = td;

        result = td->status;
    }

    if (result == Available)
        imgPath = thumbnailFilePath(keyStr);

    return result;
}
