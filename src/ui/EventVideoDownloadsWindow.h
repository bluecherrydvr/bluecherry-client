#ifndef EVENTVIDEODOWNLOADSWINDOW_H
#define EVENTVIDEODOWNLOADSWINDOW_H

#include <QMap>
#include <QFrame>

class QVBoxLayout;
class EventDownloadManager;
class EventVideoDownload;
class EventVideoDownloadWidget;

class EventVideoDownloadsWindow : public QFrame
{
    Q_OBJECT

public:
    explicit EventVideoDownloadsWindow(QWidget *parent = 0);
    virtual ~EventVideoDownloadsWindow();

    void setEventDownloadManager(EventDownloadManager *eventDownloadManager);

private slots:
    void eventVideoDownloadAdded(EventVideoDownload *eventVideoDownload);
    void eventVideoDownloadRemoved(EventVideoDownload *eventVideoDownload);

    void saveSettings();

private:
    QWeakPointer<EventDownloadManager> m_eventDownloadManager;
    QMap<EventVideoDownload *, EventVideoDownloadWidget *> m_downloadWidgets;
    QVBoxLayout *m_downloadLayout;

    void rebuildWidgets();
};

#endif // EVENTVIDEODOWNLOADSWINDOW_H
