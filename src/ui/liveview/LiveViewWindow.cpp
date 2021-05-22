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

#include "LiveViewWindow.h"
#include "ui/model/SavedLayoutsModel.h"
#include "ui/MainWindow.h"
#include "core/BluecherryApp.h"
#include "server/DVRServer.h"
#include "ui/liveview/cameracontainerwidget.h"
#include <QBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QSignalMapper>
#include <QSettings>
#include <QToolButton>
#include <QInputDialog>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QShortcut>
#include <QTextDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMimeData>


bool LiveViewWindow::m_isSessionRestoring = false;
QWidget *LiveViewWindow::m_topWidget = NULL;

LiveViewWindow *LiveViewWindow::openWindow(DVRServerRepository *serverRepository, QWidget *parent, bool fullscreen, DVRCamera *camera)
{
#ifdef Q_OS_MAC
    /* Child windows are undesirable on Mac, and cause problems (QTBUG-20652) */
    parent = 0;
#endif
    LiveViewWindow *window = new LiveViewWindow(serverRepository, parent, fullscreen, Qt::Window);
    window->setAutoSized(!fullscreen);
    window->setAttribute(Qt::WA_DeleteOnClose);

    if (camera)
        window->showSingleCamera(camera);

    QSettings settings;
    if (settings.value(QLatin1String("ui/saveSession"), false).toBool() == false)
        return window;

    settings.beginGroup("session");

    /* Let suppose at the same time max openned live view windows can be 128 */

    for (int i = 1; i <= 128 && !m_isSessionRestoring; i++)
    {
        if (settings.childKeys().indexOf(QString::fromLatin1("Window-%1").arg(i)) == -1)
        {
            QString key = QString::fromLatin1("Window-%1").arg(i);
            settings.setValue(key, QString());
            settings.setValue(QString::fromLatin1("geometry/%1").arg(key), bcApp->mainWindow->saveGeometry());
            window->setObjectName(key);
            break;
        }
    }

    return window;
}

