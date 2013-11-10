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

    void testSingleItems();
    void testSingleItems_data();
    void testMedia();
    void testMedia_data();
    void testCategoryLevel();
    void testCategoryLevel_data();

private:
    QList<EventData *> parseFile(const QString &fileName);
    EventData * parseSingleEventFile(const QString &fileName);
    QDateTime parseUTCDateTime(const QString &dateTimeString);
    QDateTime parseUTCDateTimeWithHoursOffset(const QString &dateTimeString, int offsetInHours);

};

Q_DECLARE_METATYPE(Qt::TimeSpec);
Q_DECLARE_METATYPE(EventLevel::Level);
Q_DECLARE_METATYPE(EventType::Type);

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

void EventParserTestCase::testSingleItems()
{
    QFETCH(QString, fileName);
    QFETCH(long long, eventId);
    QFETCH(QDateTime, localStartDate);
    QFETCH(Qt::TimeSpec, localStartDateTimeSpec);
    QFETCH(QDateTime, utcEndDate);
    QFETCH(Qt::TimeSpec, utcEndDateTimeSpec);
    QFETCH(QDateTime, serverStartDate);
    QFETCH(Qt::TimeSpec, serverStartDateTimeSpec);
    QFETCH(QDateTime, serverEndDate);
    QFETCH(Qt::TimeSpec, serverEndDateTimeSpec);
    QFETCH(int, durationInSeconds);
    QFETCH(bool, hasDuration);
    QFETCH(bool, inProgress);
    QFETCH(int, locationId);
    QFETCH(EventLevel::Level, level);
    QFETCH(EventType::Type, type);
    QFETCH(long long, mediaId);
    QFETCH(short, dateTzOffsetMins);
    QFETCH(bool, isSystem);
    QFETCH(bool, isCamera);
    QFETCH(bool, hasMedia);

    EventData *event = parseSingleEventFile(fileName);
    QVERIFY(!event->server());
    QCOMPARE(event->eventId(), eventId);
    QCOMPARE(event->localStartDate(), localStartDate);
    QCOMPARE(event->localStartDate().timeSpec(), localStartDateTimeSpec);
    QCOMPARE(event->localEndDate(), utcEndDate);
    QCOMPARE(event->localEndDate().timeSpec(), utcEndDateTimeSpec);
    QCOMPARE(event->serverStartDate(), serverStartDate);
    QCOMPARE(event->serverStartDate().timeSpec(), serverStartDateTimeSpec);
    QCOMPARE(event->serverEndDate(), serverEndDate);
    QCOMPARE(event->serverEndDate().timeSpec(), serverEndDateTimeSpec);
    QCOMPARE(event->durationInSeconds(), durationInSeconds);
    QCOMPARE(event->hasDuration(), hasDuration);
    QCOMPARE(event->inProgress(), inProgress);
    QCOMPARE(event->locationId(), locationId);
    QCOMPARE(event->level().level, level);
    QCOMPARE(event->type().type, type);
    QCOMPARE(event->mediaId(), mediaId);
    QCOMPARE(event->serverDateTzOffsetMins(), dateTzOffsetMins);
    QCOMPARE(event->isSystem(), isSystem);
    QCOMPARE(event->isCamera(), isCamera);
    QCOMPARE(event->hasMedia(), hasMedia);
}

