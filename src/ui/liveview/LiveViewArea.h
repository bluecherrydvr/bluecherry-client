#ifndef LIVEVIEWAREA_H
#define LIVEVIEWAREA_H

#include <QDeclarativeView>

class LiveViewLayout;

class LiveViewArea : public QDeclarativeView
{
    Q_OBJECT

public:
    explicit LiveViewArea(QWidget *parent = 0);

private:
    LiveViewLayout *m_layout;
};

#endif // LIVEVIEWAREA_H
