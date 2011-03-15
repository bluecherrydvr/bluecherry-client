#include "PtzPresetsWindow.h"
#include "core/CameraPtzControl.h"
#include "core/PtzPresetsModel.h"
#include <QTreeView>
#include <QBoxLayout>
#include <QMenu>
#include <QAction>

PtzPresetsWindow::PtzPresetsWindow(CameraPtzControl *ptzControl, QWidget *parent)
    : QWidget(parent, Qt::Tool), m_ptz(ptzControl)
{
    connect(ptzControl, SIGNAL(destroyed()), SLOT(close()));

    setWindowTitle(tr("PTZ - %1").arg(m_ptz->camera().displayName()));

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

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
}

void PtzPresetsWindow::moveToPreset(const QModelIndex &index)
{
    bool ok = false;
    int id = index.data(PtzPresetsModel::PresetIdRole).toInt(&ok);
    if (!ok || id < 0)
        return;

    m_ptz->moveToPreset(id);
}

void PtzPresetsWindow::presetsViewContextMenu(const QPoint &pos)
{
    QModelIndex idx = m_presetsView->indexAt(pos);
    if (!idx.isValid())
        return;

    QMenu menu;
    QAction *goTo = menu.addAction(tr("Go to Preset"));
    menu.setDefaultAction(goTo);
    menu.addSeparator();
    QAction *rename = menu.addAction(tr("Rename preset"));
    QAction *remove = menu.addAction(tr("Delete preset"));

    QAction *a = menu.exec(m_presetsView->mapToGlobal(pos));
    if (!a)
        return;

    if (a == goTo)
        moveToPreset(idx);
    else if (a == rename)
        m_presetsView->edit(idx);
    else if (a == remove)
        m_ptz->clearPreset(idx.data(PtzPresetsModel::PresetIdRole).toInt());
}
