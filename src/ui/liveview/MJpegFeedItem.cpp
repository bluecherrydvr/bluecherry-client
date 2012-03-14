#include "MJpegFeedItem.h"
#include "core/BluecherryApp.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSettings>
#include <QGLContext>

MJpegFeedItem::MJpegFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_useAdvancedGL(true), m_texId(0), m_texDataPtr(0)
{
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);
    updateSettings();
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
}

MJpegFeedItem::~MJpegFeedItem()
{
    if (m_texId)
        glDeleteTextures(1, (GLuint*)&m_texId);
}

void MJpegFeedItem::setStream(const QSharedPointer<LiveStream> &stream)
{
    if (stream == m_stream)
        return;

    if (m_stream)
        m_stream->disconnect(this);

    m_stream = stream;

    if (m_stream)
    {
        connect(m_stream.data(), SIGNAL(updated()), SLOT(updateFrame()));
        connect(m_stream.data(), SIGNAL(streamSizeChanged(QSize)), SLOT(updateFrameSize()));
        connect(m_stream.data(), SIGNAL(stateChanged(int)), SLOT(streamStateChanged(int)));
        connect(m_stream.data(), SIGNAL(pausedChanged(bool)), SIGNAL(pausedChanged(bool)));
        m_stream->start();
    }
    else
        emit errorTextChanged(tr("No<br>Video"));

    emit pausedChanged(isPaused());
    emit connectedChanged(isConnected());
    emit intervalChanged(interval());

    updateFrameSize();
    updateFrame();
}

void MJpegFeedItem::clear()
{
    setStream(QSharedPointer<LiveStream>());
    if (m_texId)
    {
        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
    }
}

bool MJpegFeedItem::isPaused() const
{
    return /*m_stream ? m_stream->isPaused() :*/ false;
}

void MJpegFeedItem::setPaused(bool paused)
{
    //if (m_stream)
    //    m_stream->setPaused(paused);
}

bool MJpegFeedItem::isConnected() const
{
    //return m_stream ? (m_stream->state() >= LiveStream::Streaming || m_stream->state() == LiveStream::Paused) : false;
    return true;
}

int MJpegFeedItem::interval() const
{
    return /*m_stream ? m_stream->interval() :*/ 1;
}

int MJpegFeedItem::fps() const
{
    return m_stream ? qRound(m_stream->receivedFps()) : 0;
}

void MJpegFeedItem::setInterval(int interval)
{
    if (m_stream)
    {
        //m_stream->setInterval(interval);
        emit intervalChanged(interval);
    }
}

void MJpegFeedItem::clearInterval()
{
//    m_stream->clearInterval();
    emit intervalChanged(interval());
}

void MJpegFeedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    Q_UNUSED(widget);
    if (!m_stream)
        return;

    QImage frame = m_stream->currentFrame();

    if (frame.isNull())
    {
        p->fillRect(opt->rect, Qt::black);
        return;
    }

    if (m_useAdvancedGL && p->paintEngine()->type() == QPaintEngine::OpenGL2)
    {
        static int c = 1;
        if (Q_UNLIKELY(c))
        {
            c = 0;
            qDebug("Using advanced OpenGL output");
        }

        p->beginNativePainting();
        glEnable(GL_TEXTURE_2D);

        if (Q_LIKELY(m_texId))
        {
            glBindTexture(GL_TEXTURE_2D, m_texId);
            if (frame.constBits() != m_texDataPtr)
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width(), frame.height(),
                                GL_BGRA, GL_UNSIGNED_BYTE, frame.constBits());
                m_texDataPtr = frame.constBits();
            }
        }
        else
        {
            glGenTextures(1, (GLuint*)&m_texId);
            Q_ASSERT(m_texId);
            glBindTexture(GL_TEXTURE_2D, m_texId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, 3, frame.width(), frame.height(), 0,
                         GL_BGRA, GL_UNSIGNED_BYTE, frame.constBits());
            m_texDataPtr = frame.constBits();
        }

        static const GLfloat texCoordArray[8] = {
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
        };

        const GLfloat w = opt->rect.width(), h = opt->rect.height();
        GLfloat vertexArray[4*2] = { 0, 0, w, 0, w, h, 0, h };

        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        p->endNativePainting();
    }
    else
    {
        p->save();
        //p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setCompositionMode(QPainter::CompositionMode_Source);
        p->drawImage(opt->rect, frame);
        p->restore();
    }
}

void MJpegFeedItem::streamStateChanged(int state)
{
    Q_ASSERT(m_stream);

    emit connectedChanged(isConnected());

    switch (state)
    {
    case LiveStream::Error:
        emit errorTextChanged(tr("<span style='color:#ff0000;'>Error</span>"));
        //setToolTip(m_stream->errorMessage());
        break;
    case LiveStream::StreamOffline:
        emit errorTextChanged(tr("Offline"));
        break;
    case LiveStream::NotConnected:
        emit errorTextChanged(tr("Disconnected"));
        break;
    case LiveStream::Connecting:
        emit errorTextChanged(tr("Connecting..."));
        break;
    default:
        emit errorTextChanged(QString());
        break;
    }
}

void MJpegFeedItem::updateSettings()
{
    QSettings settings;
    m_useAdvancedGL = !settings.value(QLatin1String("ui/liveview/disableAdvancedOpengl"), false).toBool();
    if (!m_useAdvancedGL && m_texId)
    {
        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
    }
}
