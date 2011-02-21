#ifndef LIVEVIEWAREA_H
#define LIVEVIEWAREA_H

#include <QDeclarativeView>

class LiveViewLayout;
class DVRCamera;

class LiveViewArea : public QDeclarativeView
{
    Q_OBJECT

public:
    explicit LiveViewArea(QWidget *parent = 0);

    LiveViewLayout *layout() const { return m_layout; }

public slots:
    void addCamera(const DVRCamera &camera);

private:
    LiveViewLayout *m_layout;
};

#endif // LIVEVIEWAREA_H
