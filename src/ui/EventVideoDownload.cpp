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

#include "EventVideoDownload.h"
#include "video/VideoHttpBuffer.h"
#include <QProgressDialog>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressBar>

EventVideoDownload::EventVideoDownload(QObject *parent)
    : QObject(parent), m_dialog(0), m_futureWatch(0), m_media(0)
{
    connect(&m_progressTimer, SIGNAL(timeout()), SLOT(updateBufferProgress()));
}

EventVideoDownload::~EventVideoDownload()
{
    if (m_media)
        setMediaDownload(0);
}

void EventVideoDownload::setMediaDownload(MediaDownload *media)
{
    Q_ASSERT(!m_dialog || !media);

    if (m_media)
    {
        m_tempFilePath.clear();

        m_media->disconnect(this);
        m_media->deref();
    }

    m_media = media;

    if (m_media)
        m_media->ref();
}

void EventVideoDownload::setFilePath(const QString &path)
{
    m_finalPath = path;
}

void EventVideoDownload::start(QWidget *parentWindow)
{
    Q_ASSERT(m_media && !m_finalPath.isEmpty());

    m_dialog = new QProgressDialog(parentWindow);
    m_dialog->setLabelText(tr("Downloading event video..."));
    m_dialog->setAutoReset(false);
    m_dialog->setAutoClose(false);
    QProgressBar *pb = new QProgressBar(m_dialog);
    pb->setTextVisible(false);
    pb->show();
    m_dialog->setBar(pb);
    m_dialog->show();

    connect(m_dialog, SIGNAL(canceled()), SLOT(cancel()));

    if (m_media->isFinished())
    {
        startCopy();
    }
    else
    {
        connect(m_media, SIGNAL(finished()), SLOT(startCopy()));
        m_progressTimer.start(1000);
    }
}

void EventVideoDownload::updateBufferProgress()
{
    if (!m_dialog || !m_media)
        return;

    m_dialog->setRange(0, (int)m_media->fileSize());
    m_dialog->setValue((int)m_media->downloadedSize());
}

void EventVideoDownload::startCopy()
{
    if (!m_media || m_finalPath.isEmpty())
    {
        qWarning() << "EventVideoDownload::startCopy: Invalid parameters";
        return;
    }

    m_tempFilePath = m_media->bufferFilePath();
    if (m_tempFilePath.isEmpty())
    {
        qWarning() << "EventVideoDownload::startCopy: No buffer file to copy from";
        return;
    }

    if (QFile::exists(m_finalPath))
    {
        if (!QFile::remove(m_finalPath))
        {
            qDebug() << "EventVideoDownload: Failed to replace video file";
            /* TODO: proper error */
            return;
        }
    }

    QFuture<bool> re = QtConcurrent::run(&QFile::copy, m_tempFilePath, m_finalPath);
    m_futureWatch = new QFutureWatcher<bool>(this);
    m_futureWatch->setFuture(re);

    connect(m_futureWatch, SIGNAL(finished()), SLOT(copyFinished()));

    m_dialog->setLabelText(tr("Copying video..."));
    m_dialog->setRange(0, 0);
}

void EventVideoDownload::copyFinished()
{
    Q_ASSERT(m_futureWatch);

    if (m_dialog)
    {
        bool success = m_futureWatch->result();
        if (!success)
        {
            m_dialog->setLabelText(tr("Copy failed!"));
            m_dialog->setRange(0, 100);
            m_dialog->setValue(0);
        }
        else
        {
            m_dialog->setLabelText(tr("Video downloaded successfully"));
            /* This actually is necessary. QProgressDialog is terrible. */
            m_dialog->setRange(0, 101);
            m_dialog->setValue(100);
            m_dialog->setMaximum(100);
        }

        m_dialog->setCancelButtonText(tr("Close"));
    }

    m_futureWatch->deleteLater();
    m_futureWatch = 0;

    setMediaDownload(0);
}

void EventVideoDownload::cancel()
{
    if (m_media)
        setMediaDownload(0);

    m_dialog->close();
    m_dialog->deleteLater();
    m_dialog = 0;

    deleteLater();
}
