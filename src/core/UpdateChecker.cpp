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

#include "UpdateChecker.h"
#include <QApplication>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QXmlStreamReader>

UpdateChecker::UpdateChecker(QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent), m_networkAccessManager(networkAccessManager), m_doingUpdateCheck(false)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), SLOT(performVersionCheck()));
}

UpdateChecker::~UpdateChecker()
{

}

void UpdateChecker::start(int interval)
{
    m_timer->setInterval(interval);
    m_timer->start();

    performVersionCheck();
}

void UpdateChecker::stop()
{
    m_timer->stop();
}

void UpdateChecker::performVersionCheck()
{
    qDebug() << Q_FUNC_INFO << "Performing update check";

    // we don't want multiple "upgrade?" dialogs
    if (m_doingUpdateCheck) {
        qDebug() << Q_FUNC_INFO << "Check in progress; ignoring";
        return;
    }

    /* perform update check */
    QNetworkRequest versionCheck(QString::fromLatin1("http://keycheck.bluecherrydvr.com/client_version/?v=") + QApplication::applicationVersion());
    QNetworkReply *versionReply = m_networkAccessManager->get(versionCheck);

    /* lack of error handling here is deliberate; we really don't care if it
     * fails. a stupid error dialog popping up because the user didn't have
     * internet connectivity (for instance) is worse than being a bit out of date.
     */
    connect(versionReply, SIGNAL(finished()), SLOT(versionInfoReceived()));
}

void UpdateChecker::versionInfoReceived()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << Q_FUNC_INFO << "Error from server: " << reply->errorString();
        return;
    }

    QByteArray versionXmlData = reply->readAll();
    QXmlStreamReader xmlStream(versionXmlData);

    if (xmlStream.hasError() || !xmlStream.readNextStartElement() || xmlStream.name() != QLatin1String("response"))
    {
        qDebug() << Q_FUNC_INFO << "Malformed version info from server";
        qDebug() << versionXmlData;
        qDebug() << xmlStream.errorString();
        return;
    }

    forever {
        if (xmlStream.tokenType() == QXmlStreamReader::StartElement &&
            xmlStream.name() == QLatin1String("version"))
            break;

        if (!xmlStream.readNext()) {
            qWarning() << "Didn't find valid version info";
            return;
        }
    }

    QString latest = xmlStream.attributes().value(QLatin1String("latest")).toString();
    Version currentVersion = Version::fromString(QApplication::applicationVersion());
    Version latestVersion = Version::fromString(latest);

    qDebug() << Q_FUNC_INFO << "Latest version info: " << latest;

    if (currentVersion.isValid() && latestVersion.isValid() && (latestVersion > currentVersion)) {
        qDebug() << Q_FUNC_INFO << "Version differs:";
        qDebug() << Q_FUNC_INFO << "Current: " << QApplication::applicationVersion();
        qDebug() << Q_FUNC_INFO << "New: " << latest;

        /* slight hack to work around timers triggering while a dialog is
         * potentially open: tell the update requester that we're busy so it
         * doesn't go and ask for the latest version again while the dialog is
         * already open.
         *
         * this is only a problem if people don't close the dialog once a day,
         * (and don't upgrade) but we might as well.
         */
        m_doingUpdateCheck = true;
        emit newVersionAvailable(latestVersion);
        m_doingUpdateCheck = false;
    }
}
