#ifndef VIDEOPLAYERBACKEND_GST_H
#define VIDEOPLAYERBACKEND_GST_H

#include <QObject>
#include <QWidget>
#include <gst/gst.h>

class QUrl;
class VideoHttpBuffer;

typedef struct _GstDecodeBin GstDecodeBin;

class VideoPlayerBackend : public QObject
{
    Q_OBJECT

public:
    enum VideoState
    {
        PermanentError = -2, /* Permanent errors; i.e., playback will not work even if restarted */
        Error = -1, /* Recoverable errors, generally by stopping and restarting the pipeline */
        Stopped,
        Playing,
        Paused,
        Done
    };

    explicit VideoPlayerBackend(QObject *parent = 0);
    ~VideoPlayerBackend();

    static bool initGStreamer(QString *errorMessage = 0);

    GstElement *sink() const { return m_sink; }
    /* setSink must be called exactly and only once prior to setting up the pipeline */
    void setSink(GstElement *sink);

    bool start(const QUrl &url);
    void clear();

    qint64 duration() const;
    qint64 position() const;
    bool isSeekable() const;
    bool atEnd() const { return m_state == Done; }
    VideoState state() const { return m_state; }
    bool isError() const { return m_state <= Error; }
    bool isPermanentError() const { return m_state == PermanentError; }
    QString errorMessage() const { return m_errorMessage; }
    VideoHttpBuffer *videoBuffer() const { return m_videoBuffer; }

public slots:
    void play();
    void pause();
    bool seek(qint64 position);
    void restart();

signals:
    void stateChanged(int newState, int oldState);
    void durationChanged(qint64 duration);
    void endOfStream();

private slots:
    void streamError(const QString &message);

private:
    GstElement *m_pipeline, *m_videoLink, *m_sink;
    VideoHttpBuffer *m_videoBuffer;
    VideoState m_state;
    QString m_errorMessage;
#ifdef Q_OS_LINUX
    unsigned int m_busWatchId;
#endif

    void setError(bool permanent, const QString &message);

    GstBusSyncReply busHandler(GstBus *bus, GstMessage *msg, bool isSynchronous);
    void decodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast);

    static GstBusSyncReply staticBusHandler(GstBus *bus, GstMessage *msg, gpointer data);
    static gboolean staticAsyncBusHandler(GstBus *bus, GstMessage *msg, gpointer data);
    static void staticDecodePadReady(GstDecodeBin *bin, GstPad *pad, gboolean islast, gpointer user_data);
};

#endif // VIDEOPLAYERBACKEND_GST_H
