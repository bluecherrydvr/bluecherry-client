#include "utils/DateTimeRange.h"
#include <QtTest/QtTest>
#include <QDebug>

const char *jpegFormatName = "jpeg"; // hack

class DateTimeRangeTestCase : public QObject
{
    Q_OBJECT

    QDateTime now;
    QDateTime nowPlus99;
    DateTimeRange nullDateTimeRange;
    DateTimeRange pointDateTimeRange;
    DateTimeRange length99DateTimeRange;
    DateTimeRange length99ShiftedBy20DateTimeRange;
    DateTimeRange allDateTimeRange;

private Q_SLOTS:
    void initTestCase();

    void testIsNullForNull();
    void testIsNullForNotNull();

    void testStartForNull();
    void testEndForNull();
    void testStartForNotNull();
    void testEndForNotNull();

    void testLengthInSecondsForNull();
    void testLengthInSecondsForPoint();
    void testLengthInSecondsForLength99();

    void testContainsForNull();
    void testContainsForPoint();
    void testContainsForLength99();

    void testNullBoundedByNull();
    void testNullBoundedByNotNull();
    void testPointBoundedByNull();
    void testPointBoundedByContainingNotNull();
    void testPointBoundedByNotContainingNotNull();
    void testLengthBoundedByShiftedLength();
    void testShiftedLengthBoundedByLength();

    void testExtendNullWithNull();
    void testExtendNullWithNotNull();
    void testExtendPointWithNull();
    void testExtendPointWithNotNull();
    void testExtendLengthWithNull();
    void testExtendLengthWithShiftedLength();
    void testExtendShiftedLengthWithLength();

    void testWithLengthInSecondsWithNullAnd0();
    void testWithLengthInSecondsWithNullAnd20();
    void testWithLengthInSecondsWithNotNullAnd0();
    void testWithLengthInSecondsWithNotNullAnd20();

    void testMoveNullIntoNull();
    void testMoveNullIntoNotNull();
    void testMoveNotNullIntoNull();
    void testMovePointIntoContainingRange();
    void testMovePointIntoNotContainingRange();
    void testMoveMatchingLengthIntoContainingRange();
    void testMoveMatchingLengthIntoNotContainingRange();
    void testMoveLongerIntoContainedRange();
    void testMoveLongerIntoRightOverlapingRange();
    void testMoveLongerIntoLeftOverlapingRange();
    void testMoveLongerIntoNonOverlapingRange();

    void testMoveStartNullIntoNull();
    void testMoveStartNullIntoNotNull();
    void testMoveStartNotNullIntoNull();
    void testMoveStartPoint();
    void testMoveStartLength();

};

void DateTimeRangeTestCase::initTestCase()
{
    now = QDateTime::currentDateTime();
    nowPlus99 = now.addSecs(99);

    pointDateTimeRange = DateTimeRange(now, now);
    length99DateTimeRange = DateTimeRange(now, nowPlus99);
    length99ShiftedBy20DateTimeRange = DateTimeRange(now.addSecs(20), nowPlus99.addSecs(20));
    allDateTimeRange = DateTimeRange(length99DateTimeRange.start(), length99ShiftedBy20DateTimeRange.end());
}

void DateTimeRangeTestCase::testIsNullForNull()
{
    QVERIFY(nullDateTimeRange.isNull());
}

void DateTimeRangeTestCase::testIsNullForNotNull()
{
    QVERIFY(!pointDateTimeRange.isNull());
    QVERIFY(!length99DateTimeRange.isNull());
}

void DateTimeRangeTestCase::testStartForNull()
{
    QVERIFY(nullDateTimeRange.start().isNull());
}

void DateTimeRangeTestCase::testEndForNull()
{
    QVERIFY(nullDateTimeRange.end().isNull());
}

void DateTimeRangeTestCase::testStartForNotNull()
{
    QCOMPARE(pointDateTimeRange.start(), now);
    QCOMPARE(length99DateTimeRange.start(), now);
}

void DateTimeRangeTestCase::testEndForNotNull()
{
    QCOMPARE(pointDateTimeRange.end(), now);
    QCOMPARE(length99DateTimeRange.end(), nowPlus99);
}

