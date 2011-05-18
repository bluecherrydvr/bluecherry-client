#include "LiveViewWindow.h"
#include "LiveViewArea.h"
#include "LiveViewLayout.h"
#include "ui/SavedLayoutsModel.h"
#include "core/BluecherryApp.h"
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

#ifdef Q_OS_MAC
#include <QMacStyle>
#endif

static QToolButton *createGridButton(const char *icon, const QString &text, QWidget *target, const char *slot)
{
    QToolButton *btn = new QToolButton;
    btn->setIcon(QIcon(QLatin1String(icon)));
    btn->setText(text);
    btn->setToolTip(text);
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);

    if (target && slot)
        QObject::connect(btn, SIGNAL(clicked()), target, slot);

    return btn;
}

LiveViewWindow *LiveViewWindow::openWindow(QWidget *parent, bool fullscreen, const DVRCamera &camera)
{
    LiveViewWindow *window = new LiveViewWindow(parent, fullscreen);
    window->setWindowFlags(Qt::Window);
    window->setAutoSized(true);
    window->setAttribute(Qt::WA_DeleteOnClose);

    if (camera)
        window->showSingleCamera(camera);

    return window;
}

LiveViewWindow::LiveViewWindow(QWidget *parent, bool openfs)
    : QWidget(parent), m_liveView(new LiveViewArea), m_savedLayouts(new QComboBox), m_lastLayoutIndex(-1), m_autoSized(false),
      m_isLayoutChanging(false), m_fsSetWindow(false), m_wasOpenedFs(openfs)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    QToolBar *toolBar = new QToolBar;
    toolBar->setIconSize(QSize(16, 16));

#ifdef Q_OS_MAC
    if (qobject_cast<QMacStyle*>(style()))
    {
        //m_cameraArea->setFrameStyle(QFrame::NoFrame);
        //cameraContainer->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }
#else
    toolBar->setStyleSheet(QLatin1String("QToolBar { border: none; }"));
#endif

    LiveViewLayout *viewLayout = m_liveView->layout();

    /* Saved layouts box */
    m_savedLayouts->setModel(SavedLayoutsModel::instance());
    m_savedLayouts->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_savedLayouts->setInsertPolicy(QComboBox::NoInsert);
    m_savedLayouts->setMinimumWidth(100);
    m_savedLayouts->setContextMenuPolicy(Qt::CustomContextMenu);
    m_savedLayouts->setCurrentIndex(-1);
    toolBar->addWidget(m_savedLayouts);

    QWidget *spacer = new QWidget;
    spacer->setFixedWidth(20);
    toolBar->addWidget(spacer);

    toolBar->addAction(QIcon(QLatin1String(":/icons/plus.png")), tr("New Layout"), this, SLOT(createNewLayout()));
    aRenameLayout = toolBar->addAction(QIcon(QLatin1String(":/icons/pencil.png")), tr("Rename Layout"), this, SLOT(renameLayout()));
    aDelLayout = toolBar->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Delete Layout"), this, SLOT(deleteCurrentLayout()));

    aRenameLayout->setEnabled(false);
    aDelLayout->setEnabled(false);

    spacer = new QWidget;
    spacer->setFixedWidth(16);
    toolBar->addWidget(spacer);

    connect(m_savedLayouts, SIGNAL(currentIndexChanged(int)), SLOT(savedLayoutChanged(int)));
    connect(m_savedLayouts, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showLayoutMenu(QPoint)));

    toolBar->addAction(QIcon(QLatin1String(":/icons/layout-split-vertical.png")),
                       tr("Add Row"), viewLayout, SLOT(appendRow()));
    toolBar->addAction(QIcon(QLatin1String(":/icons/layout-join-vertical.png")),
                       tr("Remove Row"), viewLayout, SLOT(removeRow()));

    spacer = new QWidget;
    spacer->setFixedWidth(16);
    toolBar->addWidget(spacer);

    toolBar->addAction(QIcon(QLatin1String(":/icons/layout-split.png")),
                       tr("Add Column"), viewLayout, SLOT(appendColumn()));
    toolBar->addAction(QIcon(QLatin1String(":/icons/layout-join.png")),
                       tr("Remove Column"), viewLayout, SLOT(removeColumn()));

    spacer = new QWidget;
    spacer->setFixedWidth(16);
    toolBar->addWidget(spacer);

    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), viewLayout, SLOT(setGridSize(int)));

    QAction *a = toolBar->addAction(QIcon(QLatin1String(":/icons/layout.png")),
                                    tr("Single"), mapper, SLOT(map()));
    mapper->setMapping(a, 1);
    a = toolBar->addAction(QIcon(QLatin1String(":/icons/layout-4.png")),
                           tr("2x2"), mapper, SLOT(map()));
    mapper->setMapping(a, 2);
    a = toolBar->addAction(QIcon(QLatin1String(":/icons/layout-9.png")),
                           tr("3x3"), mapper, SLOT(map()));
    mapper->setMapping(a, 3);
    a = toolBar->addAction(QIcon(QLatin1String(":/icons/layout-16.png")),
                           tr("4x4"), mapper, SLOT(map()));
    mapper->setMapping(a, 4);

    spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    a = toolBar->addAction(QIcon(QLatin1String(":/icons/application-resize-full.png")),
                       tr("Fullscreen"), this, SLOT(toggleFullScreen()));
    a->setShortcut(Qt::Key_F11);

    if (m_wasOpenedFs)
    {
        toolBar->addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Exit"),
                           this, SLOT(close()));
        new QShortcut(Qt::Key_Escape, this, SLOT(close()), 0, Qt::WindowShortcut);
    }
    else
        new QShortcut(Qt::Key_Escape, this, SLOT(exitFullScreen()), 0, Qt::WindowShortcut);

    connect(m_liveView->layout(), SIGNAL(layoutChanged()), SLOT(saveLayout()));

    layout->addWidget(toolBar);
    layout->addWidget(m_liveView);
}

