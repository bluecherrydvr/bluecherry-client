#ifndef EVENTVIDEODOWNLOADWIDGET_H
#define EVENTVIDEODOWNLOADWIDGET_H

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;
class EventVideoDownload;

class EventVideoDownloadWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventVideoDownloadWidget(EventVideoDownload *eventVideoDownload, QWidget *parent = 0);
    virtual ~EventVideoDownloadWidget();

private slots:
    void closeClicked();
    void updateProgress();

private:
    QWeakPointer<EventVideoDownload> m_eventVideoDownload;
    QPushButton *m_closeButton;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QTimer *m_progressTimer;
};

#endif // EVENTVIDEODOWNLOADWIDGET_H
