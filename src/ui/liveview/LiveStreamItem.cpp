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
    : QDeclarativeItem(parent), m_useAdvancedGL(true), m_texId(0), m_texLastContext(0), m_texInvalidate(false),
      m_texDataPtr(0)
{
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);
    updateSettings();
    connect(bcApp, SIGNAL(settingsChanged()), SLOT(updateSettings()));
}

LiveStreamItem::~LiveStreamItem()
{
    clearTexture();
}

/* This is odd and hackish logic to manage deletion of textures. The problem here is
 * that QDeclarativeItem (or QGraphicsItem) is technically divorced from the actual
 * painting surface (our GL context), but we need to bind a persistent texture in that
 * context. There is no notification when the context has changed or been deleted, but
 * we this can cover any existing case without leaking. Note that the deletion of the
 * context will implicitly free textures, so missing deletion from the destructor is likely
 * not a problem (since that almost always precedes destruction of a LiveViewArea).
 *
 * The only significant leak risk I can see is if the item is somehow deleted (without
 * deleting the entire view) without the relevant context being current. That will at
 * least warn, but it's unavoidable without creating some connection between LiveViewArea
 * and this item.
 *
 * Bug #1118 revealed the problem, which was caused by deleting textures in the wrong context.
 */
void LiveStreamItem::clearTexture()
{
    if (m_texId)
    {
        Q_ASSERT(m_texLastContext);
        if (m_texLastContext && QGLContext::currentContext() != m_texLastContext)
        {
            qDebug() << "LiveStreamItem: Current context" << QGLContext::currentContext() <<
                        "does not match texture context" << (void*)m_texLastContext << "- cannot "
                        "delete old texture. This could leak, but most likely the context was "
                        "destroyed or we will delete the texture later.";
            m_texInvalidate = true;
            return;
        }

        if (m_texInvalidate)
            qDebug() << "LiveStreamItem: Texture invalidation successful";

        glDeleteTextures(1, (GLuint*)&m_texId);
        m_texId = 0;
        m_texLastContext = 0;
        m_texInvalidate = false;
    }
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
    clearTexture();
}

void LiveStreamItem::updateFrameSize()
{
    clearTexture();
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

    /* For advanced GL textures that were invalidated (deleted when the relevant context
     * was not current), attempt to delete them here if appropriate. */
    const bool glContextChanged = QGLContext::currentContext() != m_texLastContext;
    if (m_texId && (glContextChanged || m_texInvalidate))
        clearTexture();

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

        if (Q_LIKELY(m_texId && !m_texInvalidate))
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
            if (m_texInvalidate)
                qDebug() << "LiveStreamItem: Replacing invalidated texture; potential leak, but context was probably deleted.";

            glGenTextures(1, (GLuint*)&m_texId);
            m_texLastContext = QGLContext::currentContext();
            m_texInvalidate = false;
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
        m_texId = 0;
        m_texInvalidate = 0;
        m_texLastContext = 0;

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
        clearTexture();
}
