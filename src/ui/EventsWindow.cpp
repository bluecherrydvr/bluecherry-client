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

#include "EventsWindow.h"
#include "DVRServersView.h"
#include "model/EventSourcesModel.h"
#include "EventsView.h"
#include "EventTimelineWidget.h"
#include "EventViewWindow.h"
#include "EventTypesFilter.h"
#include "EventTagsView.h"
#include "model/EventTagsModel.h"
#include "model/EventsModel.h"
#include "model/EventsProxyModel.h"
#include "core/BluecherryApp.h"
#include "event/ModelEventsCursor.h"
#include "ui/MainWindow.h"
#include "ui/model/DVRServersProxyModel.h"
#include "event/CameraEventFilter.h"
#include "event/EventDownloadManager.h"
#include "event/EventList.h"
#include "event/MediaEventFilter.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QTreeView>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QLineEdit>
#include <QHeaderView>
#include <QTabWidget>
#include <QMenu>
#include <QAction>

EventsWindow::EventsWindow(DVRServerRepository *serverRepository, QWidget *parent)
    : QWidget(parent, Qt::Window), m_serverRepository(serverRepository)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Bluecherry - Event Browser"));
    resize(QSize(900, 600));

    QBoxLayout *layout = new QHBoxLayout(this);
    QBoxLayout *filtersLayout = new QVBoxLayout;
    layout->addLayout(filtersLayout);

    createResultsView();

    /* Filters */
    m_sourcesView = new DVRServersView(m_serverRepository);
    EventSourcesModel *sourcesModel = new EventSourcesModel(m_serverRepository, m_sourcesView);

    DVRServersProxyModel *proxyModel = new DVRServersProxyModel(sourcesModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(sourcesModel);
    proxyModel->sort(0);

    m_sourcesView->setModel(proxyModel);
    m_sourcesView->setMaximumWidth(180);
    //m_sourcesView->setMaximumHeight(150);
    filtersLayout->addWidget(m_sourcesView);

    connect(sourcesModel, SIGNAL(checkedSourcesChanged(QMap<DVRServer*,QList<int>>)),
            m_resultsView->eventsModel(), SLOT(setFilterSources(QMap<DVRServer*,QList<int>>)));

    createDateFilter(filtersLayout);

#if 1 /* This is not useful currently. */
    QLabel *label = new QLabel(tr("Minimum Level"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createLevelFilter());
#endif

    label = new QLabel(tr("Type"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createTypeFilter());

#if 0 /* Tags are not fully implemented yet */
    label = new QLabel(tr("Tags"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createTags());
    filtersLayout->addWidget(createTagsInput());
#endif

    /* Splitter between results and playback */
    m_videoSplitter = new QSplitter(Qt::Vertical);
    layout->addWidget(m_videoSplitter, 1);

    /* Results */
    m_resultTabs = new QTabWidget;
    m_videoSplitter->addWidget(m_resultTabs);
    m_videoSplitter->setCollapsible(0, false);

    m_resultTabs->addTab(m_resultsView, tr("List"));
    m_resultTabs->addTab(createTimeline(), tr("Timeline"));

    /* Playback */
    m_eventViewer = new EventViewWindow;
    m_eventViewer->layout()->setMargin(0);
    m_eventViewer->hide();
    m_videoSplitter->addWidget(m_eventViewer);

    m_modelEventsCursor = new ModelEventsCursor();
    m_modelEventsCursor->setModel(m_resultsView->model());
    m_eventViewer->setEventsCursor(m_modelEventsCursor);
    connect(m_modelEventsCursor, SIGNAL(indexUpdated()), this, SLOT(cursorIndexUpdated()));

    /* Settings */
    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/events/geometry")).toByteArray());
    m_videoSplitter->restoreState(settings.value(QLatin1String("ui/events/videoSplitter")).toByteArray());
}

EventsWindow::~EventsWindow()
{
}

EventsModel *EventsWindow::model() const
{
    return m_resultsView->eventsModel();
}

void EventsWindow::createDateFilter(QBoxLayout *layout)
{
    QLabel *title = new QLabel(tr("Date"));
    title->setStyleSheet(QLatin1String("font-weight:bold;"));
    layout->addWidget(title);

    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setMaximumDate(QDate::currentDate());
    dateEdit->setDisplayFormat(QLatin1String("ddd, MMM dd, yyyy"));
    dateEdit->setTime(QTime(23, 59, 59, 999));
    dateEdit->setFixedWidth(m_sourcesView->width());
    layout->addWidget(dateEdit);

    connect(dateEdit, SIGNAL(dateTimeChanged(QDateTime)), m_resultsView->eventsModel(),
            SLOT(setFilterDay(QDateTime)));
}

QWidget *EventsWindow::createLevelFilter()
{
    m_levelFilter = new QComboBox;
    m_levelFilter->addItem(tr("Any"), -1);
    m_levelFilter->addItem(tr("Info"), EventLevel::Info);
    m_levelFilter->addItem(tr("Warning"), EventLevel::Warning);
    m_levelFilter->addItem(tr("Alarm"), EventLevel::Alarm);
    m_levelFilter->addItem(tr("Critical"), EventLevel::Critical);

    connect(m_levelFilter, SIGNAL(currentIndexChanged(int)), SLOT(levelFilterChanged()));
    return m_levelFilter;
}

QWidget *EventsWindow::createTypeFilter()
{
    m_typeFilter = new EventTypesFilter;
    m_typeFilter->setMaximumWidth(180);
    m_typeFilter->setMaximumHeight(100);

    connect(m_typeFilter, SIGNAL(checkedTypesChanged(QBitArray)), m_resultsView->eventsModel(), SLOT(setFilterTypes(QBitArray)));

    return m_typeFilter;
}

QWidget *EventsWindow::createTags()
{
    m_tagsView = new EventTagsView;
    m_tagsView->setModel(new EventTagsModel(m_tagsView));
    m_tagsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return m_tagsView;
}

QWidget *EventsWindow::createTagsInput()
{
    QComboBox *tagInput = new QComboBox;
    tagInput->setEditable(true);
    tagInput->setInsertPolicy(QComboBox::NoInsert);
#if QT_VERSION >= 0x040700
    tagInput->lineEdit()->setPlaceholderText(tr("Type or select a tag to filter"));
#endif

    return tagInput;
}

QWidget * EventsWindow::createResultsView()
{
    m_resultsView = new EventsView;
    m_resultsView->setModel(new EventsModel(m_serverRepository, this));
    m_resultsView->setFrameStyle(QFrame::NoFrame);
    m_resultsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_resultsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventContextMenu(QPoint)));
    connect(m_resultsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(showServerEvent(QModelIndex)));

    QSettings settings;
    m_resultsView->header()->restoreState(settings.value(QLatin1String("ui/events/viewHeader")).toByteArray());
    m_resultsView->header()->setSortIndicatorShown(true);
    m_resultsView->header()->setSortIndicator(EventsModel::DateColumn, Qt::DescendingOrder);
    connect(m_resultsView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(sortEvents(int,Qt::SortOrder)));

    return m_resultsView;
}

QWidget *EventsWindow::createTimeline()
{
    QWidget *container = new QWidget;
    QGridLayout *layout = new QGridLayout(container);

    m_timeline = new EventTimelineWidget;
    m_timeline->setContextMenuPolicy(Qt::CustomContextMenu);
    m_timeline->setModel(m_resultsView->eventsModel());

    m_timelineZoom = new QSlider(Qt::Horizontal);
    m_timelineZoom->setRange(0, 100);
    m_timelineZoom->setValue(0);

    connect(m_timeline, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventContextMenu(QPoint)));
    connect(m_timeline, SIGNAL(doubleClicked(QModelIndex)), SLOT(showServerEvent(QModelIndex)));

    connect(m_timelineZoom, SIGNAL(valueChanged(int)), SLOT(timelineSliderChanged(int)));

    layout->addWidget(m_timeline, 0, 0, 1, 2);

    QLabel *label = new QLabel(tr("Zoom:"));
    layout->addWidget(label, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(m_timelineZoom, 1, 1);
    return container;
}

void EventsWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(QLatin1String("ui/events/geometry"), saveGeometry());
    settings.setValue(QLatin1String("ui/events/videoSplitter"), m_videoSplitter->saveState());
    settings.setValue(QLatin1String("ui/events/viewHeader"), m_resultsView->header()->saveState());
    QWidget::closeEvent(event);
}

