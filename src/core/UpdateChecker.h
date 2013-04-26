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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include "core/Version.h"
#include <QObject>

class QNetworkAccessManager;
class QTimer;

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QNetworkAccessManager *networkAccessManager, QObject *parent = 0);
    virtual ~UpdateChecker();

    void start(int interval);
    void stop();

signals:
    void newVersionAvailable(const Version &newVersion);

private slots:
    void performVersionCheck();
    void versionInfoReceived();

private:
    QNetworkAccessManager *m_networkAccessManager;
    QTimer *m_timer;
    bool m_doingUpdateCheck;

};

#endif // UPDATECHECKER_H
