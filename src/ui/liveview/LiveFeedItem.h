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

public:
    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera.displayName(); }

public slots:
    void setCamera(const DVRCamera &camera);

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void cameraDataUpdated();

private:
    DVRCamera m_camera;
};

#endif // LIVEFEEDITEM_H
