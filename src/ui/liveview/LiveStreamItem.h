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

#ifndef LIVESTREAMITEM_H
#define LIVESTREAMITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "rtsp-stream/RtspStream.h"

class QGLContext;

class LiveStreamItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(RtspStream *stream READ stream NOTIFY streamChanged)
    Q_PROPERTY(QSizeF frameSize READ frameSize NOTIFY frameSizeChanged)

public:
    explicit LiveStreamItem(QDeclarativeItem *parent = 0);
    virtual ~LiveStreamItem();

    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    RtspStream * stream() const { return m_stream.data(); }
    void setStream(QSharedPointer<RtspStream> stream);
    void clear();

    QSizeF frameSize() const { return m_stream ? m_stream.data()->streamSize() : QSize(0, 0); }

signals:
    void streamChanged(RtspStream *stream);
    void frameSizeChanged(const QSizeF &frameSize);

private slots:
    void updateFrame()
    {
        update();
    }

    void updateFrameSize();
    void updateSettings();

private:
    QSharedPointer<RtspStream> m_stream;
    bool m_useAdvancedGL;
    unsigned m_texId;
    const QGLContext *m_texLastContext;
    bool m_texInvalidate;
    const uchar *m_texDataPtr;

    void clearTexture();
};

#endif // LIVESTREAMITEM_H