void LiveViewWindow::setAutoSized(bool autoSized)
{
    if (m_autoSized == autoSized)
        return;

    m_autoSized = autoSized;
    if (m_autoSized)
    {
        connect(m_liveView->layout(), SIGNAL(idealSizeChanged(QSize)), SLOT(doAutoResize()));
        doAutoResize();
    }
    else
        disconnect(m_liveView->layout(), SIGNAL(idealSizeChanged(QSize)), this, SLOT(doAutoResize()));
}

void LiveViewWindow::doAutoResize()
{
    if (!m_autoSized)
        return;

    m_liveView->updateGeometry();
    if (m_liveView->sizeHint().isEmpty())
        return;

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

void LiveViewWindow::showSingleCamera(const DVRCamera &camera)
{
    m_liveView->layout()->setGridSize(1, 1);
    QDeclarativeItem *item = m_liveView->layout()->addItem(0, 0);
    item->setProperty("camera", QVariant::fromValue(camera));
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

void LiveViewWindow::savedLayoutChanged(int index)
{
    if (m_isLayoutChanging)
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
    if (!data.isEmpty() && !m_liveView->layout()->loadLayout(data))
        qDebug() << "Failed to load camera layout" << m_savedLayouts->itemText(index);

    m_lastLayoutIndex = index;
    emit layoutChanged(currentLayout());

    aRenameLayout->setEnabled(index >= 0);
    aDelLayout->setEnabled(index >= 0);

    m_isLayoutChanging = false;
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
    m_savedLayouts->insertItem(index, name, m_liveView->layout()->saveLayout());

    m_isLayoutChanging = false;
    m_savedLayouts->setCurrentIndex(index);
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

    QByteArray data = m_liveView->layout()->saveLayout();
    m_savedLayouts->setItemData(m_lastLayoutIndex, data, SavedLayoutsModel::LayoutDataRole);
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
            setWindowFlags(windowFlags() | Qt::Window);
            m_fsSetWindow = true;
        }

        showFullScreen();
    }
    else
    {
        if (m_fsSetWindow)
        {
            setWindowFlags(windowFlags() & ~Qt::Window);
            m_fsSetWindow = false;
        }

        showNormal();
    }

    QSettings settings;
    if (settings.value(QLatin1String("ui/disableScreensaver/onFullscreen")).toBool())
        bcApp->setScreensaverInhibited(on);
}
