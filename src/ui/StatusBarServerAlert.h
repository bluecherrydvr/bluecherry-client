#ifndef STATUSBARSERVERALERT_H
#define STATUSBARSERVERALERT_H

#include <QWidget>

class QLabel;

class StatusBarServerAlert : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarServerAlert(QWidget *parent = 0);

private slots:
    void updateAlert();

protected:
    virtual void mousePressEvent(QMouseEvent *);

private:
    QLabel *alertText;
};

#endif // STATUSBARSERVERALERT_H
