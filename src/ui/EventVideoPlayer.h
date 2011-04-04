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
class VideoContainer;
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

private slots:
    void stateChanged(int state);
    void durationChanged(qint64 duration = -1);
    void updatePosition();

    void videoContextMenu(const QPoint &pos);

    void queryLivePaused();
    void bufferingStarted();
    void updateBufferStatus();
    void bufferingStopped();

private:
    EventData *m_event;
    VideoPlayerBackend *m_video;
    VideoContainer *m_videoContainer;
    GstSinkWidget *m_videoWidget;
    QToolButton *m_playBtn, *m_restartBtn;
    QPushButton *m_saveBtn;
    QSlider *m_seekSlider;
    QLabel *m_posText, *m_statusText;
    QTimer m_posTimer;

    void setControlsEnabled(bool enabled);
};

#endif // EVENTVIDEOPLAYER_H
