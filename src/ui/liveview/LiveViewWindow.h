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

#ifndef LIVEVIEWWINDOW_H
#define LIVEVIEWWINDOW_H

#include <QWidget>
#include <QWeakPointer>
#include "core/DVRCamera.h"

class LiveViewArea;
class QComboBox;

class LiveViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LiveViewWindow(QWidget *parent = 0, bool fullscreen = false, Qt::WindowFlags windowFlags = 0);

    /* Note that the returned window has the Qt::WA_DeleteOnClose attribute set.
     * If you intend to save this pointer long-term, put it in a guard (QWeakPointer) or
     * unset this attribute. */
    static LiveViewWindow *openWindow(QWidget *parent, bool fullscreen, const DVRCamera &camera = DVRCamera());

    LiveViewArea *view() const { return m_liveView; }
    QString currentLayout() const;

    void setAutoSized(bool autoSized);

    bool isFullScreen() const { return QWidget::isFullScreen() || m_fsSetWindow; }

public slots:
    void showSingleCamera(const DVRCamera &camera);
    bool setLayout(const QString &layout);
    void saveLayout();

    bool createNewLayout(QString name = QString());
    void renameLayout(QString name = QString());
    void deleteCurrentLayout(bool needsConfirmation = true);

    void setFullScreen(bool fullScreen = true);
    void toggleFullScreen() { setFullScreen(!isFullScreen()); }
    void exitFullScreen() { setFullScreen(false); }

signals:
    void layoutChanged(const QString &layout);

private slots:
    void savedLayoutChanged(int index);
    void showLayoutMenu(const QPoint &pos, int index = -1);
    void doAutoResize();

private:
    LiveViewArea *m_liveView;
    QComboBox * const m_savedLayouts;
    QAction *aRenameLayout, *aDelLayout;
    QWeakPointer<LiveViewWindow> m_fsSetWindow;
    int m_lastLayoutIndex;
    bool m_autoSized, m_isLayoutChanging, m_wasOpenedFs;
};

#endif // LIVEVIEWWINDOW_H
