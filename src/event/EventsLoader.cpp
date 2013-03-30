#include "EventsLoader.h"
#include "core/BluecherryApp.h"
#include "server/DVRServer.h"
#include "core/EventData.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

EventsLoader::EventsLoader(DVRServer *server, QObject *parent)
    : QObject(parent), m_server(server), m_limit(-1)
{
}

EventsLoader::~EventsLoader()
{

}

void EventsLoader::setLimit(int limit)
{
    m_limit = limit;
}

void EventsLoader::setStartTime(const QDateTime &startTime)
{
    m_startTime = startTime;
}

void EventsLoader::setEndTime(const QDateTime &endTime)
{
    m_endTime = endTime;
}

void EventsLoader::loadEvents()
{
    if (!m_server || !m_server.data()->api || !m_server.data()->api->isOnline())
    {
        emit eventsLoaded(false, QList<EventData*>());
        deleteLater();
        return;
    }

    QUrl url(QLatin1String("/events/"));
    url.addQueryItem(QLatin1String("limit"), QString::number(m_limit));
    if (!m_startTime.isNull())
        url.addQueryItem(QLatin1String("startDate"), QString::number(m_startTime.toTime_t()));
    if (!m_endTime.isNull())
        url.addQueryItem(QLatin1String("endDate"), QString::number(m_endTime.toTime_t()));

    QNetworkRequest req = m_server.data()->api->buildRequest(url);
    QNetworkReply *reply = m_server.data()->api->sendRequest(req);
    connect(reply, SIGNAL(finished()), SLOT(serverRequestFinished()));
}

void EventsLoader::serverRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);

    reply->deleteLater();

    if (!m_server)
    {
        deleteLater();
        return; // ignore data from removed servers
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Event request error:" << reply->errorString();
        /* TODO: Handle errors properly */
        emit eventsLoaded(false, QList<EventData*>());
        deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300)
    {
        qWarning() << "Event request error: HTTP code" << statusCode;
        emit eventsLoaded(false, QList<EventData*>());
        deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    // qDebug() << "EventsLoader: Received reply from server: " << data;

    QFuture<QList<EventData*> > future = QtConcurrent::run(&EventData::parseEvents, m_server.data(), data);

    QFutureWatcher<QList<EventData*> > *qfw = new QFutureWatcher<QList<EventData*> >(this);
    connect(qfw, SIGNAL(finished()), SLOT(eventParseFinished()));
    qfw->setFuture(future);
}

void EventsLoader::eventParseFinished()
{
    Q_ASSERT(sender() && sender()->inherits("QFutureWatcherBase"));
    QFutureWatcher<QList<EventData*> > *qfw = static_cast<QFutureWatcher<QList<EventData*> >*>(sender());
    qfw->deleteLater();

    if (!m_server)
    {
        emit eventsLoaded(false, QList<EventData*>());
        deleteLater();
        return; // ignore data from removed servers
    }

    QList<EventData*> events = qfw->result();
    qDebug() << "EventsLoader: Parsed event data into" << events.size() << "events";

    emit eventsLoaded(true, events);
    deleteLater();
}
