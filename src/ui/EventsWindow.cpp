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
#include "event/EventsUpdater.h"
#include "event/MediaEventFilter.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QDateTimeEdit>
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
#include <QEvent>
#include <QPushButton>
#include <QDebug>

EventsWindow::EventsWindow(DVRServerRepository *serverRepository, QWidget *parent)
	: QWidget(parent, Qt::Window), m_serverRepository(serverRepository),
	  m_tagsLabel(0), m_tagInput(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(QSize(900, 600));

    QBoxLayout *layout = new QHBoxLayout(this);
    QBoxLayout *filtersLayout = new QVBoxLayout;
    layout->addLayout(filtersLayout);

    m_eventsUpdater = new EventsUpdater(bcApp->serverRepository(), this);
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

    connect(sourcesModel, SIGNAL(checkedSourcesChanged(QMap<DVRServer*,QSet<int>>)),
            this, SLOT(setFilterSources(QMap<DVRServer*,QSet<int>>)));


    //createLoadButton(filtersLayout);
    createRangeSelector(filtersLayout);
    createDateTimeFilter(filtersLayout);
    rangeSelectorChanged();

#if 1 /* This is not useful currently. */
	m_minimumLevelLabel = new QLabel;
	m_minimumLevelLabel->setStyleSheet(QLatin1String("font-weight:bold;"));
	filtersLayout->addWidget(m_minimumLevelLabel);
    filtersLayout->addWidget(createLevelFilter());
#endif

	m_typeLabel = new QLabel(tr("Type"));
	m_typeLabel->setStyleSheet(QLatin1String("font-weight:bold;"));
	filtersLayout->addWidget(m_typeLabel);
    filtersLayout->addWidget(createTypeFilter());

#if 0 /* Tags are not fully implemented yet */
	m_tagsLabel= new QLabel;
	m_tagsLabel->setStyleSheet(QLatin1String("font-weight:bold;"));
	filtersLayout->addWidget(m_tagsLabel);
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

	m_timelineContainer = createTimeline();

    m_resultTabs->addTab(m_resultsView, tr("List"));
	m_resultTabs->addTab(m_timelineContainer, tr("Timeline"));

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

	retranslateUI();
}

EventsWindow::~EventsWindow()
{
}

void EventsWindow::changeEvent(QEvent *event)
{
	if (event && event->type() == QEvent::LanguageChange)
		retranslateUI();

	QWidget::changeEvent(event);
}
/*
void EventsWindow::createLoadButton(QBoxLayout *layout)
{
    m_loadEvents = new QPushButton;
    m_loadEvents->setStyleSheet(QLatin1String("font-weight:bold;"));
    layout->addWidget(m_loadEvents);

    connect(m_loadEvents, SIGNAL(clicked()), this, SLOT(loadEvents()));
}*/

void EventsWindow::createRangeSelector(QBoxLayout *layout)
{
    m_rangeSelector = new QComboBox;
    m_rangeSelector->setStyleSheet(QLatin1String("font-weight:bold;"));
    m_rangeSelector->addItem(tr("Last    hour "), EventsWindow::Last1Hour);
    m_rangeSelector->addItem(tr("Last  6 hours"), EventsWindow::Last6Hours);
    m_rangeSelector->addItem(tr("Last 12 hours"), EventsWindow::Last12Hours);
    m_rangeSelector->addItem(tr("Last 24 hours"), EventsWindow::Last24Hours);
    m_rangeSelector->addItem(tr("Select time range"), EventsWindow::SelectTimeRange);

    QSettings settings;
    m_rangeSelector->setCurrentIndex(settings.value(QLatin1String("ui/events/selectedRange"), 0).toInt());

    layout->addWidget(m_rangeSelector);

    connect(m_rangeSelector, SIGNAL(activated (int)), this, SLOT(rangeSelectorChanged()));
}

