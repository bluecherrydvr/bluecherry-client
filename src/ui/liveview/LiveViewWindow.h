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
    void updateLayoutActionStates();

private:
    LiveViewArea *m_liveView;
    QComboBox * const m_savedLayouts;
    QAction *aRenameLayout, *aDelLayout;
    QAction *m_addRowAction, *m_removeRowAction;
    QAction *m_addColumnAction, *m_removeColumnAction;
    QWeakPointer<LiveViewWindow> m_fsSetWindow;
    int m_lastLayoutIndex;
    bool m_autoSized, m_isLayoutChanging, m_wasOpenedFs;
};

#endif // LIVEVIEWWINDOW_H