LiveViewWindow::LiveViewWindow(DVRServerRepository *serverRepository, QWidget *parent, bool openfs, Qt::WindowFlags f)
    : QWidget(parent, f), m_serverRepository(serverRepository), m_savedLayouts(new QComboBox),
      m_lastLayoutIndex(-1), m_switchItemIndex(-1), m_autoSized(false), m_isLayoutChanging(false),
      m_wasOpenedFs(openfs), m_liveviewlayout(0),
      m_rows(1), m_cols(1), m_dragStartPosition(-1, -1),
      m_dragSrcRow(-1), m_dragSrcCol(-1)
{
    setBackgroundRole(QPalette::Shadow);
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_toolBar = new QToolBar(tr("Live View"));
    m_toolBar->setIconSize(QSize(16, 16));
    m_toolBar->setMovable(false);

#ifndef Q_OS_MAC
    //toolBar->setStyleSheet(QLatin1String("QToolBar { border: none; }"));
#endif

    //>>>m_liveView = new LiveViewArea(m_serverRepository, this);
    //LiveViewLayout *viewLayout = m_liveView->layout();

    /* Saved layouts box */
    m_savedLayouts->setModel(SavedLayoutsModel::instance());
    m_savedLayouts->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_savedLayouts->setInsertPolicy(QComboBox::NoInsert);
    m_savedLayouts->setMinimumWidth(100);
    m_savedLayouts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_savedLayouts->setCurrentIndex(-1);
    m_savedLayouts->setMaxVisibleItems(22);
    m_savedLayouts->setFocusPolicy(Qt::NoFocus);
	m_toolBar->addWidget(m_savedLayouts);

    QWidget *spacer = new QWidget;
    spacer->setFixedWidth(20);
	m_toolBar->addWidget(spacer);

	aNewLayout = m_toolBar->addAction(QIcon(QLatin1String(":/icons/plus.png")), tr("New Layout"), this, SLOT(createNewLayout()));
	aRenameLayout = m_toolBar->addAction(QIcon(QLatin1String(":/icons/pencil.png")), tr("Rename Layout"), this, SLOT(renameLayout()));
	aDelLayout = m_toolBar->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Delete Layout"), this, SLOT(deleteCurrentLayout()));

    aRenameLayout->setEnabled(false);
    aDelLayout->setEnabled(false);

    spacer = new QWidget;
    spacer->setFixedWidth(16);
	m_toolBar->addWidget(spacer);

    connect(m_savedLayouts, SIGNAL(currentIndexChanged(int)), SLOT(savedLayoutChanged(int)));
    connect(m_savedLayouts, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showLayoutMenu(QPoint)));

        m_addRowAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-split-vertical.png")),
                    tr("Add Row"), this, SLOT(appendRow()));
        m_removeRowAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-join-vertical.png")),
                    tr("Remove Row"), this, SLOT(removeRow()));

    spacer = new QWidget;
    spacer->setFixedWidth(16);
	m_toolBar->addWidget(spacer);

        m_addColumnAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-split.png")),
                       tr("Add Column"), this, SLOT(appendColumn()));
        m_removeColumnAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-join.png")),
                       tr("Remove Column"), this, SLOT(removeColumn()));

    spacer = new QWidget;
    spacer->setFixedWidth(16);
	m_toolBar->addWidget(spacer);

    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(setGridSize(int)));
    connect(mapper, SIGNAL(mapped(QString)), this, SLOT(setGridSize(QString)));

    m_singleAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout.png")),
                                    tr("Single"), mapper, SLOT(map()));
    mapper->setMapping(m_singleAction, 1);
    QAction *a = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-4.png")),
                           tr("2x2"), mapper, SLOT(map()));
    mapper->setMapping(a, 2);
    a = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-9.png")),
                           tr("3x3"), mapper, SLOT(map()));
    mapper->setMapping(a, 3);
    a = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-12.png")),
                           tr("4x3"), mapper, SLOT(map()));
    mapper->setMapping(a, QString("4x3"));
    a = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-16.png")),
                           tr("4x4"), mapper, SLOT(map()));
    mapper->setMapping(a, 4);
    a = m_toolBar->addAction(QIcon(QLatin1String(":/icons/layout-32.png")),
                           tr("8x4"), mapper, SLOT(map()));
    mapper->setMapping(a, QString("8x4"));

    spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_toolBar->addWidget(spacer);

	m_fullscreenAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/application-resize-full.png")),
                       tr("Fullscreen"), this, SLOT(toggleFullScreen()));
	m_fullscreenAction->setShortcut(Qt::Key_F11);

    if (m_wasOpenedFs)
    {
		m_closeAction = m_toolBar->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Exit"),
                           this, SLOT(close()));
        new QShortcut(Qt::Key_Escape, this, SLOT(close()), 0, Qt::WindowShortcut);
    }
    else
        new QShortcut(Qt::Key_Escape, this, SLOT(exitFullScreen()), 0, Qt::WindowShortcut);

    //connect(m_liveView->layout(), SIGNAL(layoutChanged()), SLOT(updateLayoutActionStates()));
    //connect(m_liveView->layout(), SIGNAL(layoutChanged()), SLOT(saveLayout()));
    //connect(m_liveView, SIGNAL(forwardKey(QKeyEvent*)), SLOT(camerasBrowseKeys(QKeyEvent*)));

    QMainWindow *wnd = qobject_cast<QMainWindow*>(window());
    if (wnd)
		wnd->addToolBar(Qt::TopToolBarArea, m_toolBar);
    else
                layout->addWidget(m_toolBar);
    {
        m_liveviewlayout = new QGridLayout();
        layout->addLayout(m_liveviewlayout);
        //initialize default empty cell
        m_liveviewlayout->addWidget(new CameraContainerWidget(), 0, 0);
    }
    updateLayoutActionStates();
    setAcceptDrops(true);
}

bool LiveViewWindow::findEmptyLayoutCell(int *r, int *c)
{
    int rc, cc, ar = -1, ac = -1;
    rc = m_rows;
    cc = m_cols;
    for (int i = 0; i < rc; i++)
    {
        for (int j = 0; j < cc; j++)
        {
            QLayoutItem *item = m_liveviewlayout->itemAtPosition(i, j);
            if (!(item && item->widget() && ((CameraContainerWidget*)(item->widget()))->camera()))
            {
                ar = i;
                ac = j;
                break;
            }
        }
    }
    if (ar == -1 && ac == -1)
        return false;

    *r = ar;
    *c = ac;

    return true;
}

