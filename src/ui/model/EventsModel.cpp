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

#include "EventsModel.h"
#include "core/EventData.h"
#include "core/ServerRequestManager.h"
#include "core/BluecherryApp.h"
#include "server/DVRServerRepository.h"
#include "event/ThumbnailManager.h"
#include <QDebug>
#include <QIcon>
#include <QTextDocument>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>

EventsModel::EventsModel(DVRServerRepository *serverRepository, QObject *parent)
    : QAbstractItemModel(parent), m_serverRepository(serverRepository)
{
    Q_ASSERT(m_serverRepository);

    connect(m_serverRepository, SIGNAL(serverAdded(DVRServer*)), SLOT(serverAdded(DVRServer*)));
    connect(m_serverRepository, SIGNAL(serverAboutToBeRemoved(DVRServer*)), SLOT(clearServerEvents(DVRServer*)));

    foreach (DVRServer *s, m_serverRepository->servers())
        serverAdded(s);
}

void EventsModel::serverAdded(DVRServer *server)
{
    connect(server, SIGNAL(disconnected(DVRServer*)), SLOT(clearServerEvents(DVRServer*)));
}

int EventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

int EventsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return LastColumn+1;
}

QModelIndex EventsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || column < 0 || row >= m_items.size() || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column, m_items[row].data());
}

QModelIndex EventsModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EventData *data = reinterpret_cast<EventData*>(index.internalPointer());
    if (!data)
        return QVariant();

    if (role == EventDataPtr)
    {
        return QVariant::fromValue(data);
    }
    else if (role == Qt::ToolTipRole)
    {
        QString imgPath;
        QString imgString;
        ThumbnailManager::Status imgStatus;
        QSettings settings;
        int tmbWidth;

        if (settings.value(QLatin1String("ui/enableThumbnails"), true).toBool())
        {
            imgStatus = bcApp->thumbnailManager()->getThumbnail(data, imgPath);

            switch(imgStatus)
            {
            case ThumbnailManager::Available:

                tmbWidth = QApplication::desktop()->screenGeometry().width() / 4;

                imgString = QString::fromLatin1("<img width=\"%2\" src=\"%1\">").arg(imgPath).arg(tmbWidth);//calculate size based on screen resolution
                break;

            case ThumbnailManager::Loading:
                imgString = tr("Loading thumbnail...");
                break;

            case ThumbnailManager::NotFound:
                imgString = tr("Thumbnail is not available");
                break;

            case ThumbnailManager::Unknown:
                break;
            }
        }

        return tr("%1 (%2)<br>%3 on %4<br>%5<br>%6").arg(data->uiType(), data->uiLevel(), Qt::escape(data->uiLocation()),
                                                   Qt::escape(data->uiServer()), data->localStartDate().toString(), imgString);
    }
    else if (role == Qt::ForegroundRole)
    {
        return data->uiColor(false);
    }

    switch (index.column())
    {
    case ServerColumn:
        if (role == Qt::DisplayRole)
        {
            if (data->server())
                return data->server()->configuration().displayName();
            else
                return QString();
        }
        break;
    case LocationColumn:
        if (role == Qt::DisplayRole)
            return data->uiLocation();
        break;
    case TypeColumn:
        if (role == Qt::DisplayRole)
            return data->uiType();
        else if (role == Qt::DecorationRole)
            return data->hasMedia() ? QIcon(QLatin1String(":/icons/control-000-small.png")) : QVariant();
        break;
    case DurationColumn:
        if (role == Qt::DisplayRole)
            return data->uiDuration();
        else if (role == Qt::EditRole)
            return data->durationInSeconds();
        else if (role == Qt::FontRole && data->inProgress())
        {
            QFont f;
            f.setBold(true);
            return f;
        }
        break;
    case LevelColumn:
        if (role == Qt::DisplayRole)
            return data->uiLevel();
        else if (role == Qt::EditRole)
            return data->level().level;
        break;
    case DateColumn:
        if (role == Qt::DisplayRole)
            return data->localStartDate().toString();
        else if (role == Qt::EditRole)
            return data->localStartDate();
        break;
    }

    return QVariant();
}

QVariant EventsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case ServerColumn: return tr("Server");
    case LocationColumn: return tr("Device");
    case TypeColumn: return tr("Event");
    case DurationColumn: return tr("Duration");
    case LevelColumn: return tr("Priority");
    case DateColumn: return tr("Date");
    }

    return QVariant();
}

void EventsModel::computeBoundaries()
{
    int begin = 0;
    m_serverEventsBoundaries.clear();
    foreach (DVRServer *server, m_serverRepository->servers())
    {
        int count = !m_serverEventsCount.contains(server) ? 0 : m_serverEventsCount.value(server);
        m_serverEventsBoundaries.insert(server, qMakePair(begin, begin + count - 1));
        begin += count;
    }
}

void EventsModel::setServerEvents(DVRServer *server, const QList<QSharedPointer<EventData> > &events)
{
    clearServerEvents(server);
    computeBoundaries();

    int insertedRowBegin = m_serverEventsBoundaries.value(server).first;
    int insertedRowEnd = insertedRowBegin + events.count() - 1;

    if (insertedRowEnd >= insertedRowBegin)
    {
        beginInsertRows(QModelIndex(), insertedRowBegin, insertedRowEnd);
        m_items = m_items.mid(0, insertedRowBegin) + events + m_items.mid(insertedRowBegin);
        endInsertRows();
    }

    m_serverEventsCount.insert(server, events.count());
}

void EventsModel::clearServerEvents(DVRServer *server)
{
    computeBoundaries();

    int removedRowBegin = m_serverEventsBoundaries.value(server).first;
    int removedRowEnd = m_serverEventsBoundaries.value(server).second;

    if (removedRowEnd >= removedRowBegin)
    {
        beginRemoveRows(QModelIndex(), removedRowBegin, removedRowEnd);
        //QList<QSharedPointer<EventData> > removedEvents = m_items.mid(removedRowBegin, removedRowEnd - removedRowBegin + 1);
        //qDeleteAll(removedEvents.begin(), removedEvents.end());
        m_items = m_items.mid(0, removedRowBegin) + m_items.mid(removedRowEnd + 1);
        endRemoveRows();
    }

    m_serverEventsCount.remove(server);
}
