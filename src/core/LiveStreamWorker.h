#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include <QObject>
#include <QImage>

class LiveStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamWorker(QObject *parent = 0);

public slots:
    void run();

signals:
    void frame(const QImage &image);

private:
    struct AVFormatContext *ctx;
    struct SwsContext *sws;

    bool setup();
    void destroy();

    void processVideo(struct AVStream *stream, struct AVFrame *frame);
};

#endif // LIVESTREAMWORKER_H
