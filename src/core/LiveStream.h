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

signals:
    void stateChanged(int newState);
    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);
    void updated();

private slots:
    void updateFrame(const QImage &image);

private:
    DVRCamera camera;
    QThread *thread;
    LiveStreamWorker *worker;
    QImage m_currentFrame;
    QString m_errorMessage;
    State m_state;
    bool m_autoStart;

    void setState(State newState);
};

#endif // LIVESTREAM_H
