#ifndef MJPEGSTREAM_H
#define MJPEGSTREAM_H

#include <QObject>
#include <QUrl>
#include <QPixmap>
#include <QTimer>

#if defined(Q_WS_MAC) || defined(Q_OS_WIN)
/* On mac, it is very expensive to convert between QImage and QPixmap, and
 * QImage is required when uploading textures to OpenGL. We can save significantly
 * by never using pixmaps in this case. */
typedef QImage MJpegFrame;
#else
#define MJPEGFRAME_IS_PIXMAP
typedef QPixmap MJpegFrame;
#endif

class QNetworkReply;
class ThreadTask;
class ImageDecodeTask;

class MJpegStream : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval RESET clearInterval NOTIFY intervalChanged)

public:
    enum State
    {
        Error = -3,
        StreamOffline = -2,
        Paused = -1,
        NotConnected,
        Connecting,
        Buffering,
        Streaming
    };

    explicit MJpegStream(QObject *parent = 0);
    explicit MJpegStream(const QUrl &url, QObject *parent = 0);
    virtual ~MJpegStream();

    QUrl url() const { return m_url; }
    void setUrl(const QUrl &url);

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QSize streamSize() const { return m_currentFrame.size(); }
    MJpegFrame currentFrame() const { return m_currentFrame; }

    bool isPaused() const { return m_paused; }
    int interval() const { return m_interval; }

public slots:
    void start();
    void stop();

    void setPaused(bool paused = true);
    void setInterval(int interval);
    void clearInterval() { setInterval(1); }

    void setOnline(bool online);

    void updateScaleSizes();

signals:
    void stateChanged(int newState);
    void streamRunning();
    void streamStopped();
    void streamSizeChanged(const QSize &size);

    void pausedChanged(bool isPaused);
    void intervalChanged(int interval);

    void buildScaleSizes(QVector<QSize> &sizes);

    void updateFrame(const MJpegFrame &frame, const QVector<QImage> &scaledFrames);

private slots:
    void readable();
    void requestError();
    void checkActivity();

private:
    QString m_errorMessage;
    QNetworkReply *m_httpReply;
    QByteArray m_httpBoundary;
    QByteArray m_httpBuffer;
    QUrl m_url;
    MJpegFrame m_currentFrame;
    quint64 m_currentFrameNo, m_latestFrameNo;
    ImageDecodeTask *m_decodeTask;
    QVector<QSize> m_scaleSizes;
    QTimer m_activityTimer;
    uint m_lastActivity;

    int m_httpBodyLength;
    State m_state;
    enum {
        ParserBoundary,
        ParserHeaders,
        ParserBody
    } m_parserState;
    bool m_autoStart, m_paused;
    qint8 m_interval;

    void setState(State newState);
    void setError(const QString &message);

    bool processHeaders();
    bool parseBuffer();
    void decodeFrame(const QByteArray &data);
    Q_INVOKABLE void decodeFrameResult(ThreadTask *task);
};

#endif // MJPEGSTREAM_H
