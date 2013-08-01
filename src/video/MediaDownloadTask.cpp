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

#include "MediaDownloadTask.h"
#include "core/BluecherryApp.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextDocument>

MediaDownloadTask::MediaDownloadTask(const QUrl &url, const QList<QNetworkCookie> &cookies, unsigned position, unsigned size, QObject *parent)
    : QObject(parent), m_url(url), m_cookies(cookies), m_position(position), m_size(size), m_reply(0)
{
}

void MediaDownloadTask::startDownload()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(!m_reply);

    if (!threadNAM.hasLocalData())
    {
        threadNAM.setLocalData(new QNetworkAccessManager);
        /* XXX certificate validation */
    }

    threadNAM.localData()->cookieJar()->setCookiesFromUrl(m_cookies, m_url);
    if (threadNAM.localData()->cookieJar()->cookiesForUrl(m_url).isEmpty())
        qDebug() << "MediaDownload: No cookies for media URL, likely to fail authentication";

    QNetworkRequest req(m_url);
    qDebug() << Q_FUNC_INFO << m_url << m_position << m_size;

    if (m_position || m_size)
    {
        QByteArray range = "bytes=" + QByteArray::number(m_position) + "-";
        if (m_size)
            range += QByteArray::number(m_position + m_size);
        req.setRawHeader("Range", range);
    }

    m_reply = threadNAM.localData()->get(req);
    m_reply->ignoreSslErrors(); // XXX Do this properly!
    connect(m_reply, SIGNAL(metaDataChanged()), SLOT(metaDataReady()));
    connect(m_reply, SIGNAL(readyRead()), SLOT(read()));
    connect(m_reply, SIGNAL(finished()), SLOT(requestFinished()));
}

MediaDownloadTask::~MediaDownloadTask()
{
    abort();
}

void MediaDownloadTask::abortLater()
{
    /* Mostly threadsafe; caller must know that the instance will not be deleted at this time. */
    if (m_reply)
        m_reply->disconnect(this);
    metaObject()->invokeMethod(this, "abort", Qt::QueuedConnection);
}

void MediaDownloadTask::abort()
{
    if (!m_reply)
        return;

    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = 0;
}

void MediaDownloadTask::metaDataReady()
{
    int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (m_reply->error() != QNetworkReply::NoError || status < 200 || status > 299) {
        if (m_reply->error() != QNetworkReply::NoError)
            emit error(m_reply->errorString());
        else
            emit error(Qt::escape(m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()));
        disconnect(m_reply, 0, this, 0);
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    unsigned size = m_reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    if (size)
        emit requestReady(m_position + size);
}

void MediaDownloadTask::read()
{
    QByteArray data = m_reply->readAll();
    if (data.isEmpty())
        return;

    emit dataRead(data, m_position);
    m_position += data.size();

    /* Very carefully threadsafe: bcApp and the globalRate pointer are
     * const, and 'addSampleValue' is threadsafe and lockfree for the common case. */
    bcApp->globalRate->addSampleValue(data.size());
}

void MediaDownloadTask::requestFinished()
{
    if (m_reply->error() != QNetworkReply::NoError && m_reply->error() != QNetworkReply::UnknownNetworkError)
        emit error(m_reply->errorString());
    else
        emit finished();

    m_reply->deleteLater();
    m_reply = 0;
}