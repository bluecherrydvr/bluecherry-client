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
    virtual ~LiveViewArea();

    LiveViewLayout *layout() const { return m_layout; }

    QSize sizeHint() const;

    bool isHardwareAccelerated() const;

public slots:
    void addCamera(const DVRCamera &camera);
    void updateGeometry() { m_sizeHint = QSize(); QDeclarativeView::updateGeometry(); }

    void settingsChanged();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

private slots:
    void setViewportHack();

private:
    LiveViewLayout *m_layout;
    mutable QSize m_sizeHint;
};

#endif // LIVEVIEWAREA_H
