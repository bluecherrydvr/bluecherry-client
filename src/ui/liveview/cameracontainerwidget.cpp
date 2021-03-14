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
#include <QDebug>

CameraContainerWidget::CameraContainerWidget(QWidget *parent) : QFrame(parent),
  m_cameraview(0), m_serverRepository(0)
{
    setFrameShape(QFrame::Box);
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

void CameraContainerWidget::setPtzEnabled(bool ptzEnabled)
{
    if (ptzEnabled == !m_ptz.isNull())
        return;

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
}
