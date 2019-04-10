/*
 * Copyright 2010-2019 Bluecherry, LLC
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

#ifndef DVRCAMERADATA_H
#define DVRCAMERADATA_H

#include <QObject>
#include <QWeakPointer>

class DVRServer;

class DVRCameraData : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DVRCameraData)

public:
    explicit DVRCameraData(int id, DVRServer *server);
    virtual ~DVRCameraData();

    int id() const;
    DVRServer * server() const;

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    bool disabled() const;
    void setDisabled(bool disabled);

    qint8 ptzProtocol() const;
    void setPtzProtocol(qint8 ptzProtocol);

signals:
    void changed();

private:
    const int m_id;
    DVRServer * const m_server;
    QString m_displayName;
    bool m_disabled;
    qint8 m_ptzProtocol;

};

#endif // DVRCAMERADATA_H
