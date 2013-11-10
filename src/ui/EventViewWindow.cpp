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

#include "EventViewWindow.h"
#include "model/EventsModel.h"
#include "EventTagsView.h"
#include "model/EventTagsModel.h"
#include "EventCommentsWidget.h"
#include "EventVideoPlayer.h"
#include "ExpandingTextEdit.h"
#include "ui/MainWindow.h"
#include "ui/SwitchEventsWidget.h"
#include "server/DVRServer.h"
#include "core/BluecherryApp.h"
#include "event/EventsCursor.h"
#include <QBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QTextDocument>
#include <QCloseEvent>
#include <QSettings>
#include <QListView>
#include <QTextEdit>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QApplication>
#include <QDebug>

EventViewWindow::EventViewWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    resize(590, 380);

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0,
#ifndef Q_OS_MAC
                               style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
#else
                               0
#endif
                               );

#if 1
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(m_splitter, 100);

    m_splitter->addWidget(createPlaybackArea());
    m_splitter->addWidget(createInfoArea());
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setChildrenCollapsible(false);
#else
    createInfoArea();

    QWidget *topWidget = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->addStretch(1);

    m_switchEventsWidget = new SwitchEventsWidget();
    topLayout->addWidget(m_switchEventsWidget, 1);

    layout->addWidget(topWidget);

    layout->addWidget(topWidget, 1);
    layout->addWidget(createPlaybackArea(), 100);
#endif

    QSettings settings;
    restoreGeometry(settings.value(QLatin1String("ui/eventView/geometry")).toByteArray());
#if 0
    if (!m_splitter->restoreState(settings.value(QLatin1String("ui/eventView/splitState2")).toByteArray()))
        m_splitter->setSizes(QList<int>() << 1000 << 160);
#endif

	retranslateUI();
}

EventViewWindow *EventViewWindow::open(const EventData &event, EventsCursor *eventsCursor)
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
    {
        EventViewWindow *evw;
        if ((evw = qobject_cast<EventViewWindow*>(w)) && evw->m_event == event)
        {
            evw->raise();
            return evw;
        }
    }

    EventViewWindow *evw = new EventViewWindow(bcApp->globalParentWindow());
    evw->setAttribute(Qt::WA_DeleteOnClose);
    evw->setEvent(event);
    evw->setEventsCursor(eventsCursor);
    evw->show();

    return evw;
}

void EventViewWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    if (isWindow())
        settings.setValue(QLatin1String("ui/eventView/geometry"), saveGeometry());
#if 0
    settings.setValue(QLatin1String("ui/eventView/splitState2"), m_splitter->saveState());
    settings.remove(QLatin1String("ui/eventView/splitState")); // As of 2.0.0-beta4
#endif
    QWidget::closeEvent(event);
}

void EventViewWindow::setEvent(const EventData &event)
{
    DVRCamera *camera = m_event.locationCamera();
    if (camera)
        disconnect(camera, 0, this, 0);

    m_event = event;

    DVRCamera *newCamera = m_event.locationCamera();
    if (newCamera)
        connect(newCamera, SIGNAL(destroyed(QObject*)), this, SLOT(close()));

    m_infoLabel->setText(tr("<b>%2</b> (%1)<br><br>%3 (%4)<br>%5")
                         .arg(Qt::escape(event.uiServer()))
                         .arg(Qt::escape(event.uiLocation()))
                         .arg(Qt::escape(event.uiType()))
                         .arg(Qt::escape(event.uiLevel()))
                         .arg(event.localStartDate().toString()));

    if (m_event.hasMedia() && m_event.mediaId() >= 0 && m_event.server())
    {
        QUrl url = m_event.server()->url().resolved(QUrl(QLatin1String("/media/request.php")));
        url.addQueryItem(QLatin1String("id"), QString::number(m_event.mediaId()));
        m_videoPlayer->setVideo(url, &m_event);
    }
    else
        m_videoPlayer->clearVideo();
}

void EventViewWindow::setEvent(EventData *event)
{
    if (event)
        setEvent(*event);
}

