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

#include "PtzPresetsWindow.h"
#include "core/CameraPtzControl.h"
#include "core/PtzPresetsModel.h"

#include <QAction>
#include <QBoxLayout>
#include <QEvent>
#include <QInputDialog>
#include <QMenu>
#include <QToolBar>
#include <QTreeView>

PtzPresetsWindow::PtzPresetsWindow(CameraPtzControl *ptzControl, QWidget *parent)
    : QWidget(parent, Qt::Tool), m_ptz(ptzControl)
{
    connect(ptzControl, SIGNAL(destroyed()), SLOT(close()));

    resize(150, 200);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(3);

    m_presetsView = new QTreeView;
    m_presetsView->setRootIsDecorated(false);
    m_presetsView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_presetsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_presetsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(moveToPreset(QModelIndex)));
    connect(m_presetsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(presetsViewContextMenu(QPoint)));
    layout->addWidget(m_presetsView);

    m_presetsModel = new PtzPresetsModel(m_presetsView);
    m_presetsModel->setPtzController(ptzControl);
    m_presetsView->setModel(m_presetsModel);

	m_tb = new QToolBar(tr("Presets"));
	m_tb->setIconSize(QSize(20, 20));
	m_tb->setStyleSheet(QLatin1String("QToolBar { border: none; }"));

	m_newAction = m_tb->addAction(QIcon(QLatin1String(":/icons/plus.png")), tr("New Preset"), this, SLOT(newPreset()));
	m_tb->addSeparator();
	m_goToAction = m_tb->addAction(QIcon(QLatin1String(":/icons/tick.png")), tr("Go to Preset"), this, SLOT(moveToPreset()));
	m_renameAction = m_tb->addAction(QIcon(QLatin1String(":/icons/pencil.png")), tr("Rename Preset"), this, SLOT(renamePreset()));
	m_deleteAction = m_tb->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Delete Preset"), this, SLOT(deletePreset()));
	m_tb->addSeparator();
	m_refreshPresetAction = m_tb->addAction(QIcon(QLatin1String(":/icons/arrow-circle-double.png")), tr("Refresh Presets"), m_ptz, SLOT(updateInfo()));

	layout->addWidget(m_tb);
}

void PtzPresetsWindow::changeEvent(QEvent *event)
{
	if (event && event->type() == QEvent::LanguageChange)
		retranslateUI();

	QWidget::changeEvent(event);
}

void PtzPresetsWindow::newPreset()
{
    QString re = QInputDialog::getText(this, tr("Save PTZ preset"), tr("Enter a name for the new PTZ preset:"));
    if (re.isEmpty())
        return;

    m_ptz->savePreset(-1, re);
}

void PtzPresetsWindow::moveToPreset(const QModelIndex &idx)
{
    QModelIndex index = idx.isValid() ? idx : m_presetsView->currentIndex();
    if (!index.isValid())
        return;

    bool ok = false;
    int id = index.data(PtzPresetsModel::PresetIdRole).toInt(&ok);
    if (!ok || id < 0)
        return;

    m_ptz->moveToPreset(id);
}

void PtzPresetsWindow::updatePreset(const QModelIndex &idx)
{
    QModelIndex index = idx.isValid() ? idx : m_presetsView->currentIndex();
    if (!index.isValid())
        return;

    bool ok = false;
    int id = index.data(PtzPresetsModel::PresetIdRole).toInt(&ok);
    if (!ok || id < 0)
        return;

    m_ptz->updatePreset(id);
}

void PtzPresetsWindow::renamePreset(const QModelIndex &idx)
{
    QModelIndex index = idx.isValid() ? idx : m_presetsView->currentIndex();
    if (!index.isValid())
        return;

    m_presetsView->edit(index);
}

void PtzPresetsWindow::deletePreset(const QModelIndex &idx)
{
    QModelIndex index = idx.isValid() ? idx : m_presetsView->currentIndex();
    if (!index.isValid())
        return;

    bool ok = false;
    int id = index.data(PtzPresetsModel::PresetIdRole).toInt(&ok);
    if (!ok || id < 0)
        return;

    m_ptz->clearPreset(id);
}

void PtzPresetsWindow::presetsViewContextMenu(const QPoint &pos)
{
    QModelIndex idx = m_presetsView->indexAt(pos);
    if (!idx.isValid())
        return;

    QMenu menu;
    QAction *goTo = 0, *update = 0, *rename = 0, *remove = 0, *newPreset = 0;

    if (idx.isValid())
    {
        goTo = menu.addAction(tr("Go to Preset"));
        menu.setDefaultAction(goTo);
        update = menu.addAction(tr("Update preset"));
        menu.addSeparator();
        rename = menu.addAction(tr("Rename preset"));
        remove = menu.addAction(tr("Delete preset"));
    }
    else
    {
        newPreset = menu.addAction(tr("New Preset"));
        menu.addAction(tr("Refresh Presets"), m_ptz, SLOT(updateInfo()));
    }

    QAction *a = menu.exec(m_presetsView->mapToGlobal(pos));
    if (!a)
        return;

    if (a == goTo)
        moveToPreset(idx);
    else if (a == update)
        updatePreset(idx);
    else if (a == rename)
        renamePreset(idx);
    else if (a == remove)
        deletePreset(idx);
    else if (a == newPreset)
		this->newPreset();
}

void PtzPresetsWindow::retranslateUI()
{
	if (m_ptz->camera())
		setWindowTitle(tr("PTZ - %1").arg(m_ptz->camera()->data().displayName()));
	else
		setWindowTitle(tr("PTZ"));

	m_tb->setWindowTitle(tr("Presets"));

	m_newAction->setText(tr("New Preset"));
	m_goToAction->setText(tr("Go to Preset"));
	m_renameAction->setText(tr("Rename Preset"));
	m_deleteAction->setText(tr("Delete Preset"));
	m_refreshPresetAction->setText(tr("Refresh Presets"));

}
