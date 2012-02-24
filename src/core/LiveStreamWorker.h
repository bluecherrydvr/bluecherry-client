#ifndef LIVESTREAMWORKER_H
#define LIVESTREAMWORKER_H

#include <QObject>
#include <QImage>

class LiveStreamWorker : public QObject
{
    Q_OBJECT

public:
    explicit LiveStreamWorker(QObject *parent = 0);

    void setUrl(const QByteArray &url);

public slots:
    void run();
    void stop();

signals:
    void frame(const QImage &image);

private:
    struct AVFormatContext *ctx;
    struct SwsContext *sws;
    QByteArray url;
    bool cancelFlag;

    bool setup();
    void destroy();

    void processVideo(struct AVStream *stream, struct AVFrame *frame);
};

#endif // LIVESTREAMWORKER_H
