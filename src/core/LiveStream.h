#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QObject>
#include <QThread>
#include <QImage>
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

public slots:
    void start();
    void stop();

    void setOnline(bool online);

    void setInterval(int interval) { }

    /* Update the frame from the decoding thread; returns true if redraw is needed */
    bool updateFrame();

signals:
    void stateChanged(int newState);
    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);
    void updated();

private:
    static QTimer *renderTimer;

    DVRCamera camera;
    QThread *thread;
    LiveStreamWorker *worker;
    QImage m_currentFrame;
    struct AVFrame *m_frameData;
    QString m_errorMessage;
    State m_state;
    bool m_autoStart;

    void setState(State newState);
};

#endif // LIVESTREAM_H
