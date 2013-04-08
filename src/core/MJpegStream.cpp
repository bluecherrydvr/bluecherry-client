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

#include "BluecherryApp.h"
#include "MJpegStream.h"
#include "LiveViewManager.h"
#include "utils/ImageDecodeTask.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QImage>
#include <QThreadPool>
#include <QTimer>
#include <QDateTime>

MJpegStream::MJpegStream(QObject *parent)
    : QObject(parent), m_httpReply(0), m_currentFrameNo(0), m_latestFrameNo(0), m_fpsRecvTs(0), m_fpsRecvNo(0),
      m_decodeTask(0), m_lastActivity(0), m_receivedFps(0), m_httpBodyLength(0), m_state(NotConnected),
      m_parserState(ParserBoundary), m_recordingState(NoRecording), m_autoStart(false),
      m_paused(false), m_interval(1)
{
    //bcApp->liveView->addStream(this);
    connect(&m_activityTimer, SIGNAL(timeout()), SLOT(checkActivity()));
}

MJpegStream::MJpegStream(const QUrl &url, QObject *parent)
    : QObject(parent), m_httpReply(0), m_currentFrameNo(0), m_latestFrameNo(0), m_fpsRecvTs(0), m_fpsRecvNo(0),
      m_decodeTask(0), m_lastActivity(0), m_receivedFps(0), m_httpBodyLength(0), m_state(NotConnected),
      m_parserState(ParserBoundary), m_recordingState(NoRecording), m_autoStart(false),
      m_paused(false), m_interval(1)
{
    //bcApp->liveView->addStream(this);
    setUrl(url);
    connect(&m_activityTimer, SIGNAL(timeout()), SLOT(checkActivity()));
}

MJpegStream::~MJpegStream()
{
    if (m_recordingState != NoRecording)
    {
        m_recordingState = NoRecording;
        emit recordingStateChanged(m_recordingState);
    }

    //bcApp->liveView->removeStream(this);

    if (m_httpReply)
        m_httpReply->deleteLater();
}

void MJpegStream::setState(State newState)
{
    if (m_state == newState)
        return;

    State oldState = m_state;
    m_state = newState;

    if (m_state != Error)
        m_errorMessage.clear();

    emit stateChanged(newState);

    if (newState >= Buffering && oldState < Buffering)
    {
        emit streamRunning();
        updateScaleSizes();
    }
    else if (oldState >= Buffering && newState < Buffering)
    {
        emit streamStopped();
        if (m_recordingState != NoRecording)
        {
            m_recordingState = NoRecording;
            emit recordingStateChanged(m_recordingState);
        }
    }
}

void MJpegStream::setError(const QString &message)
{
    m_errorMessage = message;
    qDebug() << "mjpeg: error:" << message;
    setState(Error);
    stop();

    QTimer::singleShot(15000, this, SLOT(start()));
}

void MJpegStream::setUrl(const QUrl &url)
{
    m_url = url;
}

void MJpegStream::start()
{
    if (state() >= Connecting)
        return;

    if (state() == StreamOffline)
    {
        m_autoStart = true;
        return;
    }

    if (m_paused)
    {
        if (m_state != Paused)
            setState(Paused);
        return;
    }

    if (!url().isValid())
    {
        setError(QLatin1String("Internal Error"));
        return;
    }

    setState(Connecting);

    /* HACK: QNetworkAccessManager limits itself to 6 parallel connections to a single
     * host and port, which is unacceptable for this. It includes the username field when
     * caching these connections, so we can work around it by sending a fake username.
     * Needs a proper fix, because this behavior could be a Qt bug. Issue #535 */
    QUrl hackUrl = url();
    hackUrl.setUserName(QString::number(qrand()));

    /* Interval */
    if (m_interval > 1)
        hackUrl.addEncodedQueryItem("interval", QByteArray::number(m_interval));
    else if (!m_interval)
        hackUrl.addEncodedQueryItem("interval", "low");

    hackUrl.addEncodedQueryItem("activity", "1");

    m_httpReply = bcApp->nam->get(QNetworkRequest(hackUrl));
    connect(m_httpReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(requestError()));
    connect(m_httpReply, SIGNAL(finished()), SLOT(requestError()));
    connect(m_httpReply, SIGNAL(readyRead()), SLOT(readable()));

    m_activityTimer.start(30000);
}

void MJpegStream::stop()
{
    if (m_httpReply)
    {
        m_httpReply->disconnect(this);
        m_httpReply->abort();
        m_httpReply->deleteLater();
        m_httpReply = 0;
    }

    m_httpBuffer.clear();
    m_httpBoundary.clear();

    if (state() > NotConnected)
    {
        setState(NotConnected);
        m_autoStart = false;
    }

    m_activityTimer.stop();
    m_fpsRecvTs = 0;
    m_fpsRecvNo = 0;
    m_receivedFps = 0;
}

void MJpegStream::setOnline(bool online)
{
    if (!online && state() != StreamOffline)
    {
        m_autoStart = (m_autoStart || state() >= Connecting);
        setState(StreamOffline);
        stop();
    }
    else if (online && state() == StreamOffline)
    {
        setState(m_paused ? Paused : NotConnected);
        if (m_autoStart)
            start();
    }
}