void EventViewWindow::setEventsCursor(EventsCursor *eventsCursor)
{
    if (m_eventsCursor.data())
        disconnect(m_eventsCursor.data(), 0, this, 0);

    m_eventsCursor = QSharedPointer<EventsCursor>(eventsCursor);

    if (m_eventsCursor.data())
        connect(m_eventsCursor.data(), SIGNAL(eventSwitched(EventData*)), this, SLOT(setEvent(EventData*)));

	m_switchEventsWidget->setEventsCursor(m_eventsCursor);
}

void EventViewWindow::changeEvent(QEvent *event)
{
	if (event && event->type() == QEvent::LanguageChange)
		retranslateUI();

	QWidget::changeEvent(event);
}

QWidget *EventViewWindow::createInfoArea()
{
    QFrame *container = new QFrame;
    container->setFrameStyle(QFrame::Plain);
    container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    QBoxLayout *layout = new QVBoxLayout(container);
    layout->setMargin(0);

    m_switchEventsWidget = new SwitchEventsWidget();
    layout->addWidget(m_switchEventsWidget);

    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(m_infoLabel);

    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::Sunken | QFrame::HLine);
    layout->addWidget(line);

#if 0 /* Comments and tags are not fully implemented yet */
    QLabel *title = new QLabel(tr("Tags:"));
    title->setStyleSheet(QLatin1String("font-weight:bold"));
    layout->addWidget(title);

    m_tagsView = new EventTagsView;
    m_tagsView->setModel(new EventTagsModel(m_tagsView));
    m_tagsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_tagsView->setFrameStyle(QFrame::NoFrame);
    layout->addWidget(m_tagsView);

    m_tagsInput = new QComboBox;
    m_tagsInput->setEditable(true);
    m_tagsInput->setInsertPolicy(QComboBox::NoInsert);
#if QT_VERSION >= 0x040700
    m_tagsInput->lineEdit()->setPlaceholderText(tr("Add a tag"));
#endif
    layout->addWidget(m_tagsInput);

    line = new QFrame;
    line->setFrameStyle(QFrame::Sunken | QFrame::HLine);
    layout->addWidget(line);

    title = new QLabel(tr("Comments:"));
    title->setStyleSheet(QLatin1String("font-weight:bold"));
    layout->addWidget(title);

    m_commentsArea = new EventCommentsWidget;
    m_commentsArea->setFrameStyle(QFrame::NoFrame);
    m_commentsArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_commentsArea->setMinimumWidth(160);
    m_commentsArea->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_commentsArea);

    m_commentInput = new ExpandingTextEdit;
    m_commentInput->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_commentInput->setTabChangesFocus(true);
    connect(m_commentInput, SIGNAL(textChanged()), SLOT(commentInputChanged()));
    layout->addWidget(m_commentInput);

    m_commentBtn = new QPushButton(tr("Add comment"));
    m_commentBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_commentBtn->hide();
    connect(m_commentBtn, SIGNAL(clicked()), SLOT(postComment()));
    layout->addWidget(m_commentBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    /* For testing purposes */
    m_commentsArea->appendComment(QLatin1String("Author"), QDateTime::currentDateTime(),
                                  QLatin1String("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed in "
                                                "nisi quis augue ultrices sagittis. Fusce porttitor sagittis urna, "
                                                "ac facilisis sem aliquet sit amet."));

    m_commentsArea->appendComment(QLatin1String("Second Author"), QDateTime::currentDateTime(),
                                  QLatin1String("Ok."));
#endif

    layout->addStretch();

    return container;
}

QWidget *EventViewWindow::createPlaybackArea()
{
    m_videoPlayer = new EventVideoPlayer;
	return m_videoPlayer;
}

void EventViewWindow::retranslateUI()
{
	setWindowTitle(tr("Bluecherry - Event Playback"));
}

void EventViewWindow::commentInputChanged()
{
    if (m_commentInput->document()->isEmpty())
    {
        m_commentBtn->setEnabled(false);
        m_commentBtn->hide();
    }
    else
    {
        m_commentBtn->setEnabled(true);
        m_commentBtn->show();
    }
}

void EventViewWindow::postComment()
{
    QString text = m_commentInput->toPlainText();
    if (text.isEmpty())
        return;

    m_commentsArea->appendComment(QLatin1String("Username"), QDateTime::currentDateTime(),
                                  text);

    m_commentInput->clear();
}
