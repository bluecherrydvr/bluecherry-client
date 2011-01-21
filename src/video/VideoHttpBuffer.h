#ifndef VIDEOHTTPBUFFER_H
#define VIDEOHTTPBUFFER_H

#include <QObject>
#include <QUrl>
#include <QTemporaryFile>
#include <QMutex>
#include <QWaitCondition>

class QNetworkReply;

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

    bool isBuffering() const { return m_networkReply; }

    QString bufferFileName() const { return m_bufferFile.fileName(); }
    qint64 fileSize() const { return m_fileSize; }
    qint64 bufferedSize() const { return m_bufferFile.pos(); }
    bool isBufferingFinished() const { return m_finished; }

    void setAutoDelete(bool enabled) { m_bufferFile.setAutoRemove(enabled); }

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
    void networkRead();
    void networkFinished();
    void networkMetaData();

    void cancelNetwork();

private:
    QTemporaryFile m_bufferFile;
    QNetworkReply *m_networkReply;
    qint64 m_fileSize, m_readPos, m_writePos;
    GstAppSrc *m_element;
    GstElement *m_pipeline;
    QMutex m_lock;
    QWaitCondition m_bufferWait;
    bool m_streamInit, m_bufferBlocked, m_finished;

    /* Rate estimation; circular buffer holding amounts for the last 64 buffer requests */
    static const int rateCount = 64;
    Q_PACKED struct
    {
        quint64 time;
        unsigned size;
    } rateData[rateCount];
    int ratePos, rateMax;

    void addRateData(quint64 time, unsigned size);
    void getRateEstimation(quint64 *duration, unsigned *size);

    static void needDataWrap(GstAppSrc *, unsigned, void*);
    static int seekDataWrap(GstAppSrc *, quint64, void*);

    void needData(unsigned size);
    bool seekData(qint64 offset);

    void sendStreamError(const QString &message);
};

#endif // VIDEOHTTPBUFFER_H