void EventParserTestCase::testSingleItems_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<long long>("eventId");
    QTest::addColumn<QDateTime>("localStartDate");
    QTest::addColumn<Qt::TimeSpec>("localStartDateTimeSpec");
    QTest::addColumn<QDateTime>("utcEndDate");
    QTest::addColumn<Qt::TimeSpec>("utcEndDateTimeSpec");
    QTest::addColumn<QDateTime>("serverStartDate");
    QTest::addColumn<Qt::TimeSpec>("serverStartDateTimeSpec");
    QTest::addColumn<QDateTime>("serverEndDate");
    QTest::addColumn<Qt::TimeSpec>("serverEndDateTimeSpec");
    QTest::addColumn<int>("durationInSeconds");
    QTest::addColumn<bool>("hasDuration");
    QTest::addColumn<bool>("inProgress");
    QTest::addColumn<int>("locationId");
    QTest::addColumn<EventLevel::Level>("level");
    QTest::addColumn<EventType::Type>("type");
    QTest::addColumn<long long>("mediaId");
    QTest::addColumn<short>("dateTzOffsetMins");
    QTest::addColumn<bool>("isSystem");
    QTest::addColumn<bool>("isCamera");
    QTest::addColumn<bool>("hasMedia");

    QTest::newRow("Full item")
        << QString::fromLatin1("v2demo-single-full-item.xml")
        << (long long)511478
        << parseUTCDateTime(QLatin1String("2013/04/16 21:10:03.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/04/16 21:10:03.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/04/16 16:10:03.000"), -300)
        << Qt::OffsetFromUTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/04/16 16:10:03.000"), -300)
        << Qt::OffsetFromUTC
        << -1
        << false
        << true
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)505052
        << (short)-300
        << false
        << true
        << true;

    QTest::newRow("Utc Date Time Without Duration")
        << QString::fromLatin1("utc-date-time-event-without-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), 0)
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), 0)
        << Qt::UTC
        << -1
        << false
        << true
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)0
        << false
        << true
        << true;

    QTest::newRow("Utc Date Time With Zero Duration")
        << QString::fromLatin1("utc-date-time-event-with-zero-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), 0)
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), 0)
        << Qt::UTC
        << 0
        << false
        << false
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)0
        << false
        << true
        << true;

    QTest::newRow("Utc Date Time With Non Zero Duration")
        << QString::fromLatin1("utc-date-time-event-with-non-zero-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:30.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), 0)
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:30.000"), 0)
        << Qt::UTC
        << 30
        << true
        << false
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)0
        << false
        << true
        << true;

    QTest::newRow("Non Utc Date Time Without Duration")
        << QString::fromLatin1("non-utc-date-time-event-without-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:00.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), -300)
        << Qt::OffsetFromUTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), -300)
        << Qt::OffsetFromUTC
        << -1
        << false
        << true
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)-300
        << false
        << true
        << true;

    QTest::newRow("Non Utc Date Time With Zero Duration")
        << QString::fromLatin1("non-utc-date-time-event-with-zero-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:00.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), -300)
        << Qt::OffsetFromUTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), -300)
        << Qt::OffsetFromUTC
        << 0
        << false
        << false
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)-300
        << false
        << true
        << true;

    QTest::newRow("Non Utc Date Time With Non Zero Duration")
        << QString::fromLatin1("non-utc-date-time-event-with-non-zero-duration.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 06:00:30.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:00.000"), -300)
        << Qt::OffsetFromUTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 01:00:30.000"), -300)
        << Qt::OffsetFromUTC
        << 30
        << true
        << false
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)1
        << (short)-300
        << false
        << true
        << true;

    QTest::newRow("Timezone Changed")
        << QString::fromLatin1("timezone-changed.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/01/01 01:00:30.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 02:00:00.000"), 60)
        << Qt::OffsetFromUTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/01/01 03:00:30.000"), 120)
        << Qt::OffsetFromUTC
        << 30
        << true
        << false
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)123
        << (short)60
        << false
        << true
        << true;

    QTest::newRow("Ended before Start")
        << QString::fromLatin1("ended-before-start.xml")
        << (long long)1
        << parseUTCDateTime(QLatin1String("2013/03/01 02:00:00.000"))
        << Qt::UTC
        << parseUTCDateTime(QLatin1String("2013/03/01 02:00:00.000"))
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/03/01 02:00:00.000"), 0)
        << Qt::UTC
        << parseUTCDateTimeWithHoursOffset(QLatin1String("2013/03/01 02:00:00.000"), 0)
        << Qt::UTC
        << -1
        << false
        << true
        << 5
        << EventLevel::Info
        << EventType::CameraContinuous
        << (long long)123
        << (short)0
        << false
        << true
        << true;
}

void EventParserTestCase::testMedia()
{
    QFETCH(QString, fileName);
    QFETCH(long long, mediaId);
    QFETCH(bool, hasMedia);

    EventData *event = parseSingleEventFile(fileName);
    QVERIFY(!event->server());
    QCOMPARE(event->mediaId(), mediaId);
    QCOMPARE(event->hasMedia(), hasMedia);
}

void EventParserTestCase::testMedia_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<long long>("mediaId");
    QTest::addColumn<bool>("hasMedia");

    QTest::newRow("With media")
        << QString::fromLatin1("with-media.xml")
        << (long long)123
        << true;

    QTest::newRow("Without media invalid attribute")
        << QString::fromLatin1("without-media-invalid-attribute.xml")
        << (long long)-1
        << false;

    QTest::newRow("Without media no attribute")
        << QString::fromLatin1("without-media-no-attribute.xml")
        << (long long)-1
        << false;

    QTest::newRow("Without media verbatim attribute")
        << QString::fromLatin1("without-media-verbatim-value.xml")
        << (long long)-1
        << false;
}

