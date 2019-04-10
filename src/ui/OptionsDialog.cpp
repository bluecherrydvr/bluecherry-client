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

#include "OptionsDialog.h"
#include "OptionsGeneralPage.h"
#include "OptionsServerPage.h"
#include <QBoxLayout>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>

bool OptionsDialog::m_dialogCreated = false;

OptionsDialog::~OptionsDialog()
{
    m_dialogCreated = false;
}

OptionsDialog::OptionsDialog(DVRServerRepository *serverRepository, QWidget *parent)
    : QDialog(parent)
{
    //setModal(true);
    m_dialogCreated = true; // OptionsDialog is modeless now, preventing creation of multiple dialogs simultaneously

    setWindowTitle(tr("Bluecherry - Options"));

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_tabWidget = new QTabWidget;
    layout->addWidget(m_tabWidget);

    m_buttons = new QDialogButtonBox();
    connect(m_buttons->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), SLOT(close()));
    layout->addWidget(m_buttons);

    m_tabWidget->addTab(new OptionsGeneralPage(), tr("General"));
    m_tabWidget->addTab(new OptionsServerPage(serverRepository), tr("DVR Servers"));
}

void OptionsDialog::showPage(OptionsPage page)
{
    m_tabWidget->setCurrentIndex((int)page);
}

QWidget *OptionsDialog::pageWidget(OptionsPage page) const
{
    return m_tabWidget->widget((int)page);
}

void OptionsDialog::closeEvent(QCloseEvent *event)
{
    bool saveChanges = false;

    /* Prompt the user to save changes if any are unsaved, but prompt only once */
    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
        OptionsDialogPage *pageWidget = qobject_cast<OptionsDialogPage*>(m_tabWidget->widget(i));
        if (!pageWidget)
            continue;

        bool always = pageWidget->alwaysSaveChanges();

        if (always || pageWidget->hasUnsavedChanges())
        {
            if (always || saveChanges)
            {
                pageWidget->saveChanges();
                continue;
            }

            QMessageBox msg(QMessageBox::Question, tr("Options"), tr("Do you want to save your changes?"),
                            QMessageBox::NoButton, this);

            msg.addButton(QMessageBox::Save);
            msg.addButton(QMessageBox::Discard);
            msg.addButton(QMessageBox::Cancel);

            switch (msg.exec())
            {
            case QMessageBox::Save:
                saveChanges = true;
                pageWidget->saveChanges();
                break;
            case QMessageBox::Discard:
                event->accept();
                return;
            case QMessageBox::Cancel:
                event->ignore();
                return;
            }
        }
    }

    event->accept();
}