void EventsWindow::createDateTimeFilter(QBoxLayout *layout)
{
    m_fromDateTimeLabel = new QLabel;
    m_toDateTimeLabel = new QLabel;
    m_fromDateTimeLabel->setStyleSheet(QLatin1String("font-weight:bold;"));
    m_toDateTimeLabel->setStyleSheet(QLatin1String("font-weight:bold;"));

    layout->addWidget(m_fromDateTimeLabel);

    QSettings settings;
    QDateTimeEdit *fromDateEdit = new QDateTimeEdit(settings.value(QLatin1String("ui/events/fromDateTime"), QDateTime::currentDateTime().addSecs(-60*60*1)).toDateTime());
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setMaximumDate(QDate::currentDate());
    fromDateEdit->setDisplayFormat(QLatin1String("ddd, MMM dd, yyyy hh:mm"));
    //fromDateEdit->setTime(QTime(23, 59, 59, 999));
    fromDateEdit->setFixedWidth(m_sourcesView->width());
    layout->addWidget(fromDateEdit);
    m_fromDateTime = fromDateEdit;

    layout->addWidget(m_toDateTimeLabel);

    QDateTimeEdit *toDateEdit = new QDateTimeEdit(settings.value(QLatin1String("ui/events/toDateTime"), QDateTime::currentDateTime()).toDateTime());
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setMaximumDate(QDate::currentDate());
    toDateEdit->setDisplayFormat(QLatin1String("ddd, MMM dd, yyyy hh:mm"));
    //toDateEdit->setTime(QTime(23, 59, 59, 999));
    toDateEdit->setFixedWidth(m_sourcesView->width());
    layout->addWidget(toDateEdit);
    m_toDateTime = toDateEdit;

    //setFilterDay(dateEdit->dateTime());
    //setFilterDateTimeRange();

    connect(fromDateEdit, SIGNAL(dateTimeChanged(QDateTime)), this,
            SLOT(setFilterDateTimeRange()));
    connect(toDateEdit, SIGNAL(dateTimeChanged(QDateTime)), this,
            SLOT(setFilterDateTimeRange()));

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

    connect(m_typeFilter, SIGNAL(checkedTypesChanged(QBitArray)), this, SLOT(setFilterTypes(QBitArray)));

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
	m_tagInput = new QComboBox;
	m_tagInput->setEditable(true);
	m_tagInput->setInsertPolicy(QComboBox::NoInsert);

	return m_tagInput;
}

QWidget * EventsWindow::createResultsView()
{
    m_resultsView = new EventsView;
    connect(m_eventsUpdater, SIGNAL(loadingStarted()), m_resultsView, SLOT(loadingStarted()));
    connect(m_eventsUpdater, SIGNAL(loadingFinished()), m_resultsView, SLOT(loadingFinished()));

    EventsModel *eventsModel = new EventsModel(m_serverRepository, this);
    m_resultsView->setModel(eventsModel, m_eventsUpdater->isUpdating());

    connect(m_eventsUpdater, SIGNAL(serverEventsAvailable(DVRServer*,QList<QSharedPointer<EventData> >)),
            eventsModel, SLOT(setServerEvents(DVRServer*,QList<QSharedPointer<EventData> >)));

    m_resultsView->setFrameStyle(QFrame::NoFrame);
    m_resultsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_resultsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventContextMenu(QPoint)));
    connect(m_resultsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(showServerEvent(QModelIndex)));

    QSettings settings;
    m_resultsView->header()->restoreState(settings.value(QLatin1String("ui/events/viewHeader")).toByteArray());
    m_resultsView->header()->setSortIndicatorShown(true);
    m_resultsView->header()->setSortIndicator(EventsModel::DateColumn, Qt::DescendingOrder);
    connect(m_resultsView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            m_resultsView, SLOT(sortEvents(int,Qt::SortOrder)));

    return m_resultsView;
}

