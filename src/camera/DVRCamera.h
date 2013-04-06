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

    friend class DVRServer;

public:
    enum PtzProtocol {
        NoPtz,
        UnknownProtocol,
        PelcoPtz
    };

    static PtzProtocol parseProtocol(const QString &protocol);

    DVRCamera() { }
    DVRCamera(const DVRCamera &o);

    DVRCamera &operator=(const DVRCamera &o);

    bool operator==(const DVRCamera &o) const
    {
        return (d.data() == o.d.data());
    }

    bool isValid() const { return d; }

    QObject * getQObject() const { return d ? d.data() : 0; }

    DVRServer *server() const { return d ? d->server : 0; }
    int uniqueId() const { return d ? d->uniqueID : -1; }
    QString displayName() const { return d ? d->displayName : QString(); }
    QByteArray streamUrl() const { return d ? d->streamUrl : QByteArray(); }
    bool isOnline() const { return d && d->isOnline && !d->isDisabled && !d->streamUrl.isEmpty(); }
    bool isDisabled() const { return d && d->isDisabled; }
    bool canStream() const { return d && !d->streamUrl.isEmpty() && isOnline(); }
    QSharedPointer<LiveStream> liveStream();

    PtzProtocol ptzProtocol() const { return d ? static_cast<PtzProtocol>(d->ptzProtocol) : NoPtz; }
    bool hasPtz() const { return d ? (d->ptzProtocol > 0) : false; }

    RecordingState recordingState() const { return d ? RecordingState(d->recordingState) : NoRecording; }

    bool parseXML(QXmlStreamReader &xml);

    static QList<DVRCamera> fromMimeData(const QMimeData *mimeData);
    static DVRCamera fromQObject(QObject *o);

signals:
    void onlineChanged(bool isOnline);

private:
    QExplicitlySharedDataPointer<DVRCameraData> d;

    DVRCamera(DVRCameraData *dt);

    void setOnline(bool on);

    void connectData();
    void disconnectData();

};

Q_DECLARE_METATYPE(DVRCamera)

uint qHash(const DVRCamera &camera);

QDataStream &operator<<(QDataStream &s, const DVRCamera &camera);

inline DVRCamera DVRCamera::fromQObject(QObject *o)
{
    return DVRCamera(qobject_cast<DVRCameraData*>(o));
}

#endif // DVRCAMERA_H
