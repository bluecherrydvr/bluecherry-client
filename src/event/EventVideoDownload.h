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

#include <QFutureWatcher>
#include <QTimer>
#include <QUrl>

class QProgressDialog;
class MediaDownload;

class EventVideoDownload : public QObject
{
    Q_OBJECT

public:
    explicit EventVideoDownload(const QUrl &fromUrl, const QString &toFilePath, QObject *parent = 0);
    ~EventVideoDownload();

    void start(QWidget *parentWindow = 0);
    void stop();

private slots:
    void startCopy();
    void copyFinished();
    void updateBufferProgress();
    void cancel();

private:
    QUrl m_fromUrl;
    QString m_finalPath;
    QProgressDialog *m_dialog;
    QFutureWatcher<bool> *m_futureWatch;
    MediaDownload *m_media;
    QString m_tempFilePath;
    QTimer m_progressTimer;
};

#endif // EVENTVIDEODOWNLOAD_H
