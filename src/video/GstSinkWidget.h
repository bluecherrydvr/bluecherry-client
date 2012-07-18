#ifndef GSTSINKWIDGET_H
#define GSTSINKWIDGET_H

#include <QFrame>
#include <QMutex>
#include <gst/gst.h> // needed for GstFlowReturn

typedef struct _GstAppSink GstAppSink;
typedef void* gpointer;

class GstSinkWidget : public QFrame
{
    Q_OBJECT

public:
    explicit GstSinkWidget(QWidget *parent = 0);
    ~GstSinkWidget();

    GstElement *createElement();
    void destroyElement();
    GstElement *gstElement() const { return GST_ELEMENT(m_element); }
    QImage currentFrame();

    virtual QSize sizeHint() const;

    void setViewport(QWidget *viewport);

public slots:
    void setFullScreen(bool on);
    void toggleFullScreen() { setFullScreen(!isFullScreen()); }

    void setOverlayMessage(const QString &message);
    void clearOverlayMessage() { setOverlayMessage(QString()); }

private slots:
    void setBufferStatus(int percent);
    void settingsChanged();

protected:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual bool eventFilter(QObject *, QEvent *);

private:
    QWidget *m_viewport;
    GstAppSink *m_element;
    GstBuffer *m_framePtr;
    QMutex m_frameLock;
    QString m_overlayMsg;
    int m_frameWidth, m_frameHeight;
    int m_normalFrameStyle;

    static void wrapEos(GstAppSink *sink, gpointer user_data);
    static GstFlowReturn wrapNewBuffer(GstAppSink *sink, gpointer user_data);
    static GstFlowReturn wrapNewPreroll(GstAppSink *sink, gpointer user_data);

    void updateFrame(GstBuffer *buffer);

    void endOfStream();
    GstFlowReturn newPreroll();
    GstFlowReturn newBuffer();
};

#endif // GSTSINKWIDGET_H
