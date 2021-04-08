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

#ifndef CAMERACONTAINERWIDGET_H
#define CAMERACONTAINERWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QStaticText>
#include "core/CameraPtzControl.h"
#include "core/LiveStream.h"


class DVRCamera;
class QMenu;
class DVRServerRepository;
class QLabel;

class CameraContainerWidget : public QFrame//QWidget
{
    Q_OBJECT
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
    enum AudioState {
        Disable = 0,
        Save = 1,
        Load = 2
    };

    explicit CameraContainerWidget(QWidget *parent = nullptr);
    QString cameraName() const;
    DVRCamera * camera() const { return m_camera.data(); }
    LiveStream *stream() const;
    CameraPtzControl *ptz() const { return m_ptz.data(); }
    bool hasHeightForWidth() const { return true; }
    void saveState(QDataStream *stream);
    void loadState(QDataStream *stream, int version);

public slots:
    void setCamera(DVRCamera *camera);
    void setServerRepository(DVRServerRepository *serverRepository);
    void saveSnapshot();
    void setPtzEnabled(bool ptzEnabled);
    void togglePtzEnabled() { setPtzEnabled(!ptz()); }
    void ptzPresetSave();
    void ptzPresetWindow();
    void showFpsMenu();
    void openNewWindow();
    void openFullScreen();
    void clear() { setCamera(0); }
    void setCustomCursor(CustomCursor cursor);

    void enableAudio();
    void disableAudio();
    void close();
signals:
    void cameraClosed(QWidget *widget);
    void cameraChanged(DVRCamera *camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);
    void ptzChanged(CameraPtzControl *ptz);
    void hasPtzChanged();
    void recordingStateChanged();
protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void cameraDataUpdated();
    void updateAudioState(enum AudioState state = Load);
    void setBandwidthModeFromAction();
    void serverRemoved(DVRServer *server);
    void set_main_stream();
    void set_sub_stream();
    void updateFrame()
    {
        update();
    }
private:
    QWeakPointer<DVRCamera> m_camera;
    QSharedPointer<CameraPtzControl> m_ptz;
    DVRServerRepository *m_serverRepository;
    CustomCursor m_customCursor;
    QSharedPointer<LiveStream> m_stream;
    QStaticText m_cameraname;
    QStaticText m_streamstatus;
    /* Caller is responsible for deleting */
    QMenu *ptzMenu();
    QList<QAction*> bandwidthActions();
    CameraPtzControl::Movement moveForPosition(int x, int y);
    QString statusOverlayMessage();
    void drawHeader(QPainter *p, const QRect &r);
    void initStaticText();
};

#endif // CAMERACONTAINERWIDGET_H
