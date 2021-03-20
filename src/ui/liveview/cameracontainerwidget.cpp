#include "cameracontainerwidget.h"
#include "camerawidget.h"
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
#include <QDebug>

CameraContainerWidget::CameraContainerWidget(QWidget *parent) : QFrame(parent),
  m_cameraview(0), m_serverRepository(0)
{
    setFrameShape(QFrame::Box);
    setFocusPolicy(Qt::StrongFocus);
    QGridLayout *vl = new QGridLayout();
    QHBoxLayout *headerlayout = new QHBoxLayout();
    m_cameraview = new CameraWidget();

    QLabel *lb_name = new QLabel(cameraName());
    QLabel *lb_fps = new QLabel(tr("0fps"));
    //headerlayout->addStrut(12);
    headerlayout->addWidget(lb_name);
    headerlayout->addStretch(1);
    headerlayout->addWidget(lb_fps);
    vl->setSizeConstraint(QLayout::SetNoConstraint);
    vl->addLayout(headerlayout, 0, 0);
    vl->addWidget(m_cameraview, 1, 0);
    vl->setRowStretch(1, 4);
    //vl->setColumnStretch(1, 0);
    setLayout(vl);
}

void CameraContainerWidget::close()
{
    //
}

QString CameraContainerWidget::cameraName() const
{
    return m_camera ? m_camera.data()->data().displayName() : QLatin1String(" ");
}

LiveStream *CameraContainerWidget::stream() const
{
    return m_cameraview ? m_cameraview->stream() : 0;
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

void CameraContainerWidget::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
//    if (event->reason() == QGraphicsSceneContextMenuEvent::Mouse && !allowNextContextMenu)
//        return;
//    allowNextContextMenu = false;

    QMenu menu(this);

    QAction *a = menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()));
    a->setEnabled(m_cameraview->stream() && !m_cameraview->stream()->currentFrame().isNull());

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
    if (!m_ptz)
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

    if (layout())
    {
        delete layout();
    }

    QGridLayout *vl = new QGridLayout();
    QHBoxLayout *headerlayout = new QHBoxLayout();
    m_cameraview = new CameraWidget();
    m_cameraview->setStream(camera->liveStream());

    QLabel *lb_name = new QLabel(cameraName());
    QLabel *lb_fps = new QLabel(tr("0fps"));
    //headerlayout->addStrut(12);
    headerlayout->addWidget(lb_name);
    headerlayout->addStretch(1);
    headerlayout->addWidget(lb_fps);
    vl->setSizeConstraint(QLayout::SetNoConstraint);
    vl->addLayout(headerlayout, 0, 0);
    vl->addWidget(m_cameraview, 1, 0);
    vl->setRowStretch(1, 4);
    //vl->setColumnStretch(1, 0);
    setLayout(vl);

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

    m_cameraview->setStream(m_camera.data()->liveStream());

    emit cameraChanged(m_camera.data());
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
        setCamera(camera);
    else
        clear();

    if (version >= 1) {
        int bandwidth_mode = 0;
        *data >> bandwidth_mode;
        // this assert assumes that configuration file is always valid
        // but this is something we cannot be sure of
        // Q_ASSERT(stream());

        if (stream())
            stream()->setBandwidthMode(bandwidth_mode);
    }
}
