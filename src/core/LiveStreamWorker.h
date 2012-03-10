#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include <QObject>
#include <QMutex>

struct AVFrame;

struct StreamFrame
{
    StreamFrame *next;
    AVFrame *d;

    StreamFrame() : next(0), d(0) { }
    ~StreamFrame();
};

class LiveStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamWorker(QObject *parent = 0);
    virtual ~LiveStreamWorker();

    void setUrl(const QByteArray &url);

public slots:
    void run();
    void stop();
    void setAutoDeinterlacing(bool enabled);

signals:
    void fatalError(const QString &message);

private:
    friend class LiveStream;

    struct AVFormatContext *ctx;
    struct SwsContext *sws;
    QByteArray url;
    bool cancelFlag;
    bool autoDeinterlacing;

    QMutex frameLock;
    StreamFrame *frameHead, *frameTail;

    bool setup();
    void destroy();

    void processVideo(struct AVStream *stream, struct AVFrame *frame);
};

#endif // LIVESTREAMWORKER_H