void DateTimeRangeTestCase::testLengthInSecondsForNull()
{
    QCOMPARE(nullDateTimeRange.lengthInSeconds(), -1);
}

void DateTimeRangeTestCase::testLengthInSecondsForPoint()
{
    QCOMPARE(pointDateTimeRange.lengthInSeconds(), 0);
}

void DateTimeRangeTestCase::testLengthInSecondsForLength99()
{
    QCOMPARE(length99DateTimeRange.lengthInSeconds(), 99);
}

void DateTimeRangeTestCase::testContainsForNull()
{
    QVERIFY(!nullDateTimeRange.contains(now.addSecs(-1)));
    QVERIFY(!nullDateTimeRange.contains(now));
    QVERIFY(!nullDateTimeRange.contains(now.addSecs(1)));
    QVERIFY(!nullDateTimeRange.contains(nowPlus99.addSecs(-1)));
    QVERIFY(!nullDateTimeRange.contains(nowPlus99));
    QVERIFY(!nullDateTimeRange.contains(nowPlus99.addSecs(1)));
}

void DateTimeRangeTestCase::testContainsForPoint()
{
    QVERIFY(!pointDateTimeRange.contains(now.addSecs(-1)));
    QVERIFY(pointDateTimeRange.contains(now));
    QVERIFY(!pointDateTimeRange.contains(now.addSecs(1)));
    QVERIFY(!pointDateTimeRange.contains(nowPlus99.addSecs(-1)));
    QVERIFY(!pointDateTimeRange.contains(nowPlus99));
    QVERIFY(!pointDateTimeRange.contains(nowPlus99.addSecs(1)));
}

void DateTimeRangeTestCase::testContainsForLength99()
{
    QVERIFY(!length99DateTimeRange.contains(now.addSecs(-1)));
    QVERIFY(length99DateTimeRange.contains(now));
    QVERIFY(length99DateTimeRange.contains(now.addSecs(1)));
    QVERIFY(length99DateTimeRange.contains(nowPlus99.addSecs(-1)));
    QVERIFY(length99DateTimeRange.contains(nowPlus99));
    QVERIFY(!length99DateTimeRange.contains(nowPlus99.addSecs(1)));
}

void DateTimeRangeTestCase::testNullBoundedByNull()
{
    QVERIFY(nullDateTimeRange.boundedBy(DateTimeRange()).isNull());
}

void DateTimeRangeTestCase::testNullBoundedByNotNull()
{
    QVERIFY(nullDateTimeRange.boundedBy(length99ShiftedBy20DateTimeRange).isNull());
}

void DateTimeRangeTestCase::testPointBoundedByNull()
{
    QVERIFY(pointDateTimeRange.boundedBy(nullDateTimeRange).isNull());
}

void DateTimeRangeTestCase::testPointBoundedByContainingNotNull()
{
    QCOMPARE(pointDateTimeRange.boundedBy(length99DateTimeRange), pointDateTimeRange);
}

void DateTimeRangeTestCase::testPointBoundedByNotContainingNotNull()
{
    QVERIFY(pointDateTimeRange.boundedBy(length99ShiftedBy20DateTimeRange).isNull());
}

void DateTimeRangeTestCase::testLengthBoundedByShiftedLength()
{
    DateTimeRange bounded = length99DateTimeRange.boundedBy(length99ShiftedBy20DateTimeRange);

    QVERIFY(!bounded.isNull());
    QCOMPARE(bounded.start(), length99ShiftedBy20DateTimeRange.start());
    QCOMPARE(bounded.end(), length99DateTimeRange.end());
    QCOMPARE(bounded.lengthInSeconds(), 99 - 20);
}

void DateTimeRangeTestCase::testShiftedLengthBoundedByLength()
{
    DateTimeRange bounded = length99ShiftedBy20DateTimeRange.boundedBy(length99DateTimeRange);

    QVERIFY(!bounded.isNull());
    QCOMPARE(bounded.start(), length99ShiftedBy20DateTimeRange.start());
    QCOMPARE(bounded.end(), length99DateTimeRange.end());
    QCOMPARE(bounded.lengthInSeconds(), 99 - 20);
}

