#ifndef GSTSINKWIDGET_H
#define GSTSINKWIDGET_H

#include <QGLWidget>
#include <QMutex>
#include <gst/gst.h> // needed for GstFlowReturn

typedef struct _GstAppSink GstAppSink;
typedef void* gpointer;

class GstSinkWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit GstSinkWidget(QWidget *parent = 0);
    ~GstSinkWidget();

    GstAppSink *gstElement() const { return m_element; }

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *ev);

private:
    GstAppSink *m_element;
    GstBuffer *m_framePtr;
    QMutex m_frameLock;
    int m_frameWidth, m_frameHeight;

    static void wrapEos(GstAppSink *sink, gpointer user_data);
    static GstFlowReturn wrapNewBuffer(GstAppSink *sink, gpointer user_data);
    static GstFlowReturn wrapNewPreroll(GstAppSink *sink, gpointer user_data);

    void updateFrame(GstBuffer *buffer);

    void endOfStream();
    GstFlowReturn newPreroll();
    GstFlowReturn newBuffer();
};

#endif // GSTSINKWIDGET_H
