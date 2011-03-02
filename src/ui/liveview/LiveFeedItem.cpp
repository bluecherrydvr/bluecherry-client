#include "LiveFeedItem.h"
#include "MJpegFeedItem.h"
#include "core/BluecherryApp.h"
#include "LiveViewWindow.h"
#include "ui/MainWindow.h"
#include "utils/FileUtils.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QGraphicsScene>
#include <QGraphicsView>

LiveFeedItem::LiveFeedItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
}

void LiveFeedItem::setCamera(const DVRCamera &camera)
{
    if (camera == m_camera)
        return;

    MJpegFeedItem *mjpeg = findChild<MJpegFeedItem*>(QLatin1String("mjpegFeed"));
    Q_ASSERT(mjpeg);
    if (!mjpeg)
        return;

    if (m_camera)
    {
        static_cast<QObject*>(m_camera)->disconnect(this);
        mjpeg->clear();
    }

    m_camera = camera;

    if (m_camera)
        connect(m_camera, SIGNAL(dataUpdated()), SLOT(cameraDataUpdated()));

    emit cameraChanged(camera);
    cameraDataUpdated();
}

void LiveFeedItem::setPaused(bool paused)
{
}

void LiveFeedItem::cameraDataUpdated()
{
    emit cameraNameChanged(cameraName());

    MJpegFeedItem *mjpeg = findChild<MJpegFeedItem*>(QLatin1String("mjpegFeed"));
    if (mjpeg)
    {
        QSharedPointer<MJpegStream> nstream = m_camera.mjpegStream();
        if (nstream != mjpeg->stream())
            mjpeg->setStream(m_camera.mjpegStream());
    }
}

void LiveFeedItem::setStatusText(const QString &text)
{
    m_statusText = text;
    emit statusTextChanged(m_statusText);
}

void LiveFeedItem::openNewWindow()
{
    LiveViewWindow::openWindow(bcApp->mainWindow, camera())->show();
}

void LiveFeedItem::close()
{
    bool closeFeedItem = parentItem()->metaObject()->invokeMethod(parentItem(), "removeItem", Q_ARG(QDeclarativeItem*, this));
    Q_ASSERT(closeFeedItem);
}

void LiveFeedItem::saveSnapshot(const QString &ifile)
{
    if (!m_camera)
        return;

    /* Grab the current frame, so the user gets what they expect regardless of the time taken by the dialog */
    QPixmap frame = m_camera.mjpegStream()->currentFrame();
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
    }

    if (!frame.save(file, "jpeg"))
    {
        QMessageBox::critical(window, tr("Snapshot Error"), tr("An error occurred while saving the snapshot image."),
                              QMessageBox::Ok);
        return;
    }
}

void LiveFeedItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

    QMenu menu(event->widget());

    menu.addAction(tr("Snapshot"), this, SLOT(saveSnapshot()))->setEnabled(m_camera && !m_camera.mjpegStream()->currentFrame().isNull());
    menu.addSeparator();

    QAction *a = menu.addAction(isPaused() ? tr("Paused") : tr("Pause"), this, SLOT(togglePaused()));
    a->setCheckable(true);
    a->setChecked(isPaused());
    a->setEnabled(m_camera && (m_camera.mjpegStream() || isPaused()));

    menu.addSeparator();
    menu.addAction(tr("Open in window"), this, SLOT(openNewWindow()));
    //menu.addAction(!isFullScreen() ? tr("Open as fullscreen") : tr("Exit fullscreen"), this, SLOT(toggleFullScreen()));
    menu.addSeparator();

    QAction *actClose = menu.addAction(tr("Close camera"), this, SLOT(close()));
    actClose->setEnabled(m_camera);

    menu.exec(event->screenPos());
}
