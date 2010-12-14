#ifndef OPTIONSGENERALPAGE_H
#define OPTIONSGENERALPAGE_H

#include "OptionsDialog.h"

class QCheckBox;

class OptionsGeneralPage : public OptionsDialogPage
{
    Q_OBJECT

public:
    explicit OptionsGeneralPage(QWidget *parent = 0);

    virtual bool alwaysSaveChanges() const { return true; }
    virtual void saveChanges();

private:
    QCheckBox *m_eventsPauseLive, *m_closeToTray;
};

#endif // OPTIONSGENERALPAGE_H
