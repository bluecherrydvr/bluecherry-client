/*
 * Copyright 2010-2014 Bluecherry
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

#include "MplVideoWidget.h"
#include "MplVideoPlayerBackend.h"
#include <QWidget>


MplVideoWidget::~MplVideoWidget()
{

}

MplVideoWidget::MplVideoWidget(QWidget *parent)
    : VideoWidget(parent),
      m_viewport(0),
      m_frameWidth(-1),
      m_frameHeight(-1)
{
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMinimumSize(320, 240);
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    p.setColor(QPalette::WindowText, Qt::white);
    setPalette(p);

    setViewport(new QWidget);
}

void MplVideoWidget::initVideo(VideoPlayerBackend *videoPlayerBackend)
{
    MplVideoPlayerBackend *backend = reinterpret_cast<MplVideoPlayerBackend *>(videoPlayerBackend);

    backend->setWindowId((quint64) m_viewport->winId());
}

void MplVideoWidget::clearVideo()
{
    m_frameWidth = -1;
    m_frameHeight = -1;
}

void MplVideoWidget::setViewport(QWidget *w)
{
    if (m_viewport)
    {
        m_viewport->removeEventFilter(this);
        m_viewport->deleteLater();
    }

    m_viewport = w;
    m_viewport->setParent(this);
    m_viewport->setGeometry(contentsRect());
    m_viewport->setAutoFillBackground(false);
    m_viewport->setAttribute(Qt::WA_OpaquePaintEvent);
    m_viewport->installEventFilter(this);
    m_viewport->show();
}

QSize MplVideoWidget::sizeHint() const
{
    return QSize(m_frameWidth, m_frameHeight);
}

void MplVideoWidget::setOverlayMessage(const QString &message)
{
    if (message == m_overlayMsg)
        return;

    m_overlayMsg = message;
    m_viewport->update();
}
