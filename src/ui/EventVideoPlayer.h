#ifndef EVENTVIDEOPLAYER_H
#define EVENTVIDEOPLAYER_H

#include <QWidget>
#include <QUrl>
#include <QTimer>

#ifdef USE_GSTREAMER
#include "video/VideoPlayerBackend_gst.h"
#endif

class QToolButton;
class QPushButton;
class QSlider;
class QLabel;
class GstSinkWidget;
class QThread;
struct EventData;

class EventVideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit EventVideoPlayer(QWidget *parent = 0);
    ~EventVideoPlayer();

public slots:
    void setVideo(const QUrl &url, EventData *event = 0);
    void clearVideo();
    void saveVideo(const QString &path = QString());
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
    QThread *m_videoThread;
    VideoPlayerBackend *m_video;
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
