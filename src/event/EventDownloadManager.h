#ifndef EVENTDOWNLOADMANAGER_H
#define EVENTDOWNLOADMANAGER_H

#include <QObject>
#include <QQueue>

class QTimer;
class EventData;
class EventVideoDownload;

class EventDownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit EventDownloadManager(QObject *parent = 0);
    virtual ~EventDownloadManager();

    QString defaultFileName(const EventData &event) const;
    QString absoluteFileName(const QString &fileName) const;
    void updateLastSaveDirectory(const QString &fileName);

    void startEventDownload(const EventData &event, const QString &fileName);
    void startEventDownload(const EventData &event);
    void startMultipleEventDownloads(const QList<EventData> &events);

    QList<EventVideoDownload *> list() const { return m_eventVideoDownloadList; }

signals:
    void eventVideoDownloadAdded(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadRemoved(EventVideoDownload *eventVideoDownload);

private:
    QTimer *m_checkQueueTimer;
    QString m_lastSaveDirectory;
    QList<EventVideoDownload *> m_eventVideoDownloadList;
    QList<EventVideoDownload *> m_activeEventVideoDownloadList;
    QQueue<EventVideoDownload *> m_eventVideoDownloadQueue;

private slots:
    void checkQueue();
    void eventVideoDownloadFinished(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadDestroyed(QObject *destroyedObject);
};

#endif // EVENTDOWNLOADMANAGER_H
