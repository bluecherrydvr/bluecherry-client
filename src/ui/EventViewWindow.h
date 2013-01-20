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
