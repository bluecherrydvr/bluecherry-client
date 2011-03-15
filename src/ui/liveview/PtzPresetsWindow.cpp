#include "PtzPresetsWindow.h"
#include "core/CameraPtzControl.h"
#include "core/PtzPresetsModel.h"
#include <QTreeView>
#include <QBoxLayout>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QInputDialog>

PtzPresetsWindow::PtzPresetsWindow(CameraPtzControl *ptzControl, QWidget *parent)
    : QWidget(parent, Qt::Tool), m_ptz(ptzControl)
{
    connect(ptzControl, SIGNAL(destroyed()), SLOT(close()));

    setWindowTitle(tr("PTZ - %1").arg(m_ptz->camera().displayName()));
    resize(140, 200);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

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

    QToolBar *tb = new QToolBar;
    tb->setIconSize(QSize(20, 20));
    tb->setStyleSheet(QLatin1String("QToolBar { border: none; }"));

    tb->addAction(QIcon(QLatin1String(":/icons/plus.png")), tr("New Preset"), this, SLOT(newPreset()));
    tb->addSeparator();
    tb->addAction(QIcon(QLatin1String(":/icons/tick.png")), tr("Go to Preset"), this, SLOT(moveToPreset()));
    tb->addAction(QIcon(QLatin1String(":/icons/pencil.png")), tr("Rename Preset"), this, SLOT(renamePreset()));
    tb->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Delete Preset"), this, SLOT(deletePreset()));

    layout->addWidget(tb);
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
    QAction *goTo = 0, *rename = 0, *remove = 0, *newPreset = 0;

    if (idx.isValid())
    {
        goTo = menu.addAction(tr("Go to Preset"));
        menu.setDefaultAction(goTo);
        menu.addSeparator();
        rename = menu.addAction(tr("Rename preset"));
        remove = menu.addAction(tr("Delete preset"));
    }
    else
    {
        newPreset = menu.addAction(tr("New Preset"));
    }

    QAction *a = menu.exec(m_presetsView->mapToGlobal(pos));
    if (!a)
        return;

    if (a == goTo)
        moveToPreset(idx);
    else if (a == rename)
        renamePreset(idx);
    else if (a == remove)
        deletePreset(idx);
    else if (a == newPreset)
        this->newPreset();
}
