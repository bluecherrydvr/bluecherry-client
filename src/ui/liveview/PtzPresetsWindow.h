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

#ifndef PTZPRESETSWINDOW_H
#define PTZPRESETSWINDOW_H

#include <QWidget>
#include <QModelIndex>

class CameraPtzControl;
class QAction;
class QModelIndex;
class QToolBar;
class QTreeView;
class PtzPresetsModel;

class PtzPresetsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PtzPresetsWindow(CameraPtzControl *ptzControl, QWidget *parent = 0);

protected:
	virtual void changeEvent(QEvent *event);

private slots:
    void newPreset();
    void moveToPreset(const QModelIndex &index = QModelIndex());
    void updatePreset(const QModelIndex &index = QModelIndex());
    void renamePreset(const QModelIndex &index = QModelIndex());
    void deletePreset(const QModelIndex &index = QModelIndex());
    void presetsViewContextMenu(const QPoint &pos);

private:
    CameraPtzControl *m_ptz;
    PtzPresetsModel *m_presetsModel;

	QAction *m_newAction;
	QAction *m_goToAction;
	QAction *m_renameAction;
	QAction *m_deleteAction;
	QAction *m_refreshPresetAction;
	QTreeView *m_presetsView;
	QToolBar *m_tb;

	void retranslateUI();
};

#endif // PTZPRESETSWINDOW_H
