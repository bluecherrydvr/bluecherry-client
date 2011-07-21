#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>
#include <QSharedPointer>
#include "core/DVRCamera.h"

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class QDataStream;
class CameraPtzControl;

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(CustomCursor RecordingState)

    Q_PROPERTY(DVRCamera camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraNameChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
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

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera ? m_camera.displayName() : QLatin1String(" "); }
    QString statusText() const { return m_statusText; }

    CustomCursor customCursor() const { return m_customCursor; }
    CameraPtzControl *ptz() const { return m_ptz.data(); }
    bool hasPtz() const { return m_camera ? m_camera.hasPtz() : false; }
    RecordingState recordingState() const { return m_camera ? RecordingState(m_camera.recordingState()) : NoRecording; }

    Q_INVOKABLE void saveState(QDataStream *stream);
    Q_INVOKABLE void loadState(QDataStream *stream);

public slots:
    void setCamera(const DVRCamera &camera);
    void clear() { setCamera(DVRCamera()); }

    void close();

    void openNewWindow();
    void openFullScreen();
    void saveSnapshot(const QString &file = QString());

    void setStatusText(const QString &text);
    void setCustomCursor(CustomCursor cursor);
    void setPtzEnabled(bool ptzEnabled);
    void togglePtzEnabled() { setPtzEnabled(!ptz()); }

    void showPtzMenu(QDeclarativeItem *sourceItem = 0);
    void ptzPresetSave();
    void ptzPresetWindow();

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);
    void statusTextChanged(const QString &statusText);
    void ptzChanged(CameraPtzControl *ptz);
    void hasPtzChanged();
    void recordingStateChanged();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

private slots:
    void cameraDataUpdated();
    void setIntervalFromAction();

private:
    DVRCamera m_camera;
    QString m_statusText;
    QSharedPointer<CameraPtzControl> m_ptz;
    CustomCursor m_customCursor;

    /* Caller is responsible for deleting */
    QMenu *ptzMenu();
};

#endif // LIVEFEEDITEM_H
