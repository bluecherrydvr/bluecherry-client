/*
 * Copyright 2010-2021 Bluecherry, LLC
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

#include "cameracontainerwidget.h"
#include "camera/DVRCameraStreamReader.h"
#include "camera/DVRCameraStreamWriter.h"
#include "server/DVRServer.h"
#include "server/DVRServerRepository.h"
#include "core/BluecherryApp.h"
#include "camera/DVRCamera.h"
#include "utils/FileUtils.h"
#include "PtzPresetsWindow.h"
#include "core/CameraPtzControl.h"
#include "core/LiveViewManager.h"
#include "core/PtzPresetsModel.h"
#include "LiveViewWindow.h"
#include "ui/MainWindow.h"
#include "audio/AudioPlayer.h"
#include <QSharedPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QString>
#include <QLabel>
#include <QMenu>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QAction>
#include <QDateTime>
#include <QContextMenuEvent>
#include <QSignalMapper>
#include <QInputDialog>
#include <QPixmapCache>
#include <QPainter>
#include <math.h>
#include <QDebug>

CameraContainerWidget::CameraContainerWidget(QWidget *parent)
    : QFrame(parent),m_serverRepository(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    //setBackgroundRole(QPalette::Shadow);
    setFrameShape(QFrame::Box);
    setFocusPolicy(Qt::StrongFocus);
}

void CameraContainerWidget::close()
{
    emit cameraClosed(this);
}

QString CameraContainerWidget::statusOverlayMessage()
{
    if (!m_stream)
        return QString();

    LiveStream::State state = m_stream->state();
    QString status;

    switch (state)
    {
    case LiveStream::Error:
        status = QString::fromLatin1("Error: %1")
                                    .arg(m_stream->errorMessage());
        break;
    case LiveStream::StreamOffline:
        status = QString::fromLatin1("Offline");
        break;
    case LiveStream::NotConnected:
        status =  QString::fromLatin1("Disconnected");
        break;
    case LiveStream::Connecting:
        status = QString::fromLatin1("Connecting...");
    }

    return status;
}

void CameraContainerWidget::initStaticText()
{
    m_streamstatus.setText(statusOverlayMessage());
    m_cameraname.setText(cameraName());
}

void CameraContainerWidget::drawHeader(QPainter *p, const QRect &r)
{
    int fps;
    QRect headerText(r);
    QRect brect;
    headerText.adjust(5, 2, -10, -4);
    QString ptztext;

    brect = p->boundingRect(headerText, Qt::AlignLeft | Qt::AlignTop, cameraName());
    p->drawStaticText(brect.topLeft(), m_cameraname);
    if (m_stream)
        fps = (int)ceilf(m_stream->receivedFps());

    if (camera() && camera()->hasPtz())
    {
        ptztext = tr("PTZ");
        if (m_ptz && m_ptz->currentPreset() != -1)
            ptztext.append(QString(": %1").arg(m_ptz->currentPresetName()));
    }


    p->drawText(headerText, Qt::AlignRight | Qt::AlignTop, tr("%1 %2fps").arg(ptztext).arg(fps), &brect);

    if (m_stream && m_stream.data()->hasAudio())
    {
        QString key = m_stream->isAudioEnabled() ? QString::fromLatin1("audio-stream-on") : QString::fromLatin1("audio-stream-available");
        QPixmap pm;
        if (!QPixmapCache::find(key, pm))
        {
            pm = QPixmap(QLatin1String(":/icons/%1.png").arg(key));
            QPixmapCache::insert(key, pm);
        }
        QRect audioIconRect(brect.x() - 20, r.y(), 20, 20);
        p->drawPixmap(audioIconRect, pm);
    }
}

void CameraContainerWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect brect;
    p.setCompositionMode(QPainter::CompositionMode_Source);

    p.fillRect(event->rect(), Qt::black);
    if (!m_stream)
    {
        return;
    }

    drawHeader(&p, event->rect());

    QImage frame = m_stream.data()->currentFrame();

    if (!frame.isNull())
    {
        QRect frameRect(event->rect().topLeft() += QPoint(0, 20), event->rect().size() -= QSize(0, 20));
        float xScale, yScale;
        bool rescale = false;

        if (frame.size().width() != frameRect.width() || frame.size().height() != frameRect.height())
        {
            xScale = (float)frameRect.width() / (float)frame.size().width();
            yScale = (float)frameRect.height() / (float)frame.size().height();

            if(xScale * frame.size().height() > frameRect.height())
            {
                int dx = (frameRect.width() - frame.size().width() * yScale) / 2;
                frameRect.setRect(frameRect.x() + dx, frameRect.y(), frame.size().width() * yScale, frameRect.height());
            }
            else
            {
                int dy = (frameRect.height() - frame.size().height() * xScale) / 2;
                frameRect.setRect(frameRect.x(), frameRect.y() + dy, frameRect.width(), frame.size().height() * xScale);
            }
            rescale = true;
        }
        p.drawImage(frameRect, frame);

        if (rescale && frameRect.width() > 0 &&  frameRect.height() > 0)
            m_stream.data()->setFrameSizeHint(frameRect.width(), frameRect.height());
    }

    if (m_stream->state() != LiveStream::Streaming)
    {
        brect = p.boundingRect(event->rect(), Qt::AlignCenter, m_streamstatus.text());
        p.drawStaticText(brect.topLeft(), m_streamstatus);
    }
}

QString CameraContainerWidget::cameraName() const
{
    return m_camera ? m_camera.data()->data().displayName() : QLatin1String(" ");
}

LiveStream *CameraContainerWidget::stream() const
{
    return m_stream.data();
}

void CameraContainerWidget::showFpsMenu()
{
    QPoint pos = this->pos();
    QMenu menu;
    QList<QAction*> actions = bandwidthActions();
    foreach (QAction *a, actions)
        a->setParent(&menu);
    menu.addActions(actions);
    menu.exec(pos);
}

void CameraContainerWidget::serverRemoved(DVRServer *server)
{
    if (!server || !m_camera)
        return;

    if (server == m_camera.data()->data().server())
        deleteLater();
}

void CameraContainerWidget::setServerRepository(DVRServerRepository* serverRepository)
{
    if (m_serverRepository)
        disconnect(m_serverRepository, 0, this, 0);

    m_serverRepository = serverRepository;

    if (m_serverRepository)
        connect(m_serverRepository, SIGNAL(serverRemoved(DVRServer*)), this, SLOT(serverRemoved(DVRServer*)));
}

void CameraContainerWidget::openNewWindow()
{
    if (m_camera)
        LiveViewWindow::openWindow(m_serverRepository, bcApp->mainWindow, false, m_camera.data())->show();
}

void CameraContainerWidget::openFullScreen()
{
    if (m_camera)
        LiveViewWindow::openWindow(m_serverRepository, bcApp->mainWindow, true, m_camera.data())->showFullScreen();
}

void CameraContainerWidget::setBandwidthModeFromAction()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (!a || a->data().isNull() || !stream())
        return;

    int mode = a->data().toInt();
    stream()->setBandwidthMode(mode);
    stream()->setPaused(false);
}

void CameraContainerWidget::enableAudio()
{
    Q_ASSERT(stream());
    updateAudioState(CameraContainerWidget::Save);
    stream()->enableAudio(true);
}

void CameraContainerWidget::disableAudio()
{
    Q_ASSERT(stream());
    updateAudioState(CameraContainerWidget::Disable);
    stream()->enableAudio(false);
}

void CameraContainerWidget::updateAudioState(CameraContainerWidget::AudioState state)
{
    QSettings settings;
    int serverId, cameraId;

    switch (state)
    {
    case CameraContainerWidget::Disable:
        settings.remove(QLatin1String("ui/audioState"));
        break;

    case CameraContainerWidget::Save:
        serverId = m_camera.data()->data().server()->configuration().id();
        cameraId = m_camera.data()->data().id();
        settings.setValue(QString::fromLatin1("ui/audioState"), QString("%1/%2").arg(serverId).arg(cameraId));
        break;

    case CameraContainerWidget::Load:
    {
        QStringList list = settings.value(QString::fromLatin1("ui/audioState")).toString().split("/");

        if (list.size() != 2)
            goto removeID;

        bool ok;
        serverId = list[0].toInt(&ok);
        if (!ok)
            goto removeID;
        cameraId = list[1].toInt(&ok);
        if (!ok)
            goto removeID;

        if (serverId == m_camera.data()->data().server()->configuration().id() &&
                cameraId == m_camera.data()->data().id() && bcApp->audioPlayer->isDeviceEnabled() &&
                     stream() && stream()->hasAudio() && !stream()->isAudioEnabled())
        {
            stream()->enableAudio(true);
        }
        break;

        removeID:
        qDebug() << "CameraContainerWidget: removing wrong settings value\n";
        settings.remove(QLatin1String("ui/audioState"));
        break;
    }
    default:
        qDebug() << "CameraContainerWidget: Used wrong audio state!\n";
    }
}

void CameraContainerWidget::saveSnapshot()
{
    if (!m_camera)
        return;

    /* Grab the current frame, so the user gets what they expect regardless of the time taken by the dialog */
    QImage frame = m_camera.data()->liveStream()->currentFrame();
    if (frame.isNull())
        return;

    QWidget *window = this;

    QString file;

    if (file.isEmpty())
    {
        file = getSaveFileNameExt(window, tr("%1 - Save Snapshot").arg(m_camera.data()->data().displayName()),
                           QDesktopServices::storageLocation(QDesktopServices::PicturesLocation),
                           QLatin1String("ui/snapshotSaveLocation"),
                           QString::fromLatin1("%1 - %2.jpg").arg(m_camera.data()->data().displayName(),
                                                                  QDateTime::currentDateTime().toString(
                                                                  QLatin1String("yyyy-MM-dd hh-mm-ss"))),
                           tr("Image (*.jpg)"));

        if (file.isEmpty())
            return;
        if (!file.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive))
            file.append(QLatin1String(".jpg"));
    }

    if (!frame.save(file, "jpeg"))
    {
        QMessageBox::critical(window, tr("Snapshot Error"), tr("An error occurred while saving the snapshot image."),
                              QMessageBox::Ok);
        return;
    }
}

