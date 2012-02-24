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

public:
    explicit LiveStream(const DVRCamera &camera, QObject *parent = 0);

    static void init();

    QImage currentFrame() const { return m_currentFrame; }

private slots:
    void updateFrame(const QImage &image);

signals:
    void updated();

private:
    DVRCamera camera;
    QThread *thread;
    LiveStreamWorker *worker;
    QImage m_currentFrame;
};

#endif // LIVESTREAM_H