void LiveViewWindow::removeCamera(QWidget *widget)
{
    QLayoutItem *item;
    CameraContainerWidget *ccw = new CameraContainerWidget();
    item = m_liveviewlayout->replaceWidget(widget, ccw);
    widget->deleteLater();
    delete(item);

    updateLayoutActionStates();
    saveLayout();
}

void LiveViewWindow::addCamera(DVRCamera *camera)
{
    int rc, cc, ar = -1, ac = -1;
    rc = m_rows;
    cc = m_cols;

    if (!findEmptyLayoutCell(&ar, &ac))
    {
        /* Add a row or a column to make space, whichever has fewer */
        if (cc < rc)
            insertColumn(cc);
        else
            insertRow(rc);

        if (!findEmptyLayoutCell(&ar, &ac))
            return;
    }

    QLayoutItem *item = m_liveviewlayout->itemAtPosition(ar, ac);

    if (!item)
        return;

    CameraContainerWidget *ccw = new CameraContainerWidget();
    ccw->setCamera(camera);
    ccw->setServerRepository(m_serverRepository);

    item = m_liveviewlayout->replaceWidget(item->widget(), ccw);
    item->widget()->deleteLater();
    delete(item);
    connect(ccw, SIGNAL(cameraClosed(QWidget*)), this, SLOT(removeCamera(QWidget*)));

    updateLayoutActionStates();
    saveLayout();
}

void LiveViewWindow::setAutoSized(bool autoSized)
{
    if (m_autoSized == autoSized)
        return;

    m_autoSized = autoSized;
    if (m_autoSized)
    {
        //>>connect(m_liveView->layout(), SIGNAL(idealSizeChanged(QSize)), SLOT(doAutoResize()));
        doAutoResize();
    }
    //>>else
        //>>disconnect(m_liveView->layout(), SIGNAL(idealSizeChanged(QSize)), this, SLOT(doAutoResize()));
}

void LiveViewWindow::doAutoResize()
{
    if (!m_autoSized)
        return;

    //>>m_liveView->updateGeometry();
    //>>if (m_liveView->sizeHint().isEmpty())
        //>>return;

    if (!isFullScreen())
    {
        QSize hint = sizeHint();
        const QRect rect = QApplication::desktop()->availableGeometry(this);

        hint.rwidth() = qRound(qMin(rect.width()*.9, double(hint.rwidth())));
        hint.rheight() = qRound(qMin(rect.height()*.9, double(hint.rheight())));

        resize(hint);
    }

    setAutoSized(false);
}

void LiveViewWindow::showSingleCamera(DVRCamera *camera)
{
    setGridSize(1, 1);
    QLayoutItem *item = m_liveviewlayout->itemAtPosition(0, 0);

    if (!item)
        return;

    CameraContainerWidget *ccw = new CameraContainerWidget();
    ccw->setCamera(camera);
    ccw->setServerRepository(m_serverRepository);

    item = m_liveviewlayout->replaceWidget(item->widget(), ccw);
    item->widget()->deleteLater();
    delete(item);
    connect(ccw, SIGNAL(cameraClosed(QWidget*)), this, SLOT(removeCamera(QWidget*)));
}

bool LiveViewWindow::setLayout(const QString &layout)
{
    int index = m_savedLayouts->findText(layout);
    if (index < 0)
        return false;

    m_savedLayouts->setCurrentIndex(index);
    return true;
}

QString LiveViewWindow::currentLayout() const
{
    int index = m_savedLayouts->currentIndex();
    if (index >= 0)
        return m_savedLayouts->itemText(index);
    return QString();
}

//<<<<<<<<<<<<<<<<<
#define MAX_ROWS 16
#define MAX_COLUMNS 16
int LiveViewWindow::maxRows()
{
    return MAX_ROWS;
}

int LiveViewWindow::maxColumns()
{
    return MAX_COLUMNS;
}
void LiveViewWindow::insertRow(int row)
{
    int cols = m_cols;
    if (cols == 0)
        cols = 1;
    for (int i = 0; i < cols; i++)
    {
        m_liveviewlayout->addWidget(new CameraContainerWidget(), row, i);
    }

    m_rows++;
    updateLayoutActionStates();
    saveLayout();
}

