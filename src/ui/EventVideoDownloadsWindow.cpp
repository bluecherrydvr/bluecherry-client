#include "EventVideoDownloadsWindow.h"
#include "event/EventDownloadManager.h"
#include "event/EventVideoDownload.h"
#include "ui/EventVideoDownloadWidget.h"
#include <QApplication>
#include <QFrame>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>

EventVideoDownloadsWindow::EventVideoDownloadsWindow(QWidget *parent) :
    QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(550, 300);
    setWindowTitle(tr("Bluecherry - Download Manager"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setMargin(2);

    QScrollArea *downloadArea = new QScrollArea(this);
    downloadArea->setBackgroundRole(QPalette::Base );
    downloadArea->move(0, 0);
    layout->addWidget(downloadArea);

    QFrame *downloadFrame = new QFrame;
    downloadFrame->setBackgroundRole(QPalette::Base);
    downloadFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    m_downloadLayout = new QVBoxLayout(downloadFrame);
    m_downloadLayout->setDirection(QBoxLayout::Up);

    downloadArea->setWidget(downloadFrame);
    downloadArea->setWidgetResizable(true);

    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/downloadsWindow/geometry")).toByteArray());

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
}

EventVideoDownloadsWindow::~EventVideoDownloadsWindow()
{
    saveSettings();
}

void EventVideoDownloadsWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(QLatin1String("ui/downloadsWindow/geometry"), saveGeometry());
}

void EventVideoDownloadsWindow::setEventDownloadManager(EventDownloadManager *eventDownloadManager)
{
    if (m_eventDownloadManager.data() == eventDownloadManager)
        return;

    if (m_eventDownloadManager)
        disconnect(m_eventDownloadManager.data(), 0, this, 0);

    m_eventDownloadManager = eventDownloadManager;

    if (m_eventDownloadManager)
    {
        connect(m_eventDownloadManager.data(), SIGNAL(eventVideoDownloadAdded(EventVideoDownload*)),
                this, SLOT(eventVideoDownloadAdded(EventVideoDownload*)));
        connect(m_eventDownloadManager.data(), SIGNAL(eventVideoDownloadRemoved(EventVideoDownload*)),
                this, SLOT(eventVideoDownloadRemoved(EventVideoDownload*)));
    }

    rebuildWidgets();
}

void EventVideoDownloadsWindow::eventVideoDownloadAdded(EventVideoDownload *eventVideoDownload)
{
    if (!eventVideoDownload)
        return;

    EventVideoDownloadWidget *widget = new EventVideoDownloadWidget(eventVideoDownload);
    m_downloadLayout->addWidget(widget);

    m_downloadWidgets.insert(eventVideoDownload, widget);
}

void EventVideoDownloadsWindow::eventVideoDownloadRemoved(EventVideoDownload *eventVideoDownload)
{
    if (!eventVideoDownload || !m_downloadWidgets.contains(eventVideoDownload))
        return;

    EventVideoDownloadWidget *widget = m_downloadWidgets.value(eventVideoDownload);
    delete widget;
    m_downloadWidgets.remove(eventVideoDownload);
}

void EventVideoDownloadsWindow::rebuildWidgets()
{
    qDeleteAll(m_downloadWidgets);
    m_downloadWidgets.clear();

    if (!m_eventDownloadManager)
        return;

    foreach (EventVideoDownload *eventVideoDownload, m_eventDownloadManager.data()->list())
        eventVideoDownloadAdded(eventVideoDownload);
}
