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

#ifndef EVENTVIEWWINDOW_H
#define EVENTVIEWWINDOW_H

#include <QWidget>
#include "core/EventData.h"
#include "events/EventsCursor.h"

class QSplitter;
class QLabel;
class QListView;
class QTextEdit;
class QComboBox;
class QPushButton;
class EventTagsView;
class EventCommentsWidget;
class EventVideoPlayer;
class ExpandingTextEdit;
class SwitchEventsWidget;

class EventViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EventViewWindow(QWidget *parent = 0);

    static EventViewWindow *open(const EventData &event, EventsCursor *eventsCursor);

    void setEvent(const EventData &event);

    void setEventsCursor(EventsCursor *eventsCursor);
    EventsCursor * eventsCursor() const { return m_eventsCursor.data(); }

public:
    void showEvent();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void commentInputChanged();
    void postComment();
    void setEvent(EventData *event);

private:
    EventData m_event;

    QSplitter *m_splitter;
    QLabel *m_infoLabel;
    EventTagsView *m_tagsView;
    QComboBox *m_tagsInput;
    EventCommentsWidget *m_commentsArea;
    ExpandingTextEdit *m_commentInput;
    QPushButton *m_commentBtn;

    EventVideoPlayer *m_videoPlayer;
    QSharedPointer<EventsCursor> m_eventsCursor;
    SwitchEventsWidget *m_switchEventsWidget;

    QWidget *createInfoArea();
    QWidget *createPlaybackArea();
};

#endif // EVENTVIEWWINDOW_H
