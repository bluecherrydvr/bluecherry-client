#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include <QObject>
#include <QMutex>

struct AVFrame;

struct StreamFrame
{
    StreamFrame *next;
    AVFrame *d;

    void free();
};

class LiveStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamWorker(QObject *parent = 0);

    void setUrl(const QByteArray &url);

    AVFrame *takeFrame();

public slots:
    void run();
    void stop();

signals:
    void fatalError(const QString &message);

private:
    struct AVFormatContext *ctx;
    struct SwsContext *sws;
    QByteArray url;
    bool cancelFlag;

    public:
    QAtomicPointer<struct AVFrame> videoFrame;
    QMutex frameLock;
    StreamFrame *frameHead, *frameTail;
    private:

    bool setup();
    void destroy();

    void processVideo(struct AVStream *stream, struct AVFrame *frame);
};

#endif // LIVESTREAMWORKER_H
