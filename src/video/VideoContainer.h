#ifndef VIDEOCONTAINER_H
#define VIDEOCONTAINER_H

#include "core/BluecherryApp.h"
#include <QFrame>
#include <QMouseEvent>
#include <QSettings>
#include <QDebug>

class VideoContainer : public QFrame
{
    Q_OBJECT

public:
    VideoContainer(QWidget *inner = 0, QWidget *parent = 0)
        : QFrame(parent), innerWidget(0)
    {
        setMinimumSize(320, 240);
        setFrameStyle(QFrame::Sunken | QFrame::Panel);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::black);
        p.setColor(QPalette::WindowText, Qt::white);
        setPalette(p);

        if (inner)
            setInnerWidget(inner);
    }

    void setInnerWidget(QWidget *inner)
    {
        if (innerWidget)
        {
            innerWidget->hide();
            innerWidget->deleteLater();
        }

        innerWidget = inner;

        if (innerWidget)
        {
            innerWidget->setParent(this);
            innerWidget->setGeometry(contentsRect());
            innerWidget->show();
        }
    }

public slots:
    void setFullScreen(bool on)
    {
        if (on)
        {
            setWindowFlags(windowFlags() | Qt::Window);
            m_normalFrameStyle = frameStyle();
            setFrameStyle(QFrame::NoFrame);
            showFullScreen();
        }
        else
        {
            setWindowFlags(windowFlags() & ~Qt::Window);
            setFrameStyle(m_normalFrameStyle);
            showNormal();
        }

        QSettings settings;
        if (settings.value(QLatin1String("ui/disableScreensaver/onFullscreen")).toBool())
            bcApp->setScreensaverInhibited(on);
    }

    void toggleFullScreen()
    {
        setFullScreen(!isFullScreen());
    }

protected:
    QWidget *innerWidget;
    int m_normalFrameStyle;

    virtual void resizeEvent(QResizeEvent *ev)
    {
        QFrame::resizeEvent(ev);
        if (innerWidget)
            innerWidget->setGeometry(contentsRect());
    }

    virtual void mouseDoubleClickEvent(QMouseEvent *ev)
    {
        ev->accept();
        toggleFullScreen();
    }

    virtual void keyPressEvent(QKeyEvent *ev)
    {
        if (ev->modifiers() != 0)
            return;

        switch (ev->key())
        {
        case Qt::Key_Escape:
            setFullScreen(false);
            break;

        default:
            return;
        }

        ev->accept();
    }
};


#endif // VIDEOCONTAINER_H
