#ifndef CAMERACONTAINERWIDGET_H
#define CAMERACONTAINERWIDGET_H

#include <QWidget>
#include <QFrame>
#include "core/CameraPtzControl.h"
#include "core/LiveStream.h"

class CameraWidget;
class DVRCamera;
class QMenu;
class DVRServerRepository;

class CameraContainerWidget : public QFrame//QWidget
{
    Q_OBJECT
public:
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

    void enableAudio();
    void disableAudio();
    void close();
signals:

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
private slots:
    void updateAudioState(enum AudioState state = Load);
    void setBandwidthModeFromAction();
    void serverRemoved(DVRServer *server);

private:
    QWeakPointer<DVRCamera> m_camera;
    QSharedPointer<CameraPtzControl> m_ptz;
    CameraWidget *m_cameraview;
    DVRServerRepository *m_serverRepository;
    /* Caller is responsible for deleting */
    QMenu *ptzMenu();
    QList<QAction*> bandwidthActions();
};

#endif // CAMERACONTAINERWIDGET_H