void MJpegStream::setPaused(bool pause)
{
    if (pause == m_paused)
        return;

    if (pause)
    {
        m_paused = true;
        if (m_state >= Connecting)
        {
            setState(Paused);
            stop();
        }
    }
    else
    {
        m_paused = false;
        Q_ASSERT(m_state <= Paused);
        if (m_state == Paused)
            start();
    }

    emit pausedChanged(pause);
}

void MJpegStream::setInterval(int interval)
{
    /* "low" FPS, a static 1fps option */
    if (interval < 1)
        interval = 0;

    if (interval == m_interval)
        return;

    m_interval = interval;
    emit intervalChanged(m_interval);

    if (state() > NotConnected)
    {
        stop();
        start();
    }
}

bool MJpegStream::processHeaders()
{
    Q_ASSERT(m_httpReply);

    QByteArray data = m_httpReply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    QByteArray dataL = data.toLower();

    /* Get the MIME type */
    QByteArray mimeType;

    int sep = dataL.indexOf(';');
    if (sep > 0)
        mimeType = dataL.left(sep).trimmed();

    m_httpBoundary.clear();
    sep = dataL.indexOf("boundary=", sep);
    if (sep > 0)
        m_httpBoundary = data.mid(sep+9);

    if (mimeType != "multipart/x-mixed-replace" || m_httpBoundary.isEmpty())
    {
        setError(QLatin1String("Invalid content type"));
        return false;
    }

    if (m_httpBoundary.startsWith("--"))
    {
        /* Hack to work around broken MJPEG feeds, particularly some Axis devices,
         * which incorrectly include '--' in their boundary header (which would mean
         * that the actual boundary is ----xxx). We'll silently discard any extra data
         * before a boundary, so we can remove the extra --'s and it will work fine whether
         * they're properly included or not. */
        m_httpBoundary.remove(0, 2);
    }

    m_httpBoundary.prepend("\n--");
    return true;
}

void MJpegStream::readable()
{
    if (!m_httpReply)
        return;

    m_lastActivity = QDateTime::currentDateTime().toTime_t();

    if (m_httpBoundary.isNull())
    {
        if (!processHeaders())
            return;
        Q_ASSERT(!m_httpBoundary.isNull());

        m_parserState = ParserBoundary;
        setState(Buffering);
    }

    /* Don't allow the buffer to exceed 2MB */
    static const int maxBuffer = 2*1024*1024;

    for (;;)
    {
        qint64 avail = m_httpReply->bytesAvailable();
        bcApp->globalRate->addSampleValue(avail);
        if (avail < 1)
            break;

        if (m_httpBuffer.size() >= maxBuffer)
        {
            setError(QLatin1String("Exceeded maximum buffer size"));
            return;
        }

        int maxRead = qMin((int)avail, maxBuffer - m_httpBuffer.size());
        int readPos = m_httpBuffer.size();
        m_httpBuffer.resize(m_httpBuffer.size()+maxRead);

        int rd = m_httpReply->read(m_httpBuffer.data()+readPos, maxRead);
        if (rd < 0)
        {
            setError(QLatin1String("Read error"));
            return;
        }

        if (rd < maxRead)
            m_httpBuffer.truncate(readPos+rd);

        if (!parseBuffer() || !m_httpReply)
            return;
    }
}