void LiveViewWindow::removeRow(int row)
{
    qDebug() << "removeRow(" << row << ")";
    if (!m_rows)
        return;
    row = qBound(0, row, m_rows - 1);

    int cols = m_cols;
    for (int i = 0; i < cols; i++)
    {
        QLayoutItem *item = m_liveviewlayout->itemAtPosition(row, i);
        m_liveviewlayout->removeItem(item);
        if (item && item->widget())
        {
            item->widget()->deleteLater();
        }
        delete item;
    }

    m_rows--;
    /* Ensure that we always have at least one row, but still clear it if asked to remove */
    if (m_rows == 0)
        insertRow(0);

    updateLayoutActionStates();
    saveLayout();
}

void LiveViewWindow::insertColumn(int column)
{
    int rows = m_rows;

    if (rows == 0)
        rows = 1;

    for(int i = 0; i < rows; i++)
    {
        m_liveviewlayout->addWidget(new CameraContainerWidget(), i, column);
    }
    m_cols++;
    updateLayoutActionStates();
    saveLayout();
}

void LiveViewWindow::removeColumn(int column)
{
    if (!m_cols)
        return;
    column = qBound(0, column, m_cols - 1);

    int rows = m_rows;
    for (int i = 0; i < rows; i++)
    {
        QLayoutItem *item = m_liveviewlayout->itemAtPosition(i, column);
        m_liveviewlayout->removeItem(item);
        if (item && item->widget())
        {
            item->widget()->deleteLater();
        }
        delete(item);
    }
    m_cols--;

    if (m_cols == 0)
        insertColumn(0);
    updateLayoutActionStates();
    saveLayout();
}
bool LiveViewWindow::isRowEmpty(int rowIndex) const
{
    for (int c = 0; c < m_cols; ++c)
    {
        QLayoutItem *item = m_liveviewlayout->itemAtPosition(rowIndex, c);

        if (item && item->widget() && ((CameraContainerWidget*)(item->widget()))->camera())
            return false;
    }

    return true;
}

bool LiveViewWindow::isColumnEmpty(int columnIndex) const
{
    for (int r = 0; r < m_rows; ++r)
    {
        QLayoutItem *item = m_liveviewlayout->itemAtPosition(r, columnIndex);

        if (item && item->widget() && ((CameraContainerWidget*)(item->widget()))->camera())
            return false;
    }
    return true;
}

void LiveViewWindow::removeRows(int remove)
{
    qDebug() << "removeRows(" << remove << ")";
    /* If there are any empty rows, remove those first */
    int rows = m_rows;

    for (int r = rows - 1; remove && r >= 0; --r, --remove)
        removeRow(r);

    Q_ASSERT(!remove);
}

void LiveViewWindow::removeColumns(int remove)
{
    int cols = m_cols;

    for (int c = cols - 1; remove && c >= 0; --c, --remove)
        removeColumn(c);

    Q_ASSERT(!remove);
}

void LiveViewWindow::setGridSize(QString size)
{
    QStringList list = size.split("x");

    if (list.count() != 2)
        return;

    int columns = list[0].toInt();
    int rows = list[1].toInt();
    if (rows == 0 || columns == 0)
        return;

    setGridSize(rows, columns);
}

void LiveViewWindow::setGridSize(int rows, int columns)
{
    rows = qBound(1, rows, maxRows());
    columns = qBound(1, columns, maxColumns());
    if (rows == m_rows && columns == m_cols)
        return;

    if (m_rows > rows)
        removeRows(m_rows - rows);

    if (m_cols > columns)
        removeColumns(m_cols - columns);

    while (m_cols < columns)
        insertColumn(m_cols);

    while (m_rows < rows)
        insertRow(m_rows);
}