CameraPtzControl::Movement CameraContainerWidget::moveForPosition(int x, int y)
{
    int xarea = width() / 4, yarea = height() / 4;
    int movement = CameraPtzControl::NoMovement;

    if (x < xarea)
        movement |= CameraPtzControl::MoveWest;
    else if (x >= 3*xarea)
        movement |= CameraPtzControl::MoveEast;
    if (y < yarea)
        movement |= CameraPtzControl::MoveNorth;
    else if (y >= 3*yarea)
        movement |= CameraPtzControl::MoveSouth;

    return (CameraPtzControl::Movement) movement;
}

void CameraContainerWidget::setCustomCursor(CustomCursor cursor)
{
    if (cursor == m_customCursor)
        return;

    m_customCursor = cursor;

    int rotate = 0;

    switch (m_customCursor)
    {
    case DefaultCursor:
        setCursor(QCursor());
        return;
    case MoveCursorE:
        break;
    case MoveCursorSE:
        rotate = 45;
        break;
    case MoveCursorS:
        rotate = 90;
        break;
    case MoveCursorSW:
        rotate = 135;
        break;
    case MoveCursorW:
        rotate = 180;
        break;
    case MoveCursorNW:
        rotate = 225;
        break;
    case MoveCursorN:
        rotate = 270;
        break;
    case MoveCursorNE:
        rotate = 315;
        break;
    }

    QString key = QString::fromLatin1("ptzcursor-%1").arg(rotate);
    QPixmap pm;
    if (!QPixmapCache::find(key, pm))
    {
        pm = QPixmap(QLatin1String(":/images/ptz-arrow.png")).transformed(QTransform().rotate(rotate), Qt::SmoothTransformation);
        QPixmapCache::insert(key, pm);
    }

    setCursor(QCursor(pm));
}

