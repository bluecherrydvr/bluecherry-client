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

#ifndef EVENTVIDEOPLAYER_H
#define EVENTVIDEOPLAYER_H

#include "bluecherry-config.h"
#include <QWidget>
#include <QUrl>
#include <QTimer>

#ifdef USE_GSTREAMER
#include "video/GstVideoPlayerBackend.h"
#endif

class QToolButton;
class QPushButton;
class QSlider;
class QLabel;
class GstSinkWidget;
class QThread;
class EventData;

class EventVideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit EventVideoPlayer(QWidget *parent = 0);
    ~EventVideoPlayer();

public slots:
    void setVideo(const QUrl &url, EventData *event = 0);
    void clearVideo();
    void saveVideo();
    void saveSnapshot(const QString &file = QString());

    void playPause();
    void seek(int position);
    void restart();
    void faster();
    void slower();

private slots:
    void stateChanged(int state);
    void durationChanged(qint64 duration = -1);
    void playbackSpeedChanged(double playbackSpeed);
    void updatePosition();
    void videoNonFatalError(const QString &message);

    void videoContextMenu(const QPoint &pos);

    void queryLivePaused();
    void bufferingStarted();
    void updateBufferStatus();
    void bufferingStopped();

    void updateUI();

private:
    EventData *m_event;
    QWeakPointer<QThread> m_videoThread;
    QWeakPointer<GstVideoPlayerBackend> m_video;
    GstSinkWidget *m_videoWidget;
    QToolButton *m_playBtn, *m_restartBtn, *m_fastBtn, *m_slowBtn;
    QPushButton *m_saveBtn;
    QSlider *m_seekSlider;
    QLabel *m_startTime, *m_endTime, *m_statusText, *m_rateText;
    QTimer m_uiTimer;

    void setControlsEnabled(bool enabled);
    bool uiRefreshNeeded() const;
};

#endif // EVENTVIDEOPLAYER_H
