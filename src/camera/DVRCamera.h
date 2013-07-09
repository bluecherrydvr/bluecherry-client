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

#ifndef DVRCAMERA_H
#define DVRCAMERA_H

#include "camera/DVRCameraData.h"
#include "camera/RecordingState.h"
#include <QObject>
#include <QSharedPointer>
#include <QUrl>
#include <QWeakPointer>
#include <QHash>
#include <QXmlStreamReader>
#include <QMetaType>

class CameraPtzControl;
class DVRServer;
class DVRServerRepository;
class LiveStream;
class QMimeData;

class DVRCamera : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DVRCamera)

    friend class DVRServer;

public:
    enum PtzProtocol {
        NoPtz,
        UnknownProtocol,
        PelcoPtz
    };

    explicit DVRCamera(int id, DVRServer *server);
    virtual ~DVRCamera();

    DVRCameraData & data();

    static PtzProtocol parseProtocol(const QString &protocol);

    QSharedPointer<CameraPtzControl> sharedPtzControl();

    void setStreamUrl(const QUrl &streamUrl);
    QUrl streamUrl() const;

    bool isOnline() const;
    LiveStream * liveStream();

    PtzProtocol ptzProtocol() const;
    bool hasPtz() const;

    RecordingState recordingState() const;

    static QList<DVRCamera *> fromMimeData(DVRServerRepository *serverRepository, const QMimeData *mimeData);

signals:
    void onlineChanged(bool isOnline);
    void dataUpdated();
    void recordingStateChanged(int recordingState);

private:
    DVRCameraData m_data;
    QWeakPointer<CameraPtzControl> m_ptzControl;
    QWeakPointer<LiveStream> m_liveStream;
    QUrl m_streamUrl;
    bool m_isOnline;
    qint8 m_recordingState;

    void setOnline(bool on);

};

Q_DECLARE_METATYPE(DVRCamera *)

#endif // DVRCAMERA_H
