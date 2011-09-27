#ifndef MACSPLITTER_H
#define MACSPLITTER_H

#include <QSplitter>

class MacSplitterHandle : public QSplitterHandle
{
public:
    MacSplitterHandle(Qt::Orientation o, QSplitter *parent);
    ~MacSplitterHandle();

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual bool eventFilter(QObject *, QEvent *);

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