QWidget *EventsWindow::createTimeline()
{
    QWidget *container = new QWidget;
    QGridLayout *layout = new QGridLayout(container);

    m_timeline = new EventTimelineWidget;
    m_timeline->setContextMenuPolicy(Qt::CustomContextMenu);
    m_timeline->setModel(m_resultsView->model());

    m_timelineZoom = new QSlider(Qt::Horizontal);
    m_timelineZoom->setRange(0, 100);
    m_timelineZoom->setValue(0);

    connect(m_timeline, SIGNAL(customContextMenuRequested(QPoint)), SLOT(eventContextMenu(QPoint)));
    connect(m_timeline, SIGNAL(doubleClicked(QModelIndex)), SLOT(showServerEvent(QModelIndex)));

    connect(m_timelineZoom, SIGNAL(valueChanged(int)), SLOT(timelineSliderChanged(int)));

    layout->addWidget(m_timeline, 0, 0, 1, 2);

	m_zoomLabel = new QLabel;
	layout->addWidget(m_zoomLabel, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(m_timelineZoom, 1, 1);
	return container;
}

void EventsWindow::retranslateUI()
{
	setWindowTitle(tr("Bluecherry - Event Browser"));
	m_minimumLevelLabel->setText(tr("Minimum Level"));
	m_typeLabel->setText(tr("Type"));
	if (m_tagsLabel)
		m_tagsLabel->setText(tr("Tags"));

	m_resultTabs->setTabText(m_resultTabs->indexOf(m_resultsView), tr("List"));
	m_resultTabs->setTabText(m_resultTabs->indexOf(m_timelineContainer), tr("Timeline"));

	m_zoomLabel->setText(tr("Zoom:"));
	if (m_tagInput)
		m_tagInput->lineEdit()->setPlaceholderText(tr("Type or select a tag to filter"));
    m_fromDateTimeLabel->setText(tr("From:"));
    m_toDateTimeLabel->setText(tr("To:"));

	m_levelFilter->blockSignals(true);
	m_levelFilter->setItemText(0, tr("Any"));
	m_levelFilter->setItemText(1, tr("Info"));
	m_levelFilter->setItemText(2, tr("Warning"));
	m_levelFilter->setItemText(3, tr("Alarm"));
	m_levelFilter->setItemText(4, tr("Critical"));
	m_levelFilter->blockSignals(false);

    m_rangeSelector->blockSignals(true);
    m_rangeSelector->setItemText(0, tr("Last    hour "));
    m_rangeSelector->setItemText(1, tr("Last  6 hours"));
    m_rangeSelector->setItemText(2, tr("Last 12 hours"));
    m_rangeSelector->setItemText(3, tr("Last 24 hours"));
    m_rangeSelector->setItemText(4, tr("Select time range"));
    m_rangeSelector->blockSignals(false);

    //m_loadEvents->setText(tr("Load Events"));
}

void EventsWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(QLatin1String("ui/events/geometry"), saveGeometry());
    settings.setValue(QLatin1String("ui/events/videoSplitter"), m_videoSplitter->saveState());
    settings.setValue(QLatin1String("ui/events/viewHeader"), m_resultsView->header()->saveState());
    settings.setValue(QLatin1String("ui/events/selectedRange"), m_rangeSelector->currentIndex());
    settings.setValue(QLatin1String("ui/events/fromDateTime"), m_fromDateTime->dateTime());
    settings.setValue(QLatin1String("ui/events/toDateTime"), m_toDateTime->dateTime());
    QWidget::closeEvent(event);
}

void EventsWindow::rangeSelectorChanged()
{
    enum EventsTimeRange range = (EventsTimeRange)m_rangeSelector->itemData(m_rangeSelector->currentIndex()).toInt();
    int hours = -1;
    QDateTime from, to;

    m_fromDateTime->blockSignals(true);
    m_toDateTime->blockSignals(true);

    switch (range)
    {
        case Last1Hour:
        hours = 1;
        break;

        case Last6Hours:
        hours = 6;
        break;

        case Last12Hours:
        hours = 12;
        break;

        case Last24Hours:
        hours = 24;
        break;

        case SelectTimeRange:
        m_fromDateTime->setEnabled(true);
        m_toDateTime->setEnabled(true);
    }

    if (hours > 0)
    {
        from = QDateTime::currentDateTime().addSecs(-60*60*hours);
        to = QDateTime::currentDateTime();

        m_fromDateTime->setEnabled(false);
        m_toDateTime->setEnabled(false);

        m_fromDateTime->setDateTime(from);
        m_toDateTime->setDateTime(to);
    }
    else
    {
        from = m_fromDateTime->dateTime();
        to = m_toDateTime->dateTime();
    }

    m_eventsUpdater->setTimeRange(from, to);
    m_resultsView->setTimeRange(from, to);

    //qDebug() << "EventsWindow::rangeSelectorChanged " << hours << "\n";

    loadEvents();

    m_fromDateTime->blockSignals(false);
    m_toDateTime->blockSignals(false);
}

void EventsWindow::levelFilterChanged()
{
    int level = m_levelFilter->itemData(m_levelFilter->currentIndex()).toInt();
    if (level < 0)
        level = EventLevel::Minimum;

    m_resultsView->setMinimumLevel((EventLevel::Level)level);
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
    m_modelEventsCursor->blockSignals(true);
    m_modelEventsCursor->setIndex(index.row());
    m_modelEventsCursor->blockSignals(false);
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

void EventsWindow::setFilterTypes(QBitArray types)
{
    m_resultsView->setTypes(types);
}

void EventsWindow::setFilterDateTimeRange()
{
    QDateTime from, to;

    from = m_fromDateTime->dateTime();
    to = m_toDateTime->dateTime();

    m_eventsUpdater->setTimeRange(from, to);
    m_resultsView->setTimeRange(from, to);

    //qDebug() << "EventsWindow::setFilterDateTimeRange()" << from << to <<"\n";

    loadEvents();
}

void EventsWindow::loadEvents()
{
    m_eventsUpdater->updateServers();
}

void EventsWindow::setFilterSources(const QMap<DVRServer *, QSet<int> > &sources)
{
    m_resultsView->setSources(sources);
}
