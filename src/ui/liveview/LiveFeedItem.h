#ifndef LIVEFEEDITEM_H
#define LIVEFEEDITEM_H

#include <QDeclarativeItem>
#include "core/DVRCamera.h"

/* Base of LiveFeed.qml, used to implement some features that are currently missing in pure QML. */

class QDataStream;

class LiveFeedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(DVRCamera camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraNameChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit LiveFeedItem(QDeclarativeItem *parent = 0);

    DVRCamera camera() const { return m_camera; }
    QString cameraName() const { return m_camera ? m_camera.displayName() : QLatin1String(" "); }
    QString statusText() const { return m_statusText; }

    Q_INVOKABLE void saveState(QDataStream *stream);
    Q_INVOKABLE void loadState(QDataStream *stream);

public slots:
    void setCamera(const DVRCamera &camera);
    void clear() { setCamera(DVRCamera()); }

    void close();

    void openNewWindow();
    void saveSnapshot(const QString &file = QString());

    void setStatusText(const QString &text);

signals:
    void cameraChanged(const DVRCamera &camera);
    void cameraNameChanged(const QString &cameraName);
    void pausedChanged(bool isPaused);
    void statusTextChanged(const QString &statusText);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private slots:
    void cameraDataUpdated();

private:
    DVRCamera m_camera;
    QString m_statusText;
};

#endif // LIVEFEEDITEM_H
