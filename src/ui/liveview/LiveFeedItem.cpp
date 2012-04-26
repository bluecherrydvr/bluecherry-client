#include "LiveFeedItem.h"
#include "LiveStreamItem.h"
#include "PtzPresetsWindow.h"
#include "core/BluecherryApp.h"
#include "core/CameraPtzControl.h"
#include "LiveViewWindow.h"
#include "ui/MainWindow.h"
#include "utils/FileUtils.h"
#include "core/DVRServer.h"
#include "core/LiveViewManager.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDataStream>
#include <QPixmapCache>
#include <QInputDialog>
#include <QSignalMapper>
#include <QApplication>
#include <QDesktopWidget>

LiveFeedItem::LiveFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_streamItem(0), m_customCursor(DefaultCursor)
{
    setAcceptedMouseButtons(acceptedMouseButtons() | Qt::RightButton);
}

void LiveFeedItem::setStreamItem(LiveStreamItem *item)
{
    if (item == m_streamItem)
        return;

    Q_ASSERT(!m_streamItem);
    m_streamItem = item;
}

LiveStream *LiveFeedItem::stream() const
{
    return m_streamItem ? m_streamItem->stream().data() : 0;
}

void LiveFeedItem::setCamera(const DVRCamera &camera)
{
    if (camera == m_camera)
        return;

    if (m_camera)
    {
        static_cast<QObject*>(m_camera)->disconnect(this);
        m_streamItem->clear();
    }

    m_camera = camera;

    if (m_camera)
    {
        connect(m_camera, SIGNAL(dataUpdated()), SLOT(cameraDataUpdated()));
        connect(m_camera, SIGNAL(onlineChanged(bool)), SLOT(cameraDataUpdated()));
        connect(m_camera, SIGNAL(recordingStateChanged(int)), SIGNAL(recordingStateChanged()));
    }

    cameraDataUpdated();
    emit cameraChanged(camera);
    emit recordingStateChanged();
}

void LiveFeedItem::cameraDataUpdated()
{
    emit cameraNameChanged(cameraName());
    emit hasPtzChanged();

    QSharedPointer<LiveStream> nstream = m_camera.liveStream();
    m_streamItem->setStream(nstream);
}

void LiveFeedItem::openNewWindow()
{
    LiveViewWindow::openWindow(bcApp->mainWindow, false, camera())->show();
}

void LiveFeedItem::openFullScreen()
{
    LiveViewWindow::openWindow(bcApp->mainWindow, true, camera())->showFullScreen();
}

void LiveFeedItem::close()
{
    bool closeFeedItem = parentItem()->metaObject()->invokeMethod(parentItem(), "removeItem", Q_ARG(QDeclarativeItem*, this));
    Q_ASSERT(closeFeedItem);
    Q_UNUSED(closeFeedItem);
}

void LiveFeedItem::saveSnapshot(const QString &ifile)
{
    if (!m_camera)
        return;

    /* Grab the current frame, so the user gets what they expect regardless of the time taken by the dialog */
    QImage frame = m_camera.liveStream()->currentFrame();
    if (frame.isNull())
        return;

    QWidget *window = scene()->views().value(0);

    QString file = ifile;

    if (file.isEmpty())
    {
        file = getSaveFileNameExt(window, tr("%1 - Save Snapshot").arg(m_camera.displayName()),
                           QDesktopServices::storageLocation(QDesktopServices::PicturesLocation),
                           QLatin1String("ui/snapshotSaveLocation"),
                           QString::fromLatin1("%1 - %2.jpg").arg(m_camera.displayName(),
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

/* contextMenuEvent will arrive after right clicks that were captured within QML.
 * To avoid this, we need a bit of a hack to tell when a context menu event was
 * caused by a right click, and only allow it if we saw the click (i.e. it wasn't
 * handled by QML). There are no reentrancy issues here. */
static bool allowNextContextMenu = false;

void LiveFeedItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        allowNextContextMenu = true;

    QDeclarativeItem::mousePressEvent(event);
}

void LiveFeedItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();
    if (event->reason() == QGraphicsSceneContextMenuEvent::Mouse && !allowNextContextMenu)
        return;
    allowNextContextMenu = false;

    QMenu menu(event->widget());

    QAction *a = menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()));
    a->setEnabled(m_streamItem->stream() && !m_streamItem->stream()->currentFrame().isNull());

    QMenu *ptzmenu = 0;
    if (camera().hasPtz())
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

    QAction *actClose = menu.addAction(tr("Close camera"), this, SLOT(close()));
    actClose->setEnabled(m_camera);

    menu.exec(event->screenPos());
    delete ptzmenu;
}

