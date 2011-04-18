#ifndef VIDEOHTTPBUFFER_H
#define VIDEOHTTPBUFFER_H

#include <QObject>
#include <QUrl>
#include "MediaDownload.h"

typedef struct _GstAppSrc GstAppSrc;
typedef struct _GstElement GstElement;

class VideoHttpBuffer : public QObject
{
    Q_OBJECT

public:
    explicit VideoHttpBuffer(QObject *parent = 0);
    ~VideoHttpBuffer();

    /* Create and prepare a source element; the element will be added to the pipeline,
     * but not linked. */
    GstElement *setupSrcElement(GstElement *pipeline);

    bool isBuffering() const { return media && !media->isFinished(); }

    //QString bufferFileName() const { return m_bufferFile.fileName(); }
    qint64 fileSize() const { return media ? media->fileSize() : 0; }
    //qint64 bufferedSize() const { return m_bufferFile.pos(); }
    bool isBufferingFinished() const { return media && media->isFinished(); }

    //void setAutoDelete(bool enabled) { m_bufferFile.setAutoRemove(enabled); }

public slots:
    bool start(const QUrl &url);
    void clearPlayback();

signals:
    void streamError(const QString &message);

    /* Emitted when buffering starts, i.e. upon start() */
    void bufferingStarted();
    /* Emitted when buffering stops for any reason, including errors */
    void bufferingStopped();
    /* Emitted when buffering is finished, and the entire file is cached locally */
    void bufferingFinished();
    /* Emitted when new data has been added to the buffer */
    void bufferUpdated();

private slots:
    void fileSizeChanged(unsigned fileSize);

private:
    MediaDownload *media;
    GstAppSrc *m_element;
    GstElement *m_pipeline;

    static void needDataWrap(GstAppSrc *, unsigned, void*);
    static int seekDataWrap(GstAppSrc *, quint64, void*);

    void needData(int size);
    bool seekData(qint64 offset);

    void sendStreamError(const QString &message);
};

#endif // VIDEOHTTPBUFFER_H
