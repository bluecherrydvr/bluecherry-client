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

#ifndef EVENTVIDEODOWNLOADSWINDOW_H
#define EVENTVIDEODOWNLOADSWINDOW_H

#include <QMap>
#include <QFrame>

class QVBoxLayout;
class EventDownloadManager;
class EventVideoDownload;
class EventVideoDownloadWidget;

class EventVideoDownloadsWindow : public QFrame
{
    Q_OBJECT

public:
    explicit EventVideoDownloadsWindow(QWidget *parent = 0);
    virtual ~EventVideoDownloadsWindow();

    void setEventDownloadManager(EventDownloadManager *eventDownloadManager);

private slots:
    void eventVideoDownloadAdded(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadRemoved(EventVideoDownload *eventVideoDownload);

    void saveSettings();

private:
    QWeakPointer<EventDownloadManager> m_eventDownloadManager;
    QMap<EventVideoDownload *, EventVideoDownloadWidget *> m_downloadWidgets;
    QVBoxLayout *m_downloadLayout;

    void rebuildWidgets();
};

#endif // EVENTVIDEODOWNLOADSWINDOW_H
