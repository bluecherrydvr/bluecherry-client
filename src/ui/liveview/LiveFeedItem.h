#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>
#include "core/DVRCamera.h"

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class QDataStream;
class CameraPtzControl;

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(CustomCursor)

    Q_PROPERTY(DVRCamera camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraNameChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(CustomCursor customCursor READ customCursor WRITE setCustomCursor)
    Q_PROPERTY(CameraPtzControl* ptz READ ptz NOTIFY ptzChanged)

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

    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera ? m_camera.displayName() : QLatin1String(" "); }
    QString statusText() const { return m_statusText; }

    CustomCursor customCursor() const { return m_customCursor; }
    CameraPtzControl *ptz() const { return m_ptz; }

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

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);
    void statusTextChanged(const QString &statusText);
    void ptzChanged(CameraPtzControl *ptz);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void cameraDataUpdated();

private:
    DVRCamera m_camera;
    QString m_statusText;
    CameraPtzControl *m_ptz;
    CustomCursor m_customCursor;
};

#endif // LIVEFEEDITEM_H
