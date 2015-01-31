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

class QFrame;
class QToolButton;
class QPushButton;
class QSlider;
class QLabel;
class QThread;
class EventData;
class VideoPlayerBackend;
class VideoWidget;

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
    void zoomIn();
    void zoomOut();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void saveSnapshot(const QString &file = QString());

    void playPause();
    void seek(int position);
    void restart();
    void faster();
    void slower();

protected:
	virtual void changeEvent(QEvent *event);

private slots:
    void stateChanged(int state);
    void durationChanged(qint64 duration = -1);
    void playbackSpeedChanged(double playbackSpeed);
    void updatePosition();
    void videoNonFatalError(const QString &message);
    void streamsInitialized(bool hasAudioSupport);

    void videoContextMenu(const QPoint &pos);

    void queryLivePaused();
    void bufferingStarted();
    void updateBufferStatus();
    void bufferingStopped();

    void mute();
    void setVolume(int volume);

    void updateUI();
    void settingsChanged();

private:
    EventData *m_event;
    //QWeakPointer<QThread> m_videoThread;
    QWeakPointer<VideoPlayerBackend> m_videoBackend;
    VideoWidget *m_videoWidget;
    QToolButton *m_playBtn, *m_restartBtn, *m_fastBtn, *m_slowBtn, *m_muteBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_zoomInBtn;
    QPushButton *m_zoomOutBtn;
    QSlider *m_seekSlider;
    QLabel *m_startTime, *m_endTime, *m_statusText, *m_rateText;
    QSlider *m_volumeSlider;
    QTimer m_uiTimer;
    double m_lastspeed;
    double m_zoomFactor;

    void setControlsEnabled(bool enabled);
    bool uiRefreshNeeded() const;
	void retranslateUI();
    //void setZoom(double z);
};

#endif // EVENTVIDEOPLAYER_H
