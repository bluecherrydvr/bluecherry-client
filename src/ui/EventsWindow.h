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

#ifndef EVENTSWINDOW_H
#define EVENTSWINDOW_H

#include <QBitArray>
#include <QWidget>
#include "model/EventsModel.h"

class DVRServersView;
class EventsUpdater;
class EventsView;
class EventTimelineWidget;
class EventTypesFilter;
class EventTagsView;
class QLabel;
class QBoxLayout;
class QDateEdit;
class QComboBox;
class QSlider;
class QModelIndex;
class QDateTime;
class QTabWidget;
class EventViewWindow;
class QSplitter;
class ModelEventsCursor;

class EventsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EventsWindow(DVRServerRepository *serverRepository, QWidget *parent = 0);
    virtual ~EventsWindow();

protected:
	virtual void changeEvent(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void levelFilterChanged();
    void cursorIndexUpdated();

    void timelineZoomChanged(int value);
    void timelineSliderChanged(int value);

    void eventContextMenu(const QPoint &pos);
    void showServerEvent(const QModelIndex &index);
    void showServerEvent(const EventData &eventData);

    void setFilterTypes(QBitArray types);
    void setFilterDay(const QDateTime &day);
    void setFilterSources(const QMap<DVRServer *, QSet<int> > &sources);

private:
    DVRServerRepository *m_serverRepository;

    /* Filter widgets */
    DVRServersView *m_sourcesView;
    QComboBox *m_levelFilter;
    EventTypesFilter *m_typeFilter;
    EventTagsView *m_tagsView;
    QLabel *m_resultTitle;

    /* Results */
    enum ResultsViewTab
    {
        ListTab = 0,
        TimelineTab
    };

    QTabWidget *m_resultTabs;

    /* Result views */
    EventsView *m_resultsView;
    EventsUpdater *m_eventsUpdater;
    EventTimelineWidget *m_timeline;
    QSlider *m_timelineZoom;
	QWidget *m_timelineContainer;

    /* Playback */
    QSplitter *m_videoSplitter;
    EventViewWindow *m_eventViewer;
    ModelEventsCursor *m_modelEventsCursor;

	QLabel *m_minimumLevelLabel;
	QLabel *m_typeLabel;
	QLabel *m_tagsLabel;
	QLabel *m_dateLabel;
	QComboBox *m_tagInput;
	QLabel *m_zoomLabel;

    void createDateFilter(QBoxLayout *layout);
    QWidget *createLevelFilter();
    QWidget *createTypeFilter();
    QWidget *createTags();
    QWidget *createTagsInput();

    QWidget *createResultsView();
    QWidget *createTimeline();

	void retranslateUI();

};

#endif // EVENTSWINDOW_H