void DateTimeRangeTestCase::testExtendNullWithNull()
{
    QCOMPARE(nullDateTimeRange.extendWith(QDateTime()), nullDateTimeRange);
}

void DateTimeRangeTestCase::testExtendNullWithNotNull()
{
    QCOMPARE(nullDateTimeRange.extendWith(now), pointDateTimeRange);
}

void DateTimeRangeTestCase::testExtendPointWithNull()
{
    QCOMPARE(pointDateTimeRange.extendWith(QDateTime()), pointDateTimeRange);
}

void DateTimeRangeTestCase::testExtendPointWithNotNull()
{
    QCOMPARE(pointDateTimeRange.extendWith(nowPlus99), length99DateTimeRange);
}

void DateTimeRangeTestCase::testExtendLengthWithNull()
{
    QCOMPARE(length99DateTimeRange.extendWith(QDateTime()), length99DateTimeRange);
    QCOMPARE(length99ShiftedBy20DateTimeRange.extendWith(QDateTime()), length99ShiftedBy20DateTimeRange);
}

void DateTimeRangeTestCase::testExtendLengthWithShiftedLength()
{
    DateTimeRange extended = length99DateTimeRange.extendWith(length99ShiftedBy20DateTimeRange.end());

    QVERIFY(!extended.isNull());
    QCOMPARE(extended.start(), length99DateTimeRange.start());
    QCOMPARE(extended.end(), length99ShiftedBy20DateTimeRange.end());
    QCOMPARE(extended.lengthInSeconds(), 99 + 20);
}

void DateTimeRangeTestCase::testExtendShiftedLengthWithLength()
{
    DateTimeRange extended = length99ShiftedBy20DateTimeRange.extendWith(length99DateTimeRange.start());

    QVERIFY(!extended.isNull());
    QCOMPARE(extended.start(), length99DateTimeRange.start());
    QCOMPARE(extended.end(), length99ShiftedBy20DateTimeRange.end());
    QCOMPARE(extended.lengthInSeconds(), 99 + 20);
}

void DateTimeRangeTestCase::testWithLengthInSecondsWithNullAnd0()
{
    QCOMPARE(nullDateTimeRange.withLengthInSeconds(0), nullDateTimeRange);
}

void DateTimeRangeTestCase::testWithLengthInSecondsWithNullAnd20()
{
    QCOMPARE(nullDateTimeRange.withLengthInSeconds(20), nullDateTimeRange);
}

void DateTimeRangeTestCase::testWithLengthInSecondsWithNotNullAnd0()
{
    DateTimeRange withLength = length99DateTimeRange.withLengthInSeconds(0);

    QVERIFY(!withLength.isNull());
    QCOMPARE(withLength.start(), length99DateTimeRange.start());
    QCOMPARE(withLength.end(), length99DateTimeRange.start());
    QCOMPARE(withLength.lengthInSeconds(), 0);
}

void DateTimeRangeTestCase::testWithLengthInSecondsWithNotNullAnd20()
{
    DateTimeRange withLength = length99DateTimeRange.withLengthInSeconds(20);

    QVERIFY(!withLength.isNull());
    QCOMPARE(withLength.start(), length99DateTimeRange.start());
    QCOMPARE(withLength.end(), length99DateTimeRange.start().addSecs(20));
    QCOMPARE(withLength.lengthInSeconds(), 20);
}

void DateTimeRangeTestCase::testMoveNullIntoNull()
{
    QVERIFY(nullDateTimeRange.moveInto(DateTimeRange()).isNull());
    QVERIFY(DateTimeRange().moveInto(nullDateTimeRange).isNull());
}

void DateTimeRangeTestCase::testMoveNullIntoNotNull()
{
    QVERIFY(nullDateTimeRange.moveInto(pointDateTimeRange).isNull());
}

void DateTimeRangeTestCase::testMoveNotNullIntoNull()
{
    QVERIFY(pointDateTimeRange.moveInto(nullDateTimeRange).isNull());
}

void DateTimeRangeTestCase::testMovePointIntoContainingRange()
{
    QCOMPARE(pointDateTimeRange.moveInto(length99DateTimeRange), pointDateTimeRange);
}

