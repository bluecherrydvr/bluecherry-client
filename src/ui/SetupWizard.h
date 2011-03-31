#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWizard>

class SetupWizard : public QWizard
{
    Q_OBJECT

public:
    SetupWizard(QWidget *parent = 0);

public slots:
    void skip();

protected:
    virtual bool validateCurrentPage();

private:
    bool skipFlag;
};

#endif // SETUPWIZARD_H
