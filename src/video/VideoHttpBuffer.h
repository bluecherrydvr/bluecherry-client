#ifndef VIDEOHTTPBUFFER_H
#define VIDEOHTTPBUFFER_H

#include <QObject>
#include <QUrl>
#include "MediaDownload.h"

typedef struct _GstAppSrc GstAppSrc;
typedef struct _GstElement GstElement;
class QNetworkCookie;

class VideoHttpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit VideoHttpBuffer(QObject *parent = 0);
    ~VideoHttpBuffer();

    /* Create and prepare a source element; the element will be added to the pipeline,
     * but not linked. */
    GstElement *setupSrcElement(GstElement *pipeline);

    MediaDownload *mediaDownload() const { return media; }

    bool isBuffering() const { return media && !media->isFinished(); }

    qint64 fileSize() const { return media ? media->fileSize() : 0; }
    qint64 bufferedSize() const { return media ? media->downloadedSize() : 0; }
    int bufferedPercent() const;
    bool isBufferingFinished() const { return media && media->isFinished(); }

    void setCookies(const QList<QNetworkCookie> &cookies);

public slots:
    bool start(const QUrl &url);
    void clearPlayback();

signals:
    void streamError(const QString &message);

    /* Emitted when buffering starts, i.e. upon start() */
    void bufferingStarted();
    /* Emitted when there is data ready in the buffer for the first time */
    void bufferingReady();
    /* Emitted when buffering stops for any reason, including errors */
    void bufferingStopped();
    /* Emitted when buffering is finished, and the entire file is cached locally */
    void bufferingFinished();

private slots:
    void fileSizeChanged(unsigned fileSize);
    void sendStreamError(const QString &message);

private:
    MediaDownload *media;
    GstAppSrc *m_element;
    GstElement *m_pipeline;
    QList<QNetworkCookie> m_cookies;

    static void needDataWrap(GstAppSrc *, unsigned, void*);
    static int seekDataWrap(GstAppSrc *, quint64, void*);

    void needData(int size);
    bool seekData(qint64 offset);
};

inline int VideoHttpBuffer::bufferedPercent() const
{
    double file = fileSize(), avail = bufferedSize();
    if (!file || !avail)
        return 0;
    return qMin(qRound((avail / file) * 100), 100);
}

#endif // VIDEOHTTPBUFFER_H
