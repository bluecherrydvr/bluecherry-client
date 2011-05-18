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

private slots:
    void ssUpdateForNever();
    void ssUpdateForOthers(bool checked);

private:
    QCheckBox *m_eventsPauseLive, *m_closeToTray, *m_liveHwAccel;
    QCheckBox *m_ssFullscreen, *m_ssVideo, *m_ssNever;
};

#endif // OPTIONSGENERALPAGE_H
