#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QElapsedTimer>
#include "DVRCamera.h"

class LiveStreamWorker;

class LiveStream : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

public:
    enum State
    {
        Error = -3,
        StreamOffline = -2,
        Paused = -1,
        NotConnected,
        Connecting,
        Streaming
    };

    static void init();
    explicit LiveStream(const DVRCamera &camera, QObject *parent = 0);
    virtual ~LiveStream();

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QImage currentFrame() const { return m_currentFrame; }
    QSize streamSize() const { return m_currentFrame.size(); }

    float receivedFps() const { return m_fps; }

public slots:
    void start();
    void stop();

    void setOnline(bool online);

    void setInterval(int interval) { }

signals:
    void stateChanged(int newState);
    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);
    void updated();

private slots:
    bool updateFrame();
    void fatalError(const QString &message);
    void updateSettings();

private:
    static QTimer *renderTimer;

    DVRCamera camera;
    QThread *thread;
    LiveStreamWorker *worker;
    QImage m_currentFrame;
    struct StreamFrame *m_frame;
    QString m_errorMessage;
    State m_state;
    bool m_autoStart;

    int m_fpsUpdateCnt;
    int m_fpsUpdateHits;
    float m_fps;

    qint64 m_ptsBase;
    QElapsedTimer m_ptsTimer;

    void setState(State newState);
};

#endif // LIVESTREAM_H