void EventParserTestCase::testCategoryLevel()
{
    QFETCH(QString, fileName);
    QFETCH(EventLevel::Level, level);
    QFETCH(EventType::Type, type);

    EventData *event = parseSingleEventFile(fileName);
    QVERIFY(!event->server());
    QCOMPARE(event->level().level, level);
    QCOMPARE(event->type().type, type);
}

void EventParserTestCase::testCategoryLevel_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<EventLevel::Level>("level");
    QTest::addColumn<EventType::Type>("type");

    QTest::newRow("Category/level invalid format")
        << QString::fromLatin1("category-level-invalid-format.xml")
        << EventLevel::Info
        << EventType::UnknownType;

    QTest::newRow("Category/level invalid level")
        << QString::fromLatin1("category-level-invalid-level.xml")
        << EventLevel::Info
        << EventType::CameraContinuous;

    QTest::newRow("Category/level invalid location id")
        << QString::fromLatin1("category-level-invalid-location-id.xml")
        << EventLevel::Info
        << EventType::CameraContinuous;

    QTest::newRow("Category/level invalid scheme")
        << QString::fromLatin1("category-level-invalid-scheme.xml")
        << EventLevel::Info
        << EventType::UnknownType;

    QTest::newRow("Category/level no scheme")
        << QString::fromLatin1("category-level-invalid-format.xml")
        << EventLevel::Info
        << EventType::UnknownType;

    QTest::newRow("Category/level invalid category")
        << QString::fromLatin1("category-level-no-scheme.xml")
        << EventLevel::Info
        << EventType::UnknownType;

    QTest::newRow("Category/level level alarm")
        << QString::fromLatin1("category-level-alarm.xml")
        << EventLevel::Alarm
        << EventType::CameraContinuous;

    QTest::newRow("Category/level level critical")
        << QString::fromLatin1("category-level-critical.xml")
        << EventLevel::Critical
        << EventType::CameraContinuous;

    QTest::newRow("Category/level level info")
        << QString::fromLatin1("category-level-info.xml")
        << EventLevel::Info
        << EventType::CameraContinuous;

    QTest::newRow("Category/level level warning")
        << QString::fromLatin1("category-level-warning.xml")
        << EventLevel::Warning
        << EventType::CameraContinuous;

    QTest::newRow("Category/level category motion")
        << QString::fromLatin1("category-level-camera-motion.xml")
        << EventLevel::Info
        << EventType::CameraMotion;

    QTest::newRow("Category/level category camera continuous")
        << QString::fromLatin1("category-level-camera-continuous.xml")
        << EventLevel::Info
        << EventType::CameraContinuous;

    QTest::newRow("Category/level category camera not found")
        << QString::fromLatin1("category-level-camera-not-found.xml")
        << EventLevel::Info
        << EventType::CameraNotFound;

    QTest::newRow("Category/level category camera video lost")
        << QString::fromLatin1("category-level-camera-video-lost.xml")
        << EventLevel::Info
        << EventType::CameraVideoLost;

    QTest::newRow("Category/level category camera audio lost")
        << QString::fromLatin1("category-level-camera-audio-lost.xml")
        << EventLevel::Info
        << EventType::CameraAudioLost;

    QTest::newRow("Category/level category system disk space")
        << QString::fromLatin1("category-level-system-disk-space.xml")
        << EventLevel::Info
        << EventType::SystemDiskSpace;

    QTest::newRow("Category/level category system crash")
        << QString::fromLatin1("category-level-system-crash.xml")
        << EventLevel::Info
        << EventType::SystemCrash;

    QTest::newRow("Category/level category system boot")
        << QString::fromLatin1("category-level-system-boot.xml")
        << EventLevel::Info
        << EventType::SystemBoot;

    QTest::newRow("Category/level category system shutdown")
        << QString::fromLatin1("category-level-system-shutdown.xml")
        << EventLevel::Info
        << EventType::SystemShutdown;

    QTest::newRow("Category/level category system reboot")
        << QString::fromLatin1("category-level-system-reboot.xml")
        << EventLevel::Info
        << EventType::SystemReboot;

    QTest::newRow("Category/level category system power outage")
        << QString::fromLatin1("category-level-system-power-outage.xml")
        << EventLevel::Info
        << EventType::SystemPowerOutage;
}

QTEST_MAIN(EventParserTestCase)

#include "EventParserTestCase.moc"
