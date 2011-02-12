#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>
#include "core/DVRCamera.h"

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(DVRCamera camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraNameChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

public:
    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera ? m_camera.displayName() : QLatin1String(" "); }
    bool isPaused() const { return false; }

public slots:
    void setCamera(const DVRCamera &camera);
    void clear() { setCamera(DVRCamera()); }

    void close();

    void setPaused(bool paused = true);
    void togglePaused() { setPaused(!isPaused()); }

    void openNewWindow();
    void saveSnapshot(const QString &file = QString());

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void cameraDataUpdated();

private:
    DVRCamera m_camera;
};

#endif // LIVEFEEDITEM_H
