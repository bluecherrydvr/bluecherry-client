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

#ifndef LIVEVIEWWINDOW_H
#define LIVEVIEWWINDOW_H

#include <QWidget>
#include <QWeakPointer>
#include "camera/DVRCamera.h"
#include <QCloseEvent>
#include <QGridLayout>
#include <QDrag>
#include <QPoint>

class DVRServerRepository;
class LiveViewArea;
class QAction;
class QComboBox;
class QToolBar;

class LiveViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LiveViewWindow(DVRServerRepository *serverRepository, QWidget *parent = 0, bool fullscreen = false, Qt::WindowFlags windowFlags = 0);

    /* Note that the returned window has the Qt::WA_DeleteOnClose attribute set.
     * If you intend to save this pointer long-term, put it in a guard (QWeakPointer) or
     * unset this attribute. */
    static LiveViewWindow *openWindow(DVRServerRepository *serverRepository, QWidget *parent, bool fullscreen, DVRCamera *camera = 0);


    QString currentLayout() const;

    void setAutoSized(bool autoSized);

    bool isFullScreen() const { return QWidget::isFullScreen() || m_fsSetWindow; }

    void restoreSession();

    QWidget *topWidget() { return m_topWidget; }
    QWidget *fullScreenWidget() { return m_fsSetWindow.data(); }
    void addCamera(DVRCamera *camera);
    void addCamera_dragdrop(QDropEvent *event, DVRCamera *camera);
    static int maxRows();
    static int maxColumns();
    void setGridSize(int rows, int columns);
    bool isRowEmpty(int rowIndex) const;
    bool isColumnEmpty(int rowIndex) const;
    QByteArray serializeLayout() const;

public slots:
    void showSingleCamera(DVRCamera *camera);
    bool setLayout(const QString &layout);
    void saveLayout();
    void setRows(int r) { setGridSize(r, m_cols); }
    void insertRow(int row);
    void appendRow() { insertRow(m_rows); }
    void removeRow(int row);
    void removeRow() { setGridSize(m_rows - 1, m_cols); }

    void setColumns(int c) { setGridSize(m_rows, c); }
    void insertColumn(int column);
    void appendColumn() { insertColumn(m_cols); }
    void removeColumn(int column);
    void removeColumn() { setGridSize(m_rows, m_cols - 1); }

    void setGridSize(int size) { setGridSize(size, size); }
    void setGridSize(QString size);
    bool loadLayout(const QByteArray &buf);

    bool createNewLayout(QString name = QString());
    void renameLayout(QString name = QString());
    void deleteCurrentLayout(bool needsConfirmation = true);

    void setFullScreen(bool fullScreen = true);
    void toggleFullScreen() { setFullScreen(!isFullScreen()); }
    void exitFullScreen() { setFullScreen(false); }
    void clean();

signals:
    void layoutChanged(const QString &layout);

protected:
    virtual void changeEvent(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void moveEvent(QMoveEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool event(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void savedLayoutChanged(int index);
    void showLayoutMenu(const QPoint &pos, int index = -1);
    void doAutoResize();
    void updateLayoutActionStates();
    void camerasBrowseKeys(QKeyEvent *event);
    void removeCamera(QWidget *widget);

private:


    DVRServerRepository *m_serverRepository;
    QToolBar *m_toolBar;
    QComboBox * const m_savedLayouts;
    QAction *aNewLayout, *aRenameLayout, *aDelLayout;
    QAction *m_addRowAction, *m_removeRowAction;
    QAction *m_addColumnAction, *m_removeColumnAction;
    QAction *m_singleAction, *m_fullscreenAction, *m_closeAction;
    QWeakPointer<LiveViewWindow> m_fsSetWindow;
    static QWidget *m_topWidget;
    int m_lastLayoutIndex;
    int m_switchLayoutIndex;
    int m_switchItemIndex;
    QList<DVRCamera*> m_cameras;
    bool m_autoSized, m_isLayoutChanging, m_wasOpenedFs;
    static bool m_isSessionRestoring;
    QGridLayout *m_liveviewlayout;
    int m_rows, m_cols;
    QPoint m_dragStartPosition;
    int m_dragSrcRow, m_dragSrcCol;

    void retranslateUI();
    void geometryChanged();
    void saveWindowLayoutName(QString name);
    void switchLayout(bool next);
    void switchCamera(bool next);
    void clearBrowseParams();
    void removeRows(int remove);
    void removeColumns(int remove);
    bool findEmptyLayoutCell(int *r, int *c);
    void gridPos(const QPoint &pos, int *row, int *column);
};

#endif // LIVEVIEWWINDOW_H