void DateTimeRangeTestCase::testMovePointIntoNotContainingRange()
{
    DateTimeRange moved = DateTimeRange(length99ShiftedBy20DateTimeRange.start(), length99ShiftedBy20DateTimeRange.start());

    QCOMPARE(pointDateTimeRange.moveInto(length99ShiftedBy20DateTimeRange), moved);
}

void DateTimeRangeTestCase::testMoveMatchingLengthIntoContainingRange()
{
    QCOMPARE(length99DateTimeRange.moveInto(length99DateTimeRange), length99DateTimeRange);
    QCOMPARE(length99ShiftedBy20DateTimeRange.moveInto(length99ShiftedBy20DateTimeRange), length99ShiftedBy20DateTimeRange);
}

void DateTimeRangeTestCase::testMoveMatchingLengthIntoNotContainingRange()
{
    QCOMPARE(length99DateTimeRange.moveInto(length99ShiftedBy20DateTimeRange), length99ShiftedBy20DateTimeRange);
    QCOMPARE(length99ShiftedBy20DateTimeRange.moveInto(length99DateTimeRange), length99DateTimeRange);
}

void DateTimeRangeTestCase::testMoveLongerIntoContainedRange()
{
    QCOMPARE(allDateTimeRange.moveInto(length99DateTimeRange), length99DateTimeRange);
    QCOMPARE(allDateTimeRange.moveInto(length99ShiftedBy20DateTimeRange), length99ShiftedBy20DateTimeRange);
}

void DateTimeRangeTestCase::testMoveLongerIntoRightOverlapingRange()
{
    DateTimeRange left = allDateTimeRange.moveStart(length99ShiftedBy20DateTimeRange.start().addSecs(-20));

    QCOMPARE(left.moveInto(length99ShiftedBy20DateTimeRange), length99ShiftedBy20DateTimeRange);
}

void DateTimeRangeTestCase::testMoveLongerIntoLeftOverlapingRange()
{
    DateTimeRange right = allDateTimeRange.moveStart(length99DateTimeRange.end().addSecs(-20));

    QCOMPARE(right.moveInto(length99DateTimeRange), length99DateTimeRange);
}

void DateTimeRangeTestCase::testMoveLongerIntoNonOverlapingRange()
{
    DateTimeRange range = allDateTimeRange.moveStart(length99ShiftedBy20DateTimeRange.end().addSecs(20));

    QCOMPARE(range.moveInto(length99DateTimeRange), length99DateTimeRange);
    QCOMPARE(range.moveInto(length99ShiftedBy20DateTimeRange), length99ShiftedBy20DateTimeRange);
}

void DateTimeRangeTestCase::testMoveStartNullIntoNull()
{
    QCOMPARE(nullDateTimeRange.moveStart(QDateTime()), nullDateTimeRange);
}

void DateTimeRangeTestCase::testMoveStartNullIntoNotNull()
{
    QCOMPARE(nullDateTimeRange.moveStart(now), nullDateTimeRange);
}

void DateTimeRangeTestCase::testMoveStartNotNullIntoNull()
{
    QCOMPARE(pointDateTimeRange.moveStart(QDateTime()), nullDateTimeRange);
}

void DateTimeRangeTestCase::testMoveStartPoint()
{
    DateTimeRange movedPoint = pointDateTimeRange.moveStart(length99ShiftedBy20DateTimeRange.end());

    QVERIFY(!movedPoint.isNull());
    QCOMPARE(movedPoint.start(), length99ShiftedBy20DateTimeRange.end());
    QCOMPARE(movedPoint.lengthInSeconds(), pointDateTimeRange.lengthInSeconds());
}

void DateTimeRangeTestCase::testMoveStartLength()
{
    DateTimeRange movedLength = length99DateTimeRange.moveStart(length99ShiftedBy20DateTimeRange.end());

    QVERIFY(!movedLength.isNull());
    QCOMPARE(movedLength.start(), length99ShiftedBy20DateTimeRange.end());
    QCOMPARE(movedLength.lengthInSeconds(), length99DateTimeRange.lengthInSeconds());
}

QTEST_MAIN(DateTimeRangeTestCase)

#include "DateTimeRangeTestCase.moc"
