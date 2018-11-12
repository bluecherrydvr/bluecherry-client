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

#include "MpvVideoWidget.h"
#include "MpvVideoPlayerBackend.h"
#include "core/BluecherryApp.h"
#include <QWidget>
#include <QImage>
#include <QApplication>
#include <QSettings>
#include <QMouseEvent>
#include <QDebug>

#define ZOOM_STEP 0.05

MpvVideoWidget::~MpvVideoWidget()
{

}

MpvVideoWidget::MpvVideoWidget(QWidget *parent)
    : VideoWidget(parent),
      m_viewport(0),
      m_frameWidth(-1),
      m_frameHeight(-1),
      m_normalFrameStyle(0),
      m_zoomFactor(1.0),
      m_originalWidth(0),
      m_originalHeight(0)
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

void MpvVideoWidget::initVideo(VideoPlayerBackend *videoPlayerBackend)
{
    MpvVideoPlayerBackend *backend = reinterpret_cast<MpvVideoPlayerBackend *>(videoPlayerBackend);

    backend->setWindowId((quint64) m_viewport->winId());
}

void MpvVideoWidget::clearVideo()
{
    m_frameWidth = -1;
    m_frameHeight = -1;
}

void MpvVideoWidget::setViewport(QWidget *w)
{
    if (m_viewport)
    {
        m_viewport->removeEventFilter(this);
        m_viewport->deleteLater();
    }

    m_viewport = w;
    //m_viewport->setAttribute(Qt::WA_DontCreateNativeAncestors);
    //m_viewport->setAttribute(Qt::WA_NativeWindow);
    m_viewport->setParent(this);
    m_viewport->setGeometry(contentsRect());
    m_viewport->setAutoFillBackground(false);
    m_viewport->setStyleSheet(QLatin1String("background-color:black;"));
    m_viewport->show();
}

QSize MpvVideoWidget::sizeHint() const
{
    return QSize(m_frameWidth, m_frameHeight);
}

void MpvVideoWidget::setOverlayMessage(const QString &message)
{
    if (message == m_overlayMsg)
        return;

    m_overlayMsg = message;
    m_viewport->update();
}

void MpvVideoWidget::setFullScreen(bool on)
{
    if (on)
    {
        setWindowFlags(windowFlags() | Qt::Window);
        m_normalFrameStyle = frameStyle();
        setFrameStyle(QFrame::NoFrame);
        showFullScreen();
    }
    else
    {
        setWindowFlags(windowFlags() & ~Qt::Window);
        setFrameStyle(m_normalFrameStyle);
        showNormal();
    }

    /*QSettings settings;
    if (settings.value(QLatin1String("ui/disableScreensaver/onFullscreen")).toBool())
        bcApp->setScreensaverInhibited(on);*/
}

void MpvVideoWidget::resizeEvent(QResizeEvent *ev)
{
    QFrame::resizeEvent(ev);
    m_viewport->setGeometry(contentsRect());

    int x, y, w, h;

    x = 0;
    y = 0;
    m_originalWidth = w = this->width();
    m_originalHeight = h = this->height();

    m_viewport->move(x, y);
    m_viewport->resize(w, h);

    setZoom(m_zoomFactor);
}

void MpvVideoWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ev->accept();
    toggleFullScreen();
}

void MpvVideoWidget::zoomIn()
{
    setZoom(m_zoomFactor + ZOOM_STEP);
}

void MpvVideoWidget::zoomOut()
{
    setZoom(m_zoomFactor - ZOOM_STEP);
}

void MpvVideoWidget::setZoom(double z)
{
    m_zoomFactor = z;

    if (m_viewport)
    {
        int x, y, w, h;

        x = m_viewport->x();
        y = m_viewport->y();
        w = m_originalWidth;//m_viewport->width();
        h = m_originalHeight;//m_viewport->height();

        if (m_zoomFactor != 1.0)
        {
            w = w * m_zoomFactor;
            h = h * m_zoomFactor;

            x = (this->width() - w) / 2;
            y = (this->height() -h) / 2;
        }

        m_viewport->move(x, y);
        m_viewport->resize(w, h);
    }

}

void MpvVideoWidget::moveFrame(int dx, int dy)
{
    m_viewport->move(m_viewport->x() + dx, m_viewport->y() + dy);
}

void MpvVideoWidget::keyPressEvent(QKeyEvent *ev)
{
    if (ev->modifiers() != 0)
        return;

    switch (ev->key())
    {
    case Qt::Key_Escape:
        setFullScreen(false);
        break;
    default:
        return;
    }

    ev->accept();
}