bool LiveViewWindow::loadLayout(const QByteArray &buf)
{
    if (buf.isEmpty())
        return false;

    QDataStream data(buf);
    data.setVersion(QDataStream::Qt_4_5);

    /* Legacy format is [rc][cc][...]
     * Newer format is [-1][version][rc][cc][...] */
    int rc = 0, cc = 0, version = 0;
    data >> rc;
    if (rc < 0)
        data >> version;

    if (version == 0)
        data >> cc;
    else if (version > 0)
        data >> rc >> cc;

    if (data.status() != QDataStream::Ok)
        return false;

    setGridSize(rc, cc);

    // update rc, cc values if were invalid
    rc = m_rows;
    cc = m_cols;

    for (int r = 0; r < rc; ++r)
    {
        for (int c = 0; c < cc; ++c)
        {
            qint64 pos = data.device()->pos();
            int value = -1;
            data >> value;

            CameraContainerWidget *ccw = 0;

            if (value != -1)
            {
                /* Seek back to before the field we peeked at */
                data.device()->seek(pos);

                ccw = new CameraContainerWidget();
                ccw->setServerRepository(m_serverRepository);
                ccw->loadState(&data, version);

                if (ccw)
                {
                    QLayoutItem *item;
                    item = m_liveviewlayout->itemAtPosition(r,c);

                    if (item)
                    {
                        item = m_liveviewlayout->replaceWidget(item->widget(), ccw);
                        item->widget()->deleteLater();
                        delete(item);
                        connect(ccw, SIGNAL(cameraClosed(QWidget*)), this, SLOT(removeCamera(QWidget*)));
                    }
                }
            }
        }
    }

    return (data.status() == QDataStream::Ok);
}
QByteArray LiveViewWindow::serializeLayout() const
{
    QByteArray re;
    QDataStream data(&re, QIODevice::WriteOnly);
    data.setVersion(QDataStream::Qt_4_5);
    int rc, cc;
    rc = m_rows;
    cc = m_cols;

    /* -1, then version */
    data << -1 << 1;
    data << rc << cc;
    for (int i = 0; i < m_rows; i++)
    {
        for(int j = 0;  j < m_cols; j++)
        {
            QLayoutItem *item;
            item = m_liveviewlayout->itemAtPosition(i, j);
            if (!item || !item->widget() || !((CameraContainerWidget*)(item->widget()))->camera())
                data << -1;
            else
                ((CameraContainerWidget *)item->widget())->saveState(&data);
        }
    }

    return re;
}
//<<<<<<<<<<<<<<<
void LiveViewWindow::savedLayoutChanged(int index)
{
    if (index == -1 || m_isLayoutChanging)
        return;

    m_isLayoutChanging = true;

    if (static_cast<SavedLayoutsModel*>(m_savedLayouts->model())->isNewLayoutItem(index))
    {
        if (!createNewLayout())
            m_savedLayouts->setCurrentIndex(m_lastLayoutIndex);

        m_isLayoutChanging = false;
        return;
    }

    QByteArray data = m_savedLayouts->itemData(index, SavedLayoutsModel::LayoutDataRole).toByteArray();
    if (!data.isEmpty() && !loadLayout(data))
        qDebug() << "Failed to load camera layout" << m_savedLayouts->itemText(index);

    m_lastLayoutIndex = index;
    emit layoutChanged(currentLayout());

    aRenameLayout->setEnabled(index >= 0);
    aDelLayout->setEnabled(index >= 0);

    m_isLayoutChanging = false;

    saveWindowLayoutName(currentLayout());
}

bool LiveViewWindow::createNewLayout(QString name)
{
    if (name.isEmpty())
    {
        name = QInputDialog::getText(window(), tr("Create live view layout"), tr("Enter a name for the new layout:"));
        if (name.isEmpty())
            return false;
    }

    int index = m_savedLayouts->count() - 1;
    m_savedLayouts->insertItem(index, name, serializeLayout());

    m_isLayoutChanging = false;
    m_savedLayouts->setCurrentIndex(index);

    saveWindowLayoutName(name);

    return true;
}

void LiveViewWindow::renameLayout(QString name)
{
    if (m_savedLayouts->currentIndex() < 0)
        return;

    if (name.isEmpty())
    {
        name = QInputDialog::getText(window(), tr("Rename live view layout"),
                                     tr("Enter a new name for the <b>%1</b>:").arg(Qt::escape(m_savedLayouts->currentText())));
        if (name.isEmpty())
            return;
    }

    m_savedLayouts->setItemText(m_savedLayouts->currentIndex(), name);

    saveWindowLayoutName(name);
}

