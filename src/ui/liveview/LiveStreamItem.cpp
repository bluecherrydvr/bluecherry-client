#include "LiveStreamItem.h"
#include "core/BluecherryApp.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSettings>
#include <QGLContext>

#ifndef Q_UNLIKELY
#define Q_UNLIKELY(x) x
#define Q_LIKELY(x) x
#endif

#if defined(Q_OS_WIN) && !defined(GL_CLAMP_TO_EDGE)
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#if defined(Q_OS_WIN) && !defined(GL_BGRA)
#define GL_BGRA GL_BGRA_EXT
#endif

LiveStreamItem::LiveStreamItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_useAdvancedGL(true), m_texId(0), m_texDataPtr(0)
{
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);
    updateSettings();
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
}

LiveStreamItem::~LiveStreamItem()
{
    if (m_texId)
        glDeleteTextures(1, (GLuint*)&m_texId);
}

void LiveStreamItem::setStream(const QSharedPointer<LiveStream> &stream)
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
        m_stream->start();
    }

    updateFrameSize();
    updateFrame();
}

void LiveStreamItem::clear()
{
    setStream(QSharedPointer<LiveStream>());
    if (m_texId)
    {
        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
    }
}

void LiveStreamItem::updateFrameSize()
{
    if (m_texId)
    {
        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
    }

    emit frameSizeChanged(frameSize());
}

void LiveStreamItem::paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget)
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

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frame.width(), frame.height(), 0,
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

void LiveStreamItem::updateSettings()
{
    QSettings settings;
    m_useAdvancedGL = !settings.value(QLatin1String("ui/liveview/disableAdvancedOpengl"), false).toBool();
    if (!m_useAdvancedGL && m_texId)
    {
        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
    }
}