void LiveFeedItem::setBandwidthModeFromAction()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (!a || a->data().isNull() || !stream())
        return;

    int mode = a->data().toInt();
    stream()->setBandwidthMode(mode);
    stream()->setPaused(false);
}

/* Version (in LiveViewLayout) must be bumped for any change to
 * this format, and loadState must support old versions. */
void LiveFeedItem::saveState(QDataStream *data)
{
    Q_ASSERT(data);

    *data << m_camera;
    *data << (stream() ? stream()->bandwidthMode() : 0);
}

void LiveFeedItem::loadState(QDataStream *data, int version)
{
    Q_ASSERT(data);

    DVRCamera c;
    *data >> c;
    setCamera(c);

    if (version >= 1) {
        int bandwidth_mode = 0;
        *data >> bandwidth_mode;
        Q_ASSERT(stream());
        stream()->setBandwidthMode(bandwidth_mode);
    }
}

void LiveFeedItem::setCustomCursor(CustomCursor cursor)
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

void LiveFeedItem::setPtzEnabled(bool ptzEnabled)
{
    if (ptzEnabled == !m_ptz.isNull())
        return;

    if (ptzEnabled)
        m_ptz = CameraPtzControl::sharedObjectFor(camera());
    else
        m_ptz.clear();

    emit ptzChanged(m_ptz.data());
}

void LiveFeedItem::wheelEvent(QGraphicsSceneWheelEvent *event)
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

QMenu *LiveFeedItem::ptzMenu()
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

QList<QAction*> LiveFeedItem::bandwidthActions()
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

QPoint LiveFeedItem::globalPosForItem(QDeclarativeItem *item)
{
    QPoint pos = QCursor::pos();
    if (item)
    {
        QGraphicsView *view = item->scene()->views().value(0);
        Q_ASSERT(view && item->scene()->views().size() == 1);
        if (view)
            pos = view->mapToGlobal(view->mapFromScene(item->mapToScene(0, item->height())));
    }
    return pos;
}

void LiveFeedItem::showPtzMenu(QDeclarativeItem *sourceItem)
{
    QPoint pos = globalPosForItem(sourceItem);
    QMenu *menu = ptzMenu();
    menu->exec(pos);
    delete menu;
}

void LiveFeedItem::ptzPresetSave()
{
    if (!m_ptz)
        return;

    QGraphicsView *view = scene()->views().value(0);
    Q_ASSERT(view && scene()->views().size() == 1);

    QString re = QInputDialog::getText(view, tr("Save PTZ preset"), tr("Enter a name for the new PTZ preset:"));
    if (re.isEmpty())
        return;

    m_ptz->savePreset(-1, re);
}

void LiveFeedItem::ptzPresetWindow()
{
    if (!m_ptz)
        return;

    PtzPresetsWindow *window = new PtzPresetsWindow(m_ptz.data(), bcApp->mainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);

    QGraphicsView *view = scene()->views().value(0);
    Q_ASSERT(view && scene()->views().size() == 1);
    if (view)
    {
        QRect itemScreenRect = view->mapFromScene(mapToScene(QRectF(0, 0, width(), height()))).boundingRect();
        itemScreenRect.moveTopLeft(view->viewport()->mapToGlobal(itemScreenRect.topLeft()));

        window->move(itemScreenRect.right() - qRound(window->width() / 2.0),
                     itemScreenRect.top() + qMax(0, qRound((itemScreenRect.height() - window->height()) / 2.0)));
    }

    window->show();
}

void LiveFeedItem::showFpsMenu(QDeclarativeItem *sourceItem)
{
    QPoint pos = globalPosForItem(sourceItem);
    QMenu menu;
    QList<QAction*> actions = bandwidthActions();
    foreach (QAction *a, actions)
        a->setParent(&menu);
    menu.addActions(actions);
    menu.exec(pos);
}
