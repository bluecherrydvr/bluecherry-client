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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

class QTabWidget;
class QDialogButtonBox;
class DVRServerRepository;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    enum OptionsPage
    {
        GeneralPage = 0,
        ServerPage
    };

    explicit OptionsDialog(DVRServerRepository *serverRepository, QWidget *parent = 0);
    ~OptionsDialog();

    void showPage(OptionsPage page);
    QWidget *pageWidget(OptionsPage page) const;

    static bool isDialogCreated() {return m_dialogCreated;};

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QTabWidget *m_tabWidget;
    QDialogButtonBox *m_buttons;

    static bool m_dialogCreated;
};

class OptionsDialogPage : public QWidget
{
    Q_OBJECT

public:
    OptionsDialogPage(QWidget *parent = 0) : QWidget(parent) { }

    virtual bool hasUnsavedChanges() const { Q_ASSERT(alwaysSaveChanges()); return false; }
    virtual bool alwaysSaveChanges() const { return false; }

public slots:
    virtual void saveChanges() = 0;
};

#endif // OPTIONSDIALOG_H