void LiveViewWindow::deleteCurrentLayout(bool confirm)
{
    if (confirm)
    {
        QMessageBox dlg(QMessageBox::Question, tr("Bluecherry Client"),
                        tr("Are you sure you want to delete the <b>%1</b> layout?").arg(Qt::escape(m_savedLayouts->currentText())));
        QPushButton *delBtn = dlg.addButton(tr("Delete"), QMessageBox::DestructiveRole);
        dlg.addButton(QMessageBox::Cancel);
        dlg.setDefaultButton(QMessageBox::Cancel);

        dlg.exec();
        if (dlg.clickedButton() != delBtn)
            return;
    }

    int index = m_savedLayouts->currentIndex();

    bool b = m_savedLayouts->blockSignals(true);
    m_savedLayouts->removeItem(index);
    m_savedLayouts->blockSignals(b);

    int i = m_lastLayoutIndex - ((index < m_lastLayoutIndex) ? 1 : 0);
    if (index == m_lastLayoutIndex)
        i = qMax(index - 1, 0);
    m_lastLayoutIndex = -1;
    if (i != m_savedLayouts->currentIndex())
        m_savedLayouts->setCurrentIndex(i);
    else
        savedLayoutChanged(i);
}

void LiveViewWindow::saveLayout()
{
    if (m_lastLayoutIndex < 0 || m_isLayoutChanging)
        return;

    clearBrowseParams();

    QByteArray data = serializeLayout();
    m_savedLayouts->setItemData(m_lastLayoutIndex, data, SavedLayoutsModel::LayoutDataRole);
    qDebug() << "saved layout: " << data;
}

void LiveViewWindow::showLayoutMenu(const QPoint &rpos, int index)
{
    if (index < 0)
        index = m_savedLayouts->currentIndex();

    if (index < 0 || static_cast<SavedLayoutsModel*>(m_savedLayouts->model())->isNewLayoutItem(index))
        return;

    QPoint pos = rpos;
    if (qobject_cast<QWidget*>(sender()))
        pos = static_cast<QWidget*>(sender())->mapToGlobal(pos);

    QMenu menu;
    menu.setTitle(m_savedLayouts->itemText(index));

    QAction *deleteAction = menu.addAction(tr("Delete \"%1\"").arg(menu.title()));
    if (m_savedLayouts->count() == 2)
        deleteAction->setEnabled(false);

    QAction *action = menu.exec(pos);
    if (!action)
        return;

    if (action == deleteAction)
        deleteCurrentLayout();
}

void LiveViewWindow::setFullScreen(bool on)
{
    if (on == isFullScreen())
        return;

    if (on)
    {
        if (!isWindow())
        {
            LiveViewWindow *wnd = LiveViewWindow::openWindow(m_serverRepository, this, true);
            wnd->setLayout(currentLayout());
            wnd->showFullScreen();
            m_fsSetWindow = wnd;
            connect(wnd, SIGNAL(layoutChanged(QString)), SLOT(setLayout(QString)));
            connect(wnd, SIGNAL(destroyed()), SLOT(show()));
            setVisible(false);
        }
        else
            showFullScreen();
    }
    else
    {
        if (m_fsSetWindow)
        {
            setVisible(true);
            m_fsSetWindow.data()->close();
            m_fsSetWindow.clear();
        }
        else
            showNormal();
    }

//    QSettings settings;
//    if (settings.value(QLatin1String("ui/disableScreensaver/onFullscreen")).toBool())
//        bcApp->setScreensaverInhibited(on);
}

void LiveViewWindow::updateLayoutActionStates()
{
    int rc, cc;
    rc = m_rows;
    cc = m_cols;
    m_addRowAction->setEnabled(rc < maxRows());
    m_removeRowAction->setEnabled(rc > 1);
    m_addColumnAction->setEnabled(cc < maxColumns());
    m_removeColumnAction->setEnabled(cc > 1);
}

void LiveViewWindow::retranslateUI()
{
	m_toolBar->setWindowTitle(tr("Live View"));

	aNewLayout->setText(tr("New Layout"));
	aRenameLayout->setText(tr("Rename Layout"));
	aDelLayout->setText(tr("Delete Layout"));
	m_addRowAction->setText(tr("Add Row"));
	m_removeRowAction->setText(tr("Remove Row"));
	m_addColumnAction->setText(tr("Add Column"));
	m_removeColumnAction->setText(tr("Remove Column"));
	m_singleAction->setText(tr("Single"));
	m_fullscreenAction->setText(tr("Fullscreen"));

	if (m_wasOpenedFs)
		m_closeAction->setText(tr("Exit"));

}

