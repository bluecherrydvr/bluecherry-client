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

#ifndef EVENTVIDEODOWNLOADWIDGET_H
#define EVENTVIDEODOWNLOADWIDGET_H

#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;
class EventVideoDownload;

class EventVideoDownloadWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventVideoDownloadWidget(EventVideoDownload *eventVideoDownload, QWidget *parent = 0);
    virtual ~EventVideoDownloadWidget();

protected:
	virtual void changeEvent(QEvent *event);

private slots:
    void closeClicked();
    void updateProgress();

private:
    QWeakPointer<EventVideoDownload> m_eventVideoDownload;
    QPushButton *m_closeButton;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QTimer *m_progressTimer;

	void retranslateUI();
};

#endif // EVENTVIDEODOWNLOADWIDGET_H