bool MJpegStream::parseBuffer()
{
    if (m_parserState == ParserBoundary)
    {
        /* Search for the boundary */
        int boundary = m_httpBuffer.indexOf(m_httpBoundary);
        if (boundary < 0)
        {
            /* Everything up to the last m_httpBoundary-2 bytes may be safely discarded */
            m_httpBuffer.remove(0, m_httpBuffer.size() - (m_httpBoundary.size()-2));
            return true;
        }

        int boundaryStart = boundary;
        if (boundaryStart && m_httpBuffer[boundaryStart-1] == '\r')
            --boundaryStart;

        boundary += m_httpBoundary.size();
        int sz = m_httpBuffer.size() - boundary;

        if (sz >= 2 && m_httpBuffer[boundary] == '-' && m_httpBuffer[boundary+1] == '-')
        {
            boundary += 2;
            sz -= 2;
        }

        if (sz && m_httpBuffer[boundary] == '\n')
        {
            boundary++;
        }
        else if (sz >= 2 && m_httpBuffer[boundary] == '\r' && m_httpBuffer[boundary+1] == '\n')
        {
            boundary += 2;
        }
        else if (sz < 2)
        {
            /* Not enough to finish the boundary; remove everything up to the start and wait */
            m_httpBuffer.remove(0, boundaryStart);
            return true;
        }
        else
        {
            /* Invalid characters mean this isn't a boundary. Remove it. */
            m_httpBuffer.remove(0, boundaryStart + m_httpBoundary.size());
            return true;
        }

        /* Reached the end of the boundary; headers follow */
        m_httpBuffer.remove(0, boundary);
        m_parserState = ParserHeaders;
        m_httpBodyLength = 0;
    }

    if (m_parserState == ParserHeaders)
    {
        int lnStart = 0, lnEnd = 0;
        for (; (lnEnd = m_httpBuffer.indexOf('\n', lnStart)) >= 0; lnStart = lnEnd+1)
        {
            if (lnStart == lnEnd || ((lnEnd-lnStart) == 1 && m_httpBuffer[lnStart] == '\r'))
            {
                m_parserState = ParserBody;
                /* Skip the final \n */
                lnStart = lnEnd+1;

                break;
            }

            /* We only care about Content-Length */
            if ((lnEnd - lnStart) > 15 && qstrnicmp(m_httpBuffer.data()+lnStart, "Content-Length:", 15) == 0)
            {
                bool ok = false;
                m_httpBodyLength = m_httpBuffer.mid(lnStart+15, lnEnd-lnStart-15).trimmed().toUInt(&ok);
                if (!ok)
                    m_httpBodyLength = 0;
            }
            else if ((lnEnd - lnStart) > 18 && qstrnicmp(m_httpBuffer.data()+lnStart, "Bluecherry-Active:", 18) == 0)
            {
                QByteArray active = m_httpBuffer.mid(lnStart+18, lnEnd-lnStart-18).trimmed();

                /* Because we remove processed headers, we can guarantee that the header is
                 * only parsed once, so it's safe to set and emit here. */
                RecordingState newState = NoRecording;

                if (active == "true")
                    newState = MotionActive;

                if (newState != m_recordingState)
                {
                    m_recordingState = newState;
                    emit recordingStateChanged(m_recordingState);
                }
            }
        }

        /* All processed headers can be removed */
        m_httpBuffer.remove(0, lnStart);
    }

    if (m_parserState == ParserBody)
    {
        if (m_httpBodyLength)
        {
            /* The easy route; Content-Length tells us where the body ends */
            if (m_httpBuffer.size() < m_httpBodyLength)
            {
                /* Not enough data yet. */
                return true;
            }
        }
        else
        {
            /* Scan for the boundary */
            int boundary = m_httpBuffer.indexOf(m_httpBoundary);
            if (boundary < 0)
                return true;

            m_httpBodyLength = boundary;
        }

        /* End of the body; m_httpBodyLength is its length, at the beginning of m_httpBuffer */
        /* Create a QByteArray for just the body, without copying */
        decodeFrame(QByteArray(m_httpBuffer.data(), m_httpBodyLength));

        m_httpBuffer.remove(0, m_httpBodyLength);
        m_parserState = ParserBoundary;

        /* Parse again; recursion isn't a problem because this should be a very rare case */
        if (!m_httpBuffer.isEmpty())
            parseBuffer();
    }

    return true;
}

void MJpegStream::checkActivity()
{
    if (QDateTime::currentDateTime().toTime_t() - m_lastActivity > 30)
        setError(QLatin1String("Stream timeout"));
}

void MJpegStream::requestError()
{
    if (m_httpReply->error() == QNetworkReply::NoError)
        setError(QLatin1String("Connection lost"));
    else
        setError(QString::fromLatin1("HTTP error: %1").arg(m_httpReply->errorString()));
}

void MJpegStream::updateScaleSizes()
{
    /* Remove duplicates and such? */
    m_scaleSizes.clear();
    emit buildScaleSizes(m_scaleSizes);
}

void MJpegStream::decodeFrame(const QByteArray &data)
{
    /* This will cancel the task if it hasn't started yet; in-progress or completed tasks will still
     * deliver a result */
    if (m_decodeTask)
        m_decodeTask->cancel();

    m_decodeTask = new ImageDecodeTask(this, "decodeFrameResult", ++m_latestFrameNo);
    m_decodeTask->setData(data);
    m_decodeTask->setScaleSizes(m_scaleSizes);

    QThreadPool::globalInstance()->start(m_decodeTask);

    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_fpsRecvTs >= 1500)
    {
        if (m_fpsRecvTs)
            m_receivedFps = float((m_latestFrameNo - m_fpsRecvNo) * 1000 / double(now - m_fpsRecvTs));

        m_fpsRecvTs = now;
        m_fpsRecvNo = m_latestFrameNo;
    }
}

void MJpegStream::decodeFrameResult(ThreadTask *task)
{
    ImageDecodeTask *decodeTask = static_cast<ImageDecodeTask*>(task);
    if (m_decodeTask == decodeTask)
        m_decodeTask = 0;

    if (decodeTask->result().isNull() || decodeTask->imageId <= m_currentFrameNo)
        return;

    bool sizeChanged = decodeTask->result().size() != m_currentFrame.size();
#ifdef MJPEGFRAME_IS_PIXMAP
    m_currentFrame = QPixmap::fromImage(decodeTask->result());
#else
    m_currentFrame = decodeTask->result();
#endif
    m_currentFrameNo = decodeTask->imageId;

    if (sizeChanged)
        emit streamSizeChanged(m_currentFrame.size());
    emit updateFrame(m_currentFrame, decodeTask->scaleResults());

    if (m_state == Buffering)
        setState(Streaming);
}
