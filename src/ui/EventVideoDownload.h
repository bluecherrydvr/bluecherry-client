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

#ifndef EVENTVIDEODOWNLOAD_H
#define EVENTVIDEODOWNLOAD_H

#include <QObject>
#include <QFutureWatcher>
#include <QTimer>

class QProgressDialog;
class MediaDownload;

class EventVideoDownload : public QObject
{
    Q_OBJECT

public:
    explicit EventVideoDownload(QObject *parent = 0);
    ~EventVideoDownload();

    void setMediaDownload(MediaDownload *download);
    void setFilePath(const QString &path);

    void start(QWidget *parentWindow = 0);

private slots:
    void startCopy();
    void copyFinished();
    void updateBufferProgress();
    void cancel();

private:
    QProgressDialog *m_dialog;
    QFutureWatcher<bool> *m_futureWatch;
    MediaDownload *m_media;
    QString m_tempFilePath;
    QString m_finalPath;
    QTimer m_progressTimer;
};

#endif // EVENTVIDEODOWNLOAD_H