void EventsWindow::levelFilterChanged()
{
    int level = m_levelFilter->itemData(m_levelFilter->currentIndex()).toInt();
    if (level < 0)
        level = EventLevel::Minimum;

    m_resultsView->eventsProxyModel()->setMinimumLevel((EventLevel::Level)level);
}

void EventsWindow::cursorIndexUpdated()
{
    const int row = m_modelEventsCursor->index();

    QItemSelectionModel *selectionModel = m_resultsView->selectionModel();
    const QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if (selectedIndexes.count() == 1 && selectedIndexes.at(0).row() == row)
        return;

    selectionModel->select(m_resultsView->model()->index(row, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void EventsWindow::timelineZoomChanged(int value)
{
    m_timelineZoom->setValue(m_timelineZoom->maximum() - (value - m_timelineZoom->minimum()));
}

void EventsWindow::timelineSliderChanged(int value)
{
    m_timeline->setZoomLevel(value);
}

void EventsWindow::showServerEvent(const QModelIndex &index)
{
    EventData *data = index.data(EventsModel::EventDataPtr).value<EventData*>();
    showServerEvent(*data);

    m_modelEventsCursor->setCameraFilter(data->locationCamera());
    m_modelEventsCursor->setIndex(index.row());
}

void EventsWindow::showServerEvent(const EventData &eventData)
{
    if (!eventData.hasMedia())
        return;

    m_eventViewer->setEvent(eventData);

    /* Hack to ensure that the video area isn't collapsed */
    if (m_videoSplitter->sizes()[1] == 0)
        m_videoSplitter->setSizes(QList<int>() << m_videoSplitter->sizes()[0] << 1);
    m_eventViewer->show();
}

void EventsWindow::eventContextMenu(const QPoint &pos)
{
    EventsView *view = qobject_cast<EventsView*>(sender());
    if (!view)
        return;

    EventList selectedEvents = view->selectedEvents();
    EventList selectedMediaEvents = selectedEvents.filter(MediaEventFilter());
    EventList selectedCameraEvents = selectedEvents.filter(CameraEventFilter());

    QMenu menu(view);

    QAction *aPlay = menu.addAction(tr("Play video"));
    aPlay->setEnabled(selectedMediaEvents.size() == 1);
    menu.setDefaultAction(aPlay);

    QAction *aPlayWindow = menu.addAction(tr("Play in a new window"));
    aPlayWindow->setEnabled(selectedMediaEvents.size() == 1);
    menu.addSeparator();

    QAction *aSave = menu.addAction(tr("Save video"));
    aSave->setEnabled(!selectedMediaEvents.isEmpty());
    menu.addSeparator();

    QAction *aSelectOnly = menu.addAction(tr("Show only this camera"));
    aSelectOnly->setEnabled(!selectedCameraEvents.isEmpty());
    QAction *aSelectElse = menu.addAction(tr("Exclude this camera"));
    aSelectElse->setEnabled(!selectedCameraEvents.isEmpty());

    QAction *act = menu.exec(view->mapToGlobal(pos));

    if (!act)
        return;
    else if (act == aPlay)
        showServerEvent(view->currentIndex());
    else if (act == aPlayWindow)
    {
        ModelEventsCursor *modelEventsCursor = new ModelEventsCursor();
        modelEventsCursor->setModel(view->model());
        modelEventsCursor->setCameraFilter(selectedMediaEvents.at(0).locationCamera());
        modelEventsCursor->setIndex(view->currentIndex().row());
        EventViewWindow::open(selectedMediaEvents.at(0), modelEventsCursor);
    }
    else if (act == aSave)
    {
        if (selectedMediaEvents.size() == 1)
            bcApp->eventDownloadManager()->startEventDownload(selectedMediaEvents.at(0));
        else
            bcApp->eventDownloadManager()->startMultipleEventDownloads(selectedMediaEvents);
    }
    else if (act == aSelectOnly || act == aSelectElse)
    {
        EventSourcesModel *sModel = qobject_cast<EventSourcesModel*>(m_sourcesView->model());
        Q_ASSERT(sModel);
        if (!sModel)
            return;

        QSet<DVRCamera *> cameras = selectedCameraEvents.cameras();
        QModelIndex sIdx = sModel->indexOfCamera(*cameras.begin());

        if (act == aSelectOnly)
        {
            m_sourcesView->checkOnlyIndex(sIdx); // uncheck all, some kind of temporary hack
            foreach (DVRCamera *camera, cameras)
                sModel->setData(sModel->indexOfCamera(camera), Qt::Checked, Qt::CheckStateRole);
        }
        else
        {
            foreach (DVRCamera *camera, cameras)
                sModel->setData(sModel->indexOfCamera(camera), Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void EventsWindow::sortEvents(int logicalIndex, Qt::SortOrder sortOrder)
{
    m_resultsView->eventsProxyModel()->setDynamicSortFilter(false);
    m_resultsView->eventsProxyModel()->setColumn(logicalIndex);
    m_resultsView->eventsProxyModel()->sort(0, sortOrder);
    m_resultsView->eventsProxyModel()->setDynamicSortFilter(true);
}
