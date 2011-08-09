#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>

class MacSplitterHandle : public QSplitterHandle
{
public:
    MacSplitterHandle(Qt::Orientation o, QSplitter *parent);

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);

private:
    bool isMouseGrabbed;
};

class MacSplitter : public QSplitter
{
public:
    explicit MacSplitter(Qt::Orientation o, QWidget *parent = 0)
        : QSplitter(o, parent)
    {
    }

protected:
    virtual QSplitterHandle *createHandle()
    {
        return new MacSplitterHandle(orientation(), this);
    }
};

#endif // MACSPLITTER_H
