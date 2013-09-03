/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MJPEGSTREAM_H
#define MJPEGSTREAM_H

#include <QObject>
#include <QUrl>
#include <QPixmap>
#include <QTimer>
#include "camera/DVRCamera.h"

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
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
    Q_ENUMS(RecordingState)

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
    RecordingState recordingState() const { return m_recordingState; }

    float receivedFps() const { return m_receivedFps; }

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
    void recordingStateChanged(int state);

    void buildScaleSizes(QVector<QSize> &sizes);

    void updateFrame(const MJpegFrame &frame, const QVector<QImage> &scaledFrames);
    void bytesDownloaded(unsigned int bytes);

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
    quint64 m_fpsRecvTs, m_fpsRecvNo;
    ImageDecodeTask *m_decodeTask;
    QVector<QSize> m_scaleSizes;
    QTimer m_activityTimer;
    uint m_lastActivity;
    float m_receivedFps;

    int m_httpBodyLength;
    State m_state;
    enum {
        ParserBoundary,
        ParserHeaders,
        ParserBody
    } m_parserState;
    RecordingState m_recordingState;
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