void CameraContainerWidget::setPtzEnabled(bool ptzEnabled)
{
    if (ptzEnabled == !m_ptz.isNull())
        return;

    if (ptzEnabled)
        setMouseTracking(true);

    if (ptzEnabled && m_camera)
        m_ptz = m_camera.data()->sharedPtzControl();
    else
        m_ptz.clear();
}
void CameraContainerWidget::ptzPresetSave()
{
    if (!m_ptz)
        return;

    QString re = QInputDialog::getText(this, tr("Save PTZ preset"), tr("Enter a name for the new PTZ preset:"));
    if (re.isEmpty())
        return;

    m_ptz->savePreset(-1, re);
}

void CameraContainerWidget::ptzPresetWindow()
{
    if (!m_ptz)
        return;

    PtzPresetsWindow *window = new PtzPresetsWindow(m_ptz.data(), bcApp->mainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);

    {
        QRect itemScreenRect = this->rect(); // ?? //view->mapFromScene(mapToScene(QRectF(0, 0, width(), height()))).boundingRect();
        //itemScreenRect.moveTopLeft(view->viewport()->mapToGlobal(itemScreenRect.topLeft()));

        window->move(itemScreenRect.right() - qRound(window->width() / 2.0),
                     itemScreenRect.top() + qMax(0, qRound((itemScreenRect.height() - window->height()) / 2.0)));
    }

    window->show();
}

