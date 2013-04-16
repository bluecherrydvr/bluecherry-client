#include "bluecherry-config.h"
#include "core/EventData.h"
#include "event/EventParser.h"
#include <QtTest/QtTest>
#include <QDebug>

const char *jpegFormatName = "jpeg"; // hack

class EventParserTestCase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testV2DemoFileSize();
    void testV2DemoSingleFullItem();

    void testUtcDateTimeEventWithoutDuration();
    void testUtcDateTimeEventWithDuration();
    void testNonUtcDateTimeEventWithoutDuration();
    void testNonUtcDateTimeEventWithDuration();

private:
    QList<EventData *> parseFile(const QString &fileName);
    EventData * parseSingleEventFile(const QString &fileName);
    QDateTime parseUTCDateTime(const QString &dateTimeString);
    QDateTime parseUTCDateTimeWithHoursOffset(const QString &dateTimeString, int offsetInHours);

};

QList<EventData *> EventParserTestCase::parseFile(const QString &fileName)
{
    QFile file(QString::fromLatin1("%1/event/%2").arg(QString::fromLatin1(TEST_DATA_DIR)).arg(fileName));
    if (!file.open(QIODevice::ReadOnly))
        return QList<EventData *>();

    QList<EventData *> result = EventParser::parseEvents(0, file.readAll());
    file.close();

    return result;
}

EventData * EventParserTestCase::parseSingleEventFile(const QString &fileName)
{
    QList<EventData *> events = parseFile(fileName);
    return events.at(0);
}

QDateTime EventParserTestCase::parseUTCDateTime(const QString &dateTimeString)
{
    QDateTime result = QDateTime::fromString(dateTimeString, Qt::ISODate);
    result.setTimeSpec(Qt::UTC);

    return result;
}

QDateTime EventParserTestCase::parseUTCDateTimeWithHoursOffset(const QString &dateTimeString, int offsetInMinutes)
{
    QDateTime result = QDateTime::fromString(dateTimeString, Qt::ISODate);
    result.setTimeSpec(Qt::UTC);
    result.setUtcOffset(offsetInMinutes * 60);

    return result;
}

void EventParserTestCase::testV2DemoFileSize()
{
    QList<EventData *> events = parseFile(QLatin1String("v2demo.xml"));
    QCOMPARE(events.size(), 50);
}

void EventParserTestCase::testV2DemoSingleFullItem()
{
    EventData *event = parseSingleEventFile(QLatin1String("v2demo-single-full-item.xml"));
    QVERIFY(!event->server());
    QCOMPARE(event->eventId(), (long long)511478);
    QCOMPARE(event->utcStartDate(), parseUTCDateTime(QLatin1String("2013/04/16 21:10:03.000")));
    QCOMPARE(event->utcStartDate().timeSpec(), Qt::UTC);
    QCOMPARE(event->utcEndDate(), parseUTCDateTime(QLatin1String("2013/04/16 21:10:03.000")));
    QCOMPARE(event->utcEndDate().timeSpec(), Qt::UTC);
    QCOMPARE(event->serverStartDate(), parseUTCDateTimeWithHoursOffset(QLatin1String("2013/04/16 16:10:03.000"), event->dateTzOffsetMins()));
    QCOMPARE(event->serverEndDate(), parseUTCDateTimeWithHoursOffset(QLatin1String("2013/04/16 16:10:03.000"), event->dateTzOffsetMins()));
    QCOMPARE(event->durationInSeconds(), -1);
    QVERIFY(!event->hasDuration());
    QVERIFY(event->inProgress());
    QCOMPARE(event->locationId(), 5);
    QCOMPARE(event->level().level, EventLevel::Info);
    QCOMPARE(event->type().type, EventType::CameraContinuous);
    QCOMPARE(event->mediaId(), (long long)505052);
    QCOMPARE(event->dateTzOffsetMins(), (short)-300);
    QVERIFY(!event->isSystem());
    QVERIFY(event->isCamera());
    QVERIFY(event->hasMedia());

}

void EventParserTestCase::testUtcDateTimeEventWithoutDuration()
{
    EventData *event = parseSingleEventFile(QLatin1String("utc-date-time-event-without-duration.xml"));
    QVERIFY(!event->server());
    QCOMPARE(event->eventId(), (long long)1);
    QCOMPARE(event->utcStartDate(), parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000")));
    QCOMPARE(event->utcStartDate().timeSpec(), Qt::UTC);
    QCOMPARE(event->utcEndDate(), parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000")));
    QCOMPARE(event->utcEndDate().timeSpec(), Qt::UTC);
    QCOMPARE(event->serverStartDate(), parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), event->dateTzOffsetMins()));
    QCOMPARE(event->serverEndDate(), parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000.000"), event->dateTzOffsetMins()));
    QCOMPARE(event->durationInSeconds(), -1);
    QVERIFY(!event->hasDuration());
    QCOMPARE(event->dateTzOffsetMins(), (short)0);
}

void EventParserTestCase::testUtcDateTimeEventWithDuration()
{
}

void EventParserTestCase::testNonUtcDateTimeEventWithoutDuration()
{
}

void EventParserTestCase::testNonUtcDateTimeEventWithDuration()
{
}


QTEST_MAIN(EventParserTestCase)

#include "EventParserTestCase.moc"
