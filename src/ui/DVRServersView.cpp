/*
 * Copyright 2010-2013 Bluecherry
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

#include "DVRServersView.h"
#include "DVRServersModel.h"
#include "OptionsDialog.h"
#include "OptionsServerPage.h"
#include "ServerConfigWindow.h"
#include "server/DVRServer.h"
#include "core/BluecherryApp.h"
#include "MainWindow.h"
#include "liveview/LiveViewWindow.h"
#include "liveview/LiveViewArea.h"
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>

DVRServersView::DVRServersView(QWidget *parent)
    : QTreeView(parent)
{
    header()->setVisible(false);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setDragEnabled(true);
    setAnimated(true);

    DVRServersModel *model = new DVRServersModel(bcApp->serverRepository(), this);
    model->setOfflineDisabled(true);
    setModel(model);

    /* We only show the server name in this list; hide other columns */
    for (int i = 1, n = model->columnCount(); i < n; ++i)
        header()->setSectionHidden(i, true);
}

void DVRServersView::setModel(QAbstractItemModel *m)
{
    if (model())
        model()->disconnect(this);

    QTreeView::setModel(m);

    connect(m, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
}

DVRServer *DVRServersView::currentServer() const
{
    QModelIndex c = currentIndex();
    DVRServer *re = 0;

    while (c.isValid())
    {
        re = c.data(DVRServersModel::DVRServerRole).value<DVRServer*>();
        if (re)
            break;
        c = c.parent();
    }

    return re;
}

void DVRServersView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    DVRServer *server = index.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
    DVRCamera *camera = index.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();

    /* For servers, we prefer the checkable menu over the server controls menu,
     * due to the way they're used specifically. Somewhat unclean. */
    if (server && !(index.flags() & Qt::ItemIsUserCheckable))
    {
        /* Use the shared context menu for servers */
        QMenu *menu = bcApp->mainWindow->serverMenu(server);
        menu->exec(event->globalPos());
        return;
    }

    QMenu menu(this);

    QAction *aOptions = 0, *aAddServer = 0;
    QAction *aSelectOnly = 0, *aSelectElse = 0;
    QAction *aAddFeed = 0, *aOpenWin = 0, *aOpenFull = 0, *aCamRename = 0;

    if (index.isValid())
    {
        if (index.flags() & Qt::ItemIsUserCheckable)
        {
            aSelectOnly = menu.addAction(tr("Select only this"));
            aSelectElse = menu.addAction(tr("Select everything else"));
            if (!(index.flags() & Qt::ItemIsEnabled))
            {
                aSelectOnly->setEnabled(false);
                aSelectElse->setEnabled(false);
            }
            menu.addSeparator();
        }

        if (camera && camera->isValid())
        {
            aAddFeed = menu.addAction(tr("Add to view"));
            menu.addSeparator();
            aOpenWin = menu.addAction(tr("Open in window"));
            aOpenFull = menu.addAction(tr("Open as fullscreen"));
            menu.addSeparator();
            aCamRename = menu.addAction(tr("Rename device"));
            aCamRename->setEnabled(false);
        }
    }
    else
    {
        aAddServer = menu.addAction(tr("Add another server"));
        aOptions = menu.addAction(tr("Options"));
    }

    QAction *action = menu.exec(event->globalPos());
    if (!action)
        return;

    if (action == aOptions)
    {
        bcApp->mainWindow->showOptionsDialog();
    }
    else if (action == aAddServer)
    {
        bcApp->mainWindow->addServer();
    }
    else if (action == aAddFeed)
    {
        if (camera)
            bcApp->mainWindow->liveView()->view()->addCamera(*camera);
    }
    else if (action == aOpenWin || action == aOpenFull)
    {
        if (camera)
        {
            LiveViewWindow *w = LiveViewWindow::openWindow(bcApp->mainWindow, (action == aOpenFull), *camera);
            if (action == aOpenFull)
                w->showFullScreen();
            else
                w->show();
        }
    }
    else if (action == aSelectOnly)
    {
        checkOnlyIndex(index);
    }
    else if (action == aSelectElse)
    {
        for (int i = 0, n = model()->rowCount(); i < n; ++i)
        {
            QModelIndex idx = model()->index(i, index.column(), QModelIndex());
            if (idx != index)
                model()->setData(idx, Qt::Checked, Qt::CheckStateRole);
        }

        model()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }
}

void DVRServersView::checkOnlyIndex(const QModelIndex &index)
{
    /* This assumes that unchecking all top-level items will uncheck everything under them */
    for (int i = 0, n = model()->rowCount(); i < n; ++i)
    {
        QModelIndex idx = model()->index(i, index.column(), QModelIndex());
        if (idx != index)
            model()->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
    }

    model()->setData(index, Qt::Checked, Qt::CheckStateRole);
}

void DVRServersView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index;
    if (event->button() == Qt::LeftButton && (index = indexAt(event->pos())).isValid())
    {
        DVRServer *server = index.data(DVRServersModel::DVRServerRole).value<DVRServer *>();
        DVRCamera *camera = index.data(DVRServersModel::DVRCameraRole).value<DVRCamera *>();
        if (index.flags() & Qt::ItemIsUserCheckable)
        {
            Qt::CheckState state = (index.data(Qt::CheckStateRole).toInt() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            model()->setData(index, state, Qt::CheckStateRole);

            event->accept();
        }
        else if (server && !(index.flags() & Qt::ItemIsEnabled))
        {
            if (server->isLoginPending())
            {
                OptionsDialog *dlg = new OptionsDialog(bcApp->serverRepository(), this);
                dlg->showPage(OptionsDialog::ServerPage);
                dlg->setAttribute(Qt::WA_DeleteOnClose);

                OptionsServerPage *pg = static_cast<OptionsServerPage*>(dlg->pageWidget(OptionsDialog::ServerPage));
                pg->setCurrentServer(server);
                dlg->show();
            }
            else
                server->login();
        }
        else if (server)
        {
            ServerConfigWindow::instance()->setServer(server);
            ServerConfigWindow::instance()->show();
            ServerConfigWindow::instance()->raise();
        }
        else if (camera && camera->isValid())
        {
            bcApp->mainWindow->liveView()->view()->addCamera(*camera);
        }

        event->accept();
        return;
    }

    QAbstractItemView::mouseDoubleClickEvent(event);
}

void DVRServersView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex current = currentIndex();
    if (current.isValid() && (current == topLeft || topLeft < current) &&
        (current == bottomRight || current < bottomRight) && !(current.flags() & Qt::ItemIsEnabled))
    {
        setCurrentIndex(QModelIndex());
    }

    QTreeView::dataChanged(topLeft, bottomRight);
}

void DVRServersView::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (!model()->hasChildren(parent))
        setExpanded(parent, true);
}