QMenu *CameraContainerWidget::ptzMenu()
{
    QMenu *menu = new QMenu;
    menu->setTitle(tr("Pan / Tilt / Zoom"));

    menu->addAction(m_ptz ? tr("Disable PTZ") : tr("Enable PTZ"),
                    this, SLOT(togglePtzEnabled()));

    if (m_ptz)
    {
        menu->addSeparator();

        QMenu *presetsMenu = menu->addMenu(tr("Presets"));
        QSignalMapper *mapper = new QSignalMapper(presetsMenu);
        connect(mapper, SIGNAL(mapped(int)), m_ptz.data(), SLOT(moveToPreset(int)));

        const QMap<int,QString> &presets = m_ptz->presets();
        for (QMap<int,QString>::ConstIterator it = presets.constBegin(); it != presets.constEnd(); ++it)
        {
            /* Necessary to avoid mnemonics (issue #730) */
            QString text = it.value();
            text.replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction *a = presetsMenu->addAction(text, mapper, SLOT(map()));
            mapper->setMapping(a, it.key());
        }

        presetsMenu->addSeparator();
        presetsMenu->addAction(tr("Edit presets..."), this, SLOT(ptzPresetWindow()));

        menu->addAction(tr("Edit presets..."), this, SLOT(ptzPresetWindow()));
        menu->addAction(tr("Save preset..."), this, SLOT(ptzPresetSave()));

        menu->addSeparator();
        QAction *a = menu->addAction(tr("Cancel actions"), m_ptz.data(), SLOT(cancelAll()));
        a->setEnabled(m_ptz->hasPendingActions());
        connect(m_ptz.data(), SIGNAL(hasPendingActionsChanged(bool)), a, SLOT(setEnabled(bool)));
    }

    return menu;
}

QList<QAction*> CameraContainerWidget::bandwidthActions()
{
    bool paused = stream() ? stream()->isPaused() : false;

    QList<QAction*> actions;
    QAction *a = new QAction(paused ? tr("Paused") : tr("Pause"), this);
    connect(a, SIGNAL(triggered()), stream(), SLOT(togglePaused()));
    a->setCheckable(true);
    a->setChecked(paused);
    actions << a;

    actions << bcApp->liveView->bandwidthActions((paused || !stream()) ? -1 : stream()->bandwidthMode(),
                                                 this, SLOT(setBandwidthModeFromAction()));
    return actions;
}