void LiveViewWindow::saveWindowLayoutName(QString name)
{
    QSettings settings;

    if (settings.value(QLatin1String("ui/saveSession"), false).toBool())
    {
        settings.beginGroup("session");
        QString key = objectName();

        if (settings.childKeys().indexOf(key) != -1)
            settings.setValue(key, name);
    }
}

void LiveViewWindow::restoreSession()
{
    QSettings settings;
    QString topWidget = settings.value(QLatin1String("ui/topWindow"), QString()).toString();
    settings.beginGroup("session");
    QStringList keyList = settings.childKeys();
    LiveViewWindow *top = NULL;

    m_isSessionRestoring = true;

    foreach(QString key, keyList)
    {
        if (settings.value(key).toString().isEmpty())
        {
            settings.remove(key);
            settings.remove(QString::fromLatin1("geometry/%1").arg(key));
            keyList.removeOne(key);
        }
        else
        {
            LiveViewWindow *window;
            window = openWindow(m_serverRepository, bcApp->mainWindow, false, NULL);
            window->setObjectName(key);
            window->setLayout(settings.value(key).toString());
            window->restoreGeometry(settings.value(QString::fromLatin1("geometry/%1").arg(key)).toByteArray());
            window->show();

            if (!topWidget.isEmpty() && topWidget == key)
                top = window;
        }
    }

    m_topWidget = top;

    m_isSessionRestoring = false;
}

void LiveViewWindow::geometryChanged()
{
    QSettings settings;

    if (settings.value(QLatin1String("ui/saveSession"), false).toBool() == false)
        return;

    settings.beginGroup("session/geometry");
    QStringList keyList = settings.allKeys();

    foreach(QString key, keyList)
    {
        if (key == objectName())
            settings.setValue(key, saveGeometry());
    }
}

void LiveViewWindow::changeEvent(QEvent *event)
{
    if (event && event->type() == QEvent::LanguageChange)
        retranslateUI();

    QWidget::changeEvent(event);
}

void LiveViewWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.beginGroup("session");
    settings.remove(objectName());
    settings.remove(QString::fromLatin1("geometry/%1").arg(objectName()));

    QWidget::closeEvent(event);
}

void LiveViewWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    geometryChanged();
}


void LiveViewWindow::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);

    geometryChanged();
}

bool LiveViewWindow::event(QEvent *event)
{
    if (event && event->type() == QEvent::WindowActivate)
        bcApp->mainWindow->saveTopWindow(this);

    return QWidget::event(event);
}

void LiveViewWindow::gridPos(const QPoint &pos, int *row, int *column)
{
    Q_ASSERT(row && column);
    Q_ASSERT(m_cols && m_rows);
    int w, h, left, top;

    w = width() / m_cols;
    h = height() / m_rows;

    left = width() % m_cols;
    top = height() % m_rows;

    if (pos.x() < 0 || pos.x() > width() || pos.y() < 0 || pos.y() > height())
    {
        *row = *column = -1;
        return;
    }

    *row = (pos.y() - top) / h;
    *column = (pos.x() - left) / w;
}

void LiveViewWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - m_dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    CameraContainerWidget *cw = static_cast<CameraContainerWidget*>(childAt(event->pos()));

    if (!cw)
        return;
    if (!cw->camera())
        return;

    gridPos(event->pos(), &m_dragSrcRow, &m_dragSrcCol);

    QLayoutItem *item = m_liveviewlayout->itemAtPosition(m_dragSrcRow, m_dragSrcCol);

    if (!(item && item->widget() == cw))
        qDebug() << "drag source widget not found!!!!";
    qDebug() << "drag source is at grid pos (" << m_dragSrcRow << "," << m_dragSrcCol << ")";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray emptydata;
    mimeData->setData(QLatin1String("application/x-bluecherry-camerawidget"), emptydata);
    mimeData->setText(QString("test"));
    drag->setMimeData(mimeData);
    if (cw->stream())
        drag->setPixmap(QPixmap::fromImage(cw->stream()->currentFrame()));

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void LiveViewWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPosition = event->pos();
}

void LiveViewWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void LiveViewWindow::dropEvent(QDropEvent *event)
{
    //drag from servers/cameras view on the left
    if (event->mimeData()->hasFormat(QLatin1String("application/x-bluecherry-dvrcamera")))
    {

        QList<DVRCamera *> cameras = DVRCamera::fromMimeData(m_serverRepository, event->mimeData());
        if (cameras.isEmpty())
            return;
        addCamera(cameras.at(0));
        event->acceptProposedAction();
        saveLayout();
    }
    else if(event->mimeData()->hasFormat(QLatin1String("application/x-bluecherry-camerawidget")))
    {
        int targetRow, targetCol;
        QWidget *target, *source;
        gridPos(event->pos(), &targetRow, &targetCol);

        if (targetRow == m_dragSrcRow && targetCol == m_dragSrcCol)
            return;

        QLayoutItem *srcItem = m_liveviewlayout->itemAtPosition(m_dragSrcRow, m_dragSrcCol);
        QLayoutItem *targetItem = m_liveviewlayout->itemAtPosition(targetRow, targetCol);

        if (srcItem && targetItem)
        {
            target = targetItem->widget();
            source = srcItem->widget();

            m_liveviewlayout->removeWidget(source);

            targetItem = m_liveviewlayout->replaceWidget(target, source);
            m_liveviewlayout->addWidget(target, m_dragSrcRow, m_dragSrcCol);

            delete(targetItem);
            event->acceptProposedAction();
            saveLayout();
        }
    }
}

void LiveViewWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
            event->key() == Qt::Key_Comma || event->key() == Qt::Key_Period)
    {
        camerasBrowseKeys(event);
    }

    QWidget::keyPressEvent(event);
}

void LiveViewWindow::camerasBrowseKeys(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)
    {
        switchLayout(event->key() == Qt::Key_PageDown ? true : false);
    }
    else if (event->key() == Qt::Key_Comma || event->key() == Qt::Key_Period)
    {
        //>switchCamera(event->key() == Qt::Key_Period ? true : false);
    }
}

void LiveViewWindow::switchLayout(bool next)
{
    if (!m_cameras.isEmpty())
    {
        clearBrowseParams();
        m_savedLayouts->setCurrentIndex(m_switchLayoutIndex);
        return;
    }

    if (m_savedLayouts->count() < 2)
        return;

    int index = m_savedLayouts->currentIndex();

    if (next && index == m_savedLayouts->count() - 2)
        m_savedLayouts->setCurrentIndex(0);
    else if (!next && index == 0)
        m_savedLayouts->setCurrentIndex(m_savedLayouts->count() - 2);
    else
        m_savedLayouts->setCurrentIndex(next ? index + 1 : index - 1);
}

/*void LiveViewWindow::switchCamera(bool next)
{
    if (m_switchItemIndex == -1)
    {
        if (m_liveView->layout()->count() <= 1)
            return;

        int rows = m_liveView->layout()->rows();
        int columns = m_liveView->layout()->columns();

        QQuickItem *item;

        for (int x = 0; x < rows; x++)
        {
            for (int y = 0; y < columns; y++)
            {
                item = m_liveView->layout()->at(x, y);
                if (item)
                    m_cameras.append(item->property("camera").value<DVRCamera*>());
            }
        }

        m_switchLayoutIndex = m_savedLayouts->currentIndex();
        m_lastLayoutIndex = -1;
        m_savedLayouts->setCurrentIndex(-1);

        m_liveView->layout()->setGridSize(1, 1);

        if (m_liveView->layout()->count() == 0)
            m_liveView->layout()->addItem(0, 0);
    }

    int size = m_cameras.count();

    if (size <= 1)
        return;

    if (m_switchItemIndex == -1 || m_switchItemIndex >= size)
        m_switchItemIndex = 0;
    else if (next && m_switchItemIndex == size - 1)
        m_switchItemIndex = 0;
    else if (!next && m_switchItemIndex == 0)
        m_switchItemIndex = size - 1;
    else
        m_switchItemIndex += (next ? 1 : -1);

    QQuickItem *item = m_liveView->layout()->at(0, 0);

    if (item)
        item->setProperty("camera", QVariant::fromValue(m_cameras.at(m_switchItemIndex)));
}*/

void LiveViewWindow::clearBrowseParams()
{
    if (!m_cameras.isEmpty())
    {
        m_switchItemIndex = -1;
        m_cameras.clear();
    }
}

void LiveViewWindow::clean()
{
    if (m_fsSetWindow)
    {
        m_fsSetWindow.data()->close();
        m_fsSetWindow.clear();
    }
}

