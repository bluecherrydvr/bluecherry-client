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
#include <QWeakPointer>
#include <QHash>
#include <QXmlStreamReader>
#include <QMetaType>

class DVRServer;
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

    static PtzProtocol parseProtocol(const QString &protocol);

    QObject * getQObject() const { return d ? d : 0; }

    DVRServer *server() const { return d ? d->server : 0; }
    int id() const { return d ? d->id: -1; }
    QString displayName() const { return d ? d->displayName : QString(); }
    QByteArray streamUrl() const { return d ? d->streamUrl : QByteArray(); }
    bool isOnline() const { return d && d->isOnline && !d->isDisabled && !d->streamUrl.isEmpty(); }
    bool isDisabled() const { return d && d->isDisabled; }
    bool canStream() const { return d && !d->streamUrl.isEmpty() && isOnline(); }
    LiveStream * liveStream();

    PtzProtocol ptzProtocol() const { return d ? static_cast<PtzProtocol>(d->ptzProtocol) : NoPtz; }
    bool hasPtz() const { return d ? (d->ptzProtocol > 0) : false; }

    RecordingState recordingState() const { return d ? RecordingState(d->recordingState) : NoRecording; }

    bool parseXML(QXmlStreamReader &xml);

    static QList<DVRCamera *> fromMimeData(const QMimeData *mimeData);

signals:
    void onlineChanged(bool isOnline);
    void dataUpdated();
    void recordingStateChanged(int recordingState);

private:
    DVRCameraData *d;

    DVRCamera(int id, DVRServer *server);

    void setOnline(bool on);

};

Q_DECLARE_METATYPE(DVRCamera *)

#endif // DVRCAMERA_H
