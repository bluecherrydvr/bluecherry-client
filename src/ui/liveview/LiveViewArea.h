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

    QSize sizeHint() const;

public slots:
    void addCamera(const DVRCamera &camera);
    void updateGeometry() { m_sizeHint = QSize(); QDeclarativeView::updateGeometry(); }

private:
    LiveViewLayout *m_layout;
    mutable QSize m_sizeHint;
};

#endif // LIVEVIEWAREA_H
