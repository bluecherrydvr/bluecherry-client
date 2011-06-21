#ifndef EVENTVIDEODOWNLOAD_H
#define EVENTVIDEODOWNLOAD_H

#include <QObject>
#include <QFutureWatcher>
#include <QTimer>

class QProgressDialog;
class MediaDownload;

class EventVideoDownload : public QObject
{
    Q_OBJECT

public:
    explicit EventVideoDownload(QObject *parent = 0);
    ~EventVideoDownload();

    void setMediaDownload(MediaDownload *download);
    void setFilePath(const QString &path);

    void start(QWidget *parentWindow = 0);

private slots:
    void startCopy();
    void copyFinished();
    void updateBufferProgress();
    void cancel();

private:
    QProgressDialog *m_dialog;
    QFutureWatcher<bool> *m_futureWatch;
    MediaDownload *m_media;
    QString m_tempFilePath;
    QString m_finalPath;
    QTimer m_progressTimer;
};

#endif // EVENTVIDEODOWNLOAD_H
