#include "utils/Range.h"
#include <QtTest/QtTest>
#include <QDebug>

const char *jpegFormatName = "jpeg"; // hack

class RangeTestCase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInvalidRange();
    void testValueRange();
    void testStartEndRange();
    void testStartSizeRange();
};

void RangeTestCase::testInvalidRange()
{
    Range defaultRange;
    QVERIFY(!defaultRange.isValid());

    Range invalidRange = Range::invalid();
    QVERIFY(!invalidRange.isValid());

    Range valueRange = Range::fromValue(0);
    QVERIFY(valueRange.isValid());

    valueRange = Range::fromValue(1);
    QVERIFY(valueRange.isValid());

    valueRange = Range::fromValue(330);
    QVERIFY(valueRange.isValid());
}

void RangeTestCase::testValueRange()
{
    Range valueRange = Range::fromValue(0);
    QVERIFY(valueRange.isValid());
    QCOMPARE(valueRange.size(), 1u);
    QCOMPARE(valueRange.start(), 0u);
    QCOMPARE(valueRange.end(), 0u);

    valueRange = Range::fromValue(1);
    QVERIFY(valueRange.isValid());
    QCOMPARE(valueRange.size(), 1u);
    QCOMPARE(valueRange.start(), 1u);
    QCOMPARE(valueRange.end(), 1u);

    valueRange = Range::fromValue(330);
    QVERIFY(valueRange.isValid());
    QCOMPARE(valueRange.size(), 1u);
    QCOMPARE(valueRange.start(), 330u);
    QCOMPARE(valueRange.end(), 330u);
}

void RangeTestCase::testStartEndRange()
{
    Range startEndRange = Range::fromStartEnd(0, 0);
    QVERIFY(startEndRange.isValid());
    QCOMPARE(startEndRange.size(), 1u);
    QCOMPARE(startEndRange.start(), 0u);
    QCOMPARE(startEndRange.end(), 0u);

    startEndRange = Range::fromStartEnd(1, 1);
    QVERIFY(startEndRange.isValid());
    QCOMPARE(startEndRange.size(), 1u);
    QCOMPARE(startEndRange.start(), 1u);
    QCOMPARE(startEndRange.end(), 1u);

    startEndRange = Range::fromStartEnd(330, 330);
    QVERIFY(startEndRange.isValid());
    QCOMPARE(startEndRange.size(), 1u);
    QCOMPARE(startEndRange.start(), 330u);
    QCOMPARE(startEndRange.end(), 330u);

    startEndRange = Range::fromStartEnd(4, 2);
    QVERIFY(!startEndRange.isValid());

    startEndRange = Range::fromStartEnd(0, 1);
    QVERIFY(startEndRange.isValid());
    QCOMPARE(startEndRange.size(), 2u);
    QCOMPARE(startEndRange.start(), 0u);
    QCOMPARE(startEndRange.end(), 1u);

    startEndRange = Range::fromStartEnd(12, 17);
    QVERIFY(startEndRange.isValid());
    QCOMPARE(startEndRange.size(), 6u);
    QCOMPARE(startEndRange.start(), 12u);
    QCOMPARE(startEndRange.end(), 17u);
}

void RangeTestCase::testStartSizeRange()
{
    Range startSizeRange = Range::fromStartSize(5, 0);
    QVERIFY(!startSizeRange.isValid());

    startSizeRange = Range::fromStartSize(5, 1);
    QVERIFY(startSizeRange.isValid());
    QCOMPARE(startSizeRange.size(), 1u);
    QCOMPARE(startSizeRange.start(), 5u);
    QCOMPARE(startSizeRange.end(), 5u);

    startSizeRange = Range::fromStartSize(5, 2);
    QVERIFY(startSizeRange.isValid());
    QCOMPARE(startSizeRange.size(), 2u);
    QCOMPARE(startSizeRange.start(), 5u);
    QCOMPARE(startSizeRange.end(), 6u);

    startSizeRange = Range::fromStartSize(0, 12);
    QVERIFY(startSizeRange.isValid());
    QCOMPARE(startSizeRange.size(), 12u);
    QCOMPARE(startSizeRange.start(), 0u);
    QCOMPARE(startSizeRange.end(), 11u);
}

QTEST_MAIN(RangeTestCase)

#include "RangeTestCase.moc"