void CameraContainerWidget::set_main_stream()
{
    if (!camera())
        return;

    camera()->data().server()->switchSubstream(camera()->data().id(), false);
}

void CameraContainerWidget::set_sub_stream()
{
    if (!camera())
        return;

    camera()->data().server()->switchSubstream(camera()->data().id(), true);
}

void CameraContainerWidget::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
//    if (event->reason() == QGraphicsSceneContextMenuEvent::Mouse && !allowNextContextMenu)
//        return;
//    allowNextContextMenu = false;

    QMenu menu(this);

    QAction *a = menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()));
    a->setEnabled(stream() && !stream()->currentFrame().isNull());

    QMenu substream_menu;
    substream_menu.setTitle(tr("Switch liveview substream"));
    substream_menu.addAction(tr("Main Stream"), this, SLOT(set_main_stream()));
    substream_menu.addAction(tr("Sub Stream"), this, SLOT(set_sub_stream()));
    a = menu.addMenu(&substream_menu);
    a->setEnabled(camera());

    QMenu *ptzmenu = 0;
    if (camera() && camera()->hasPtz())
    {
        if (m_ptz)
        {
            ptzmenu = ptzMenu();
            QAction *a = menu.addMenu(ptzmenu);
            a->setCheckable(true);
            a->setChecked(ptz());
        }
        else
        {
            menu.addAction(tr("Pan / Tilt / Zoom"), this, SLOT(togglePtzEnabled()));
        }
    }

    menu.addSeparator();

    QList<QAction*> bw = bandwidthActions();
    foreach (QAction *a, bw)
        a->setParent(&menu);
    menu.addActions(bw);

    menu.addSeparator();
    menu.addAction(tr("Open in window"), this, SLOT(openNewWindow()));
    menu.addAction(tr("Open as fullscreen"), this, SLOT(openFullScreen()));
    menu.addSeparator();

    if (bcApp->audioPlayer->isDeviceEnabled() && stream() && stream()->hasAudio())
    {
        if (stream()->isAudioEnabled())
            menu.addAction(tr("Disable audio"), this, SLOT(disableAudio()));
        else
            menu.addAction(tr("Enable audio"), this, SLOT(enableAudio()));
        menu.addSeparator();
    }

    QAction *actClose = menu.addAction(tr("Close camera"), this, SLOT(close()));
    actClose->setEnabled(!m_camera.isNull());

    menu.exec(event->globalPos());
    delete ptzmenu;
}

void CameraContainerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_ptz)
    {
        event->ignore();
        setMouseTracking(false);
        return;
    }

    event->accept();

    CameraPtzControl::Movement movements = moveForPosition(event->x(), event->y());
    CustomCursor cursor = DefaultCursor;

    if (movements & CameraPtzControl::MoveNorth) {
        if (movements & CameraPtzControl::MoveWest)
            cursor = MoveCursorNW;
        else if (movements & CameraPtzControl::MoveEast)
            cursor = MoveCursorNE;
        else
            cursor = MoveCursorN;
    }
    else if (movements & CameraPtzControl::MoveSouth) {
        if (movements & CameraPtzControl::MoveWest)
            cursor = MoveCursorSW;
        else if (movements & CameraPtzControl::MoveEast)
            cursor = MoveCursorSE;
        else
            cursor = MoveCursorS;
    }
    else if (movements & CameraPtzControl::MoveWest)
        cursor = MoveCursorW;
    else if (movements & CameraPtzControl::MoveEast)
        cursor = MoveCursorE;

    setCustomCursor(cursor);
}

void CameraContainerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_ptz)
    {
        event->ignore();
        return;
    }
    CameraPtzControl::Movement movement = moveForPosition(event->x(), event->y());
    if (movement == CameraPtzControl::NoMovement)
    {
        event->accept();
        m_ptz->move(CameraPtzControl::MoveTele);
    }
    else
        event->ignore();
}

void CameraContainerWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_ptz)
    {
        event->ignore();
        return;
    }

    event->accept();

    CameraPtzControl::Movement movement = moveForPosition(event->x(), event->y());
    if (movement != CameraPtzControl::NoMovement)
        m_ptz->move(movement);
}

void CameraContainerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_ptz)
    {
        event->ignore();
        return;
    }

    event->accept();
    setCustomCursor(DefaultCursor);
}

void CameraContainerWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_ptz)
    {
        event->ignore();
        return;
    }

    event->accept();

    int steps = event->delta() / 120;
    if (!steps)
        return;

    m_ptz->move((steps < 0) ? CameraPtzControl::MoveWide : CameraPtzControl::MoveTele);
}

void CameraContainerWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_ptz || event->isAutoRepeat())
    {
        QFrame::keyPressEvent(event);
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Left:
        m_ptz->move(CameraPtzControl::MoveWest);
        break;
    case Qt::Key_Right:
        m_ptz->move(CameraPtzControl::MoveEast);
        break;
    case Qt::Key_Up:
        m_ptz->move((event->modifiers() & Qt::ShiftModifier) ? CameraPtzControl::MoveTele : CameraPtzControl::MoveNorth);
        break;
    case Qt::Key_Down:
        m_ptz->move((event->modifiers() & Qt::ShiftModifier) ? CameraPtzControl::MoveWide : CameraPtzControl::MoveSouth);
        break;
    default:
        QFrame::keyPressEvent(event);
        return;
    }
}

void CameraContainerWidget::setCamera(DVRCamera *camera)
{
    if (camera == m_camera.data())
        return;

    if (m_camera)
    {
        m_camera.data()->disconnect(this);
        //m_streamItem->clear();
    }

    m_camera = camera;

    {
        if (camera->liveStream() == m_stream)
            return;

        if (m_stream)
        {
            m_stream.data()->disconnect(this);
            m_stream.data()->unref();
        }

        m_stream = camera->liveStream();

        if (m_stream.data())
        {
            connect(m_stream.data(), SIGNAL(updated()), SLOT(updateFrame()));
            //connect(m_stream.data(), SIGNAL(streamSizeChanged(QSize)), SLOT(updateFrameSize()));
            m_stream.data()->start();
            m_stream.data()->ref();
        }

        //updateFrameSize();
        updateFrame();
    }

    if (m_camera)
    {
        connect(m_camera.data()->data().server(), SIGNAL(changed()), SLOT(cameraDataUpdated()));
        connect(m_camera.data(), SIGNAL(dataUpdated()), SLOT(cameraDataUpdated()));
        connect(m_camera.data(), SIGNAL(onlineChanged(bool)), SLOT(cameraDataUpdated()));
        connect(m_camera.data(), SIGNAL(recordingStateChanged(int)), SIGNAL(recordingStateChanged()));
    }

    cameraDataUpdated();
    emit cameraChanged(camera);
    emit recordingStateChanged();

    if (camera && stream())
        connect(stream(), SIGNAL(audioChanged()), SLOT(updateAudioState()));
}

void CameraContainerWidget::cameraDataUpdated()
{
    emit cameraNameChanged(cameraName());
    emit hasPtzChanged();

    emit cameraChanged(m_camera.data());
    initStaticText();
}

void CameraContainerWidget::saveState(QDataStream *data)
{
    Q_ASSERT(data);

    if (m_camera)
    {
        DVRCameraStreamWriter writer(*data);
        writer.writeCamera(m_camera.data());
        *data << (stream() ? stream()->bandwidthMode() : 0);
    }
}

void CameraContainerWidget::loadState(QDataStream *data, int version)
{
    Q_ASSERT(data);

    DVRCameraStreamReader reader(m_serverRepository, *data);
    DVRCamera *camera = reader.readCamera();
    if (camera)
    {
        setCamera(camera);
    }
    else
    {
        clear();
    }

    if (version >= 1) {
        int bandwidth_mode = 0;
        *data >> bandwidth_mode;

        if (stream())
            stream()->setBandwidthMode(bandwidth_mode);
    }
}
