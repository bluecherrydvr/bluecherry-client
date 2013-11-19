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
#include "core/LiveViewManager.h"
#include "core/LiveStream.h"

class QNetworkReply;
class ThreadTask;
class ImageDecodeTask;

class MJpegStream : public LiveStream
{
    Q_OBJECT

public:

    explicit MJpegStream(DVRCamera *camera, QObject *parent = 0);
    virtual ~MJpegStream();

    QUrl url() const;

    int bandwidthMode() const { return m_bandwidthMode; }

    State state() const { return m_state; }
    QString errorMessage() const { return m_errorMessage; }

    QImage currentFrame() const { return m_currentFrame; }
    QSize streamSize() const { return m_currentFrame.size(); }

    float receivedFps() const { return m_receivedFps; }

    bool isPaused() const { return m_paused; }
    int interval() const { return m_interval; }

public slots:
    void start();
    void stop();

    void setPaused(bool paused);
    void togglePaused() { setPaused(!isPaused()); }
    void setOnline(bool online);
    void setBandwidthMode(int bandwidthMode);

private slots:
    void readable();
    void requestError();
    void checkActivity();

private:
    QWeakPointer<DVRCamera> m_camera;

    QString m_errorMessage;
    QNetworkReply *m_httpReply;
    QByteArray m_httpBoundary;
    QByteArray m_httpBuffer;
    QImage m_currentFrame;
    quint64 m_currentFrameNo, m_latestFrameNo;
    quint64 m_fpsRecvTs, m_fpsRecvNo;
    ImageDecodeTask *m_decodeTask;
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
    bool m_autoStart, m_paused;
    qint8 m_interval;
    LiveViewManager::BandwidthMode m_bandwidthMode;

    void setState(State newState);
    void setError(const QString &message);

    bool processHeaders();
    bool parseBuffer();
    void decodeFrame(const QByteArray &data);
    Q_INVOKABLE void decodeFrameResult(ThreadTask *task);
};

#endif // MJPEGSTREAM_H
