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

#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QUrl>
#include <QMap>



class DVRServer;
class DVRServerRepository;
class EventData;
class QString;

struct ThumbnailData;

class ThumbnailManager : public QObject
{
    Q_OBJECT

public:

    enum Status
    {
        Unknown,
        Available,
        Loading,
        NotFound
    };

    explicit ThumbnailManager(QObject *parent = 0);
    ~ThumbnailManager();

    Status getThumbnail(const EventData *event, QString &imgPath);

private:

    QMap<QString,ThumbnailData*> m_thumbnails;
};

#endif
