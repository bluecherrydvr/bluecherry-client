#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QElapsedTimer>
#include "DVRCamera.h"
#include "LiveViewManager.h"

class LiveStreamWorker;

class LiveStream : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(int bandwidthMode READ bandwidthMode WRITE setBandwidthMode NOTIFY bandwidthModeChanged)

public:
    enum State
    {
        Error = -3,
        StreamOffline = -2,
        NotConnected,
        Connecting,
        Streaming,
        Paused
    };

    static void init();
    explicit LiveStream(const DVRCamera &camera, QObject *parent = 0);
    virtual ~LiveStream();

    QByteArray url() const;

    int bandwidthMode() const { return m_bandwidthMode; }

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QImage currentFrame() const { return m_currentFrame; }
    QSize streamSize() const { return m_currentFrame.size(); }

    float receivedFps() const { return m_fps; }

    bool isPaused() { return state() == Paused; }

public slots:
    void start();
    void stop();

    void setPaused(bool paused);
    void setOnline(bool online);
    void setBandwidthMode(int bandwidthMode);

signals:
    void stateChanged(int newState);
    void pausedChanged(bool paused);
    void bandwidthModeChanged(int mode);

    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);
    void updated();

private slots:
    bool updateFrame();
    void fatalError(const QString &message);
    void updateSettings();
    void checkState();

private:
    static QTimer *renderTimer, *stateTimer;

    DVRCamera camera;
    QThread *thread;
    LiveStreamWorker *worker;
    QImage m_currentFrame;
    struct StreamFrame *m_frame;
    QString m_errorMessage;
    State m_state;
    bool m_autoStart;
    LiveViewManager::BandwidthMode m_bandwidthMode;

    int m_fpsUpdateCnt;
    int m_fpsUpdateHits;
    float m_fps;

    qint64 m_ptsBase;
    QElapsedTimer m_ptsTimer;
    QElapsedTimer m_frameInterval;

    void setState(State newState);
};

#endif // LIVESTREAM_H
