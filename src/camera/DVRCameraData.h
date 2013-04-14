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

#ifndef DVRCAMERADATA_H
#define DVRCAMERADATA_H

#include <QObject>
#include <QWeakPointer>

class DVRServer;
class LiveStream;

class DVRCameraData : public QObject
{
    Q_OBJECT

    friend class DVRCamera;
    friend class DVRServer;

public:
    DVRServer * const server;
    const int id;
    QString displayName;
    QByteArray streamUrl;
    QWeakPointer<LiveStream> liveStream;
    bool isLoaded, isOnline, isDisabled;
    qint8 ptzProtocol;
    qint8 recordingState;

    DVRCameraData(int id, DVRServer *server);
    virtual ~DVRCameraData();

    void loadSavedSettings();
    void doDataUpdated();

public slots:
    void setRecordingState(int recordingState);

signals:
    void dataUpdated();

    void recordingStateChanged(int recordingState);

};

#endif // DVRCAMERADATA_H
