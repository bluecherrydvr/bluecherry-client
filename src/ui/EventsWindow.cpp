#include "EventsWindow.h"
#include "DVRServersView.h"
#include "EventSourcesModel.h"
#include "EventsView.h"
#include "EventTimelineWidget.h"
#include "EventViewWindow.h"
#include "EventTypesFilter.h"
#include "EventTagsView.h"
#include "EventTagsModel.h"
#include "EventsModel.h"
#include "core/BluecherryApp.h"
#include "ui/MainWindow.h"
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
#include <QTabWidget>I pa

EventsWindow *EventsWindow::m_instance = 0;

EventsWindow::EventsWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle(tr("Bluecherry - Event Viewer"));
    resize(QSize(900, 600));

    QBoxLayout *layout = new QHBoxLayout(this);
    QBoxLayout *filtersLayout = new QVBoxLayout;
    layout->addLayout(filtersLayout);

    createResultsView();

    /* Filters */
    m_sourcesView = new DVRServersView;
    EventSourcesModel *sourcesModel = new EventSourcesModel(m_sourcesView);
    m_sourcesView->setModel(sourcesModel);
    m_sourcesView->setMaximumWidth(180);
    //m_sourcesView->setMaximumHeight(150);
    filtersLayout->addWidget(m_sourcesView);

    connect(sourcesModel, SIGNAL(checkedSourcesChanged(QMap<DVRServer*,QList<int>>)),
            m_resultsView->eventsModel(), SLOT(setFilterSources(QMap<DVRServer*,QList<int>>)));

    createDateFilter(filtersLayout);

    QLabel *label = new QLabel(tr("Minimum Level"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createLevelFilter());

#if 0 /* The server currently only implements Motion events, so this is just confusing. */
    label = new QLabel(tr("Type"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createTypeFilter());
#endif

#if 0 /* Tags are not fully implemented yet */
    label = new QLabel(tr("Tags"));
    label->setStyleSheet(QLatin1String("font-weight:bold;"));
    filtersLayout->addWidget(label);
    filtersLayout->addWidget(createTags());
    filtersLayout->addWidget(createTagsInput());
#endif

    /* Results */
    m_resultTabs = new QTabWidget;
    //m_resultTabs->setDocumentMode(true);
    layout->addWidget(m_resultTabs, 1);

    m_resultTabs->addTab(m_resultsView, tr("List"));
    m_resultTabs->addTab(createTimeline(), tr("Timeline"));

//    resultLayout->addWidget(createResultTitle());

    /* Settings */
    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/events/geometry")).toByteArray());
}

EventsWindow::~EventsWindow()
{
    if (m_instance == this)
        m_instance = 0;
}

EventsWindow *EventsWindow::instance()
{
    if (!m_instance)
    {
        m_instance = new EventsWindow(bcApp->mainWindow);
        m_instance->setAttribute(Qt::WA_DeleteOnClose);
    }

    return m_instance;
}

void EventsWindow::createDateFilter(QBoxLayout *layout)
{
    QCheckBox *title = new QCheckBox(tr("Date after..."));
    title->setStyleSheet(QLatin1String("font-weight:bold;"));
    layout->addWidget(title);
    connect(title, SIGNAL(clicked(bool)), SLOT(setStartDateEnabled(bool)));

    m_startDate = new QDateEdit(QDate::currentDate());
    m_startDate->setCalendarPopup(true);
    m_startDate->setMaximumDate(QDate::currentDate());
    m_startDate->setDisplayFormat(QLatin1String("dddd, MMM dd, yyyy"));
    m_startDate->setVisible(false);
    layout->addWidget(m_startDate);

    connect(m_startDate, SIGNAL(dateTimeChanged(QDateTime)), m_resultsView->eventsModel(),
            SLOT(setFilterBeginDate(QDateTime)));
    connect(m_startDate, SIGNAL(dateTimeChanged(QDateTime)), SLOT(setEndDateMinimum(QDateTime)));
    title->setChecked(false);

    title = new QCheckBox(tr("Date before..."));
    title->setStyleSheet(QLatin1String("font-weight:bold;"));
    layout->addWidget(title);
    connect(title, SIGNAL(clicked(bool)), SLOT(setEndDateEnabled(bool)));

    m_endDate = new QDateEdit(QDate::currentDate());
    m_endDate->setCalendarPopup(true);
    m_endDate->setMaximumDate(QDate::currentDate());
    m_endDate->setDisplayFormat(QLatin1String("dddd, MMM dd, yyyy"));
    m_endDate->setTime(QTime(23, 59, 59, 999));
    m_endDate->setVisible(false);
    layout->addWidget(m_endDate);

    connect(m_endDate, SIGNAL(dateTimeChanged(QDateTime)), m_resultsView->eventsModel(),
            SLOT(setFilterEndDate(QDateTime)));
}

void EventsWindow::setStartDateEnabled(bool enabled)
{
    m_startDate->setVisible(enabled);
    if (enabled)
    {
        m_resultsView->eventsModel()->setFilterBeginDate(m_startDate->dateTime());
        setEndDateMinimum(m_startDate->dateTime());
    }
    else
    {
        m_resultsView->eventsModel()->setFilterBeginDate(QDateTime());
        setEndDateMinimum(QDateTime());
    }
}

void EventsWindow::setEndDateEnabled(bool enabled)
{
    m_endDate->setVisible(enabled);
    if (enabled)
        m_resultsView->eventsModel()->setFilterEndDate(m_endDate->dateTime());
    else
        m_resultsView->eventsModel()->setFilterEndDate(QDateTime());
}

void EventsWindow::setEndDateMinimum(const QDateTime &date)
{
    m_endDate->setMinimumDate(date.date());
    m_endDate->setTime(QTime(23, 59, 59, 999));
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

QWidget *EventsWindow::createResultTitle()
{
    m_resultTitle = new QLabel;
    QFont font = m_resultTitle->font();
    font.setPointSize(font.pointSize()+4);
    m_resultTitle->setFont(font);
    m_resultTitle->setWordWrap(true);

    connect(m_resultsView->eventsModel(), SIGNAL(filtersChanged()), SLOT(updateResultTitle()));
    updateResultTitle();

    return m_resultTitle;
}

QWidget *EventsWindow::createResultsView()
{
    m_resultsView = new EventsView;
    m_resultsView->setModel(new EventsModel(this));
    m_resultsView->setFrameStyle(QFrame::NoFrame);
    connect(m_resultsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(showEvent(QModelIndex)));

    QSettings settings;
    m_resultsView->header()->restoreState(settings.value(QLatin1String("ui/events/viewHeader")).toByteArray());
    m_resultsView->header()->setSortIndicatorShown(true);
    m_resultsView->header()->setSortIndicator(EventsModel::DateColumn, Qt::DescendingOrder);
    m_resultsView->setSortingEnabled(true);

    return m_resultsView;
}

QWidget *EventsWindow::createTimeline()
{
    QWidget *container = new QWidget;
    QGridLayout *layout = new QGridLayout(container);

    m_timeline = new EventTimelineWidget;
    m_timeline->setModel(m_resultsView->eventsModel());
    //m_timeline->setSelectionModel(m_resultsView->selectionModel());

    m_timelineZoom = new QSlider(Qt::Horizontal);
    timelineZoomRangeChanged(m_timeline->minZoomSeconds(), m_timeline->maxZoomSeconds());
    timelineZoomChanged(m_timeline->zoomSeconds());

    connect(m_timeline, SIGNAL(doubleClicked(QModelIndex)), SLOT(showEvent(QModelIndex)));

    connect(m_timeline, SIGNAL(zoomSecondsChanged(int)), SLOT(timelineZoomChanged(int)));
    connect(m_timeline, SIGNAL(zoomRangeChanged(int,int)), SLOT(timelineZoomRangeChanged(int,int)));

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
    settings.setValue(QLatin1String("ui/events/viewHeader"), m_resultsView->header()->saveState());
    QWidget::closeEvent(event);
}

void EventsWindow::levelFilterChanged()
{
    int level = m_levelFilter->itemData(m_levelFilter->currentIndex()).toInt();
    if (level < 0)
        level = EventLevel::Minimum;

    m_resultsView->eventsModel()->setFilterLevel((EventLevel::Level)level);
}

void EventsWindow::updateResultTitle()
{
    m_resultTitle->setText(m_resultsView->eventsModel()->filterDescription());
}

void EventsWindow::timelineZoomChanged(int value)
{
    m_timelineZoom->setValue(m_timelineZoom->maximum() - (value - m_timelineZoom->minimum()));
}

void EventsWindow::timelineSliderChanged(int value)
{
    m_timeline->setZoomSeconds(m_timelineZoom->maximum() - (value - m_timelineZoom->minimum()));
}

void EventsWindow::timelineZoomRangeChanged(int min, int max)
{
    bool block = m_timelineZoom->blockSignals(true);
    m_timelineZoom->setRange(min, max);
    m_timelineZoom->blockSignals(block);
}

void EventsWindow::showEvent(const QModelIndex &index)
{
    EventData *data = index.data(EventsModel::EventDataPtr).value<EventData*>();
    EventViewWindow::open(data);
}
