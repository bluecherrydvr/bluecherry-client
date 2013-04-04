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

#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "camera/DVRCamera.h"
#include "core/CameraPtzControl.h"

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class QDataStream;
class LiveStreamItem;

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(CustomCursor RecordingState)

    Q_PROPERTY(LiveStreamItem* streamItem READ streamItem WRITE setStreamItem)
    Q_PROPERTY(LiveStream* stream READ stream NOTIFY cameraChanged)
    Q_PROPERTY(DVRCamera camera READ camera WRITE setCamera NOTIFY cameraChanged)

    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraNameChanged)
    Q_PROPERTY(CustomCursor customCursor READ customCursor WRITE setCustomCursor)
    Q_PROPERTY(CameraPtzControl* ptz READ ptz NOTIFY ptzChanged)
    Q_PROPERTY(bool hasPtz READ hasPtz NOTIFY hasPtzChanged)
    Q_PROPERTY(RecordingState recordingState READ recordingState NOTIFY recordingStateChanged)

public:
    enum CustomCursor {
        DefaultCursor = 0,
        MoveCursorN,
        MoveCursorS,
        MoveCursorW,
        MoveCursorE,
        MoveCursorNW,
        MoveCursorNE,
        MoveCursorSW,
        MoveCursorSE
    };

    /* Duplicated with regrets from DVRCamera for the benefit of QML/QMetaObject */
    enum RecordingState {
        NoRecording = 0,
        Continuous,
        MotionInactive,
        MotionActive
    };

    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

    LiveStreamItem *streamItem() const { return m_streamItem; }
    void setStreamItem(LiveStreamItem *item);

    LiveStream *stream() const;

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera ? m_camera.displayName() : QLatin1String(" "); }

    CustomCursor customCursor() const { return m_customCursor; }
    CameraPtzControl *ptz() const { return m_ptz.data(); }
    bool hasPtz() const { return m_camera ? m_camera.hasPtz() : false; }
    RecordingState recordingState() const { return m_camera ? RecordingState(m_camera.recordingState()) : NoRecording; }

    Q_INVOKABLE void saveState(QDataStream *stream);
    Q_INVOKABLE void loadState(QDataStream *stream, int version);

public slots:
    void setCamera(const DVRCamera &camera);
    void clear() { setCamera(DVRCamera()); }

    void close();

    void openNewWindow();
    void openFullScreen();
    void saveSnapshot(const QString &file = QString());

    void setCustomCursor(CustomCursor cursor);
    void setPtzEnabled(bool ptzEnabled);
    void togglePtzEnabled() { setPtzEnabled(!ptz()); }

    void showPtzMenu(QDeclarativeItem *sourceItem = 0);
    void ptzPresetSave();
    void ptzPresetWindow();

    void showFpsMenu(QDeclarativeItem *sourceItem = 0);

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);
    void ptzChanged(CameraPtzControl *ptz);
    void hasPtzChanged();
    void recordingStateChanged();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

private slots:
    void cameraDataUpdated();
    void setBandwidthModeFromAction();

private:
    LiveStreamItem *m_streamItem;
    DVRCamera m_camera;
    QSharedPointer<CameraPtzControl> m_ptz;
    CustomCursor m_customCursor;

    /* Caller is responsible for deleting */
    QMenu *ptzMenu();
    QList<QAction*> bandwidthActions();

    QPoint globalPosForItem(QDeclarativeItem *item);
};

#endif // LIVEFEEDITEM_H
