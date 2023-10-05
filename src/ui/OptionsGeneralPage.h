/*
 * Copyright 2010-2019 Bluecherry, LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPTIONSGENERALPAGE_H
#define OPTIONSGENERALPAGE_H

#include "OptionsDialog.h"

class QCheckBox;
class QComboBox;

class OptionsGeneralPage : public OptionsDialogPage
{
    Q_OBJECT

public:
    explicit OptionsGeneralPage(QWidget *parent = 0);

    virtual bool alwaysSaveChanges() const { return true; }
    virtual void saveChanges();

private slots:
    /*void ssUpdateForNever();
    void ssUpdateForOthers(bool checked);*/
    void updateStartup(bool on);

private:
    QCheckBox *m_eventsPauseLive, *m_closeToTray, *m_vaapiDecodingAcceleration,
                    *m_deinterlace, *m_updateNotifications, *m_thumbnails,
                    *m_session, *m_fullScreen, *m_startup, *m_hidetoolbar /*,
                    *m_ssFullscreen, *m_ssVideo, *m_ssNever*/;

	QComboBox *m_languages;
    QComboBox *m_mpvvo;

    void fillLanguageComboBox();
    void fillMpvVOComboBox();
};

#endif // OPTIONSGENERALPAGE_H
