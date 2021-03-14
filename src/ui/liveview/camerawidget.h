#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include "core/LiveStream.h"

class CameraWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();
    QSize sizeHint() const;
    bool hasHeightForWidth() const { return true; }
    int heightForWidth(int w) const;
    LiveStream * stream() const { return m_stream.data(); }
    void setStream(QSharedPointer<LiveStream> stream);

protected:
    void paintEvent(QPaintEvent *event);
signals:
private slots:
    void updateFrame()
    {
        update();
    }
private:
    QSharedPointer<LiveStream> m_stream;
    QSize m_framesize;
};

#endif // CAMERAWIDGET_H
