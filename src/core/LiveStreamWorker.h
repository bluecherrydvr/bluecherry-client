#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include <QObject>
#include <QAtomicPointer>

struct AVFrame;

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

    QAtomicPointer<struct AVFrame> videoFrame;

    bool setup();
    void destroy();

    void processVideo(struct AVStream *stream, struct AVFrame *frame);
};

#endif // LIVESTREAMWORKER_H
