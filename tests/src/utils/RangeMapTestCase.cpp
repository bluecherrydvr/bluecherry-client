#include "utils/RangeMap.h"
#include <QtTest/QtTest>
#include <QDebug>

const char *jpegFormatName = "jpeg"; // hack

class RangeMapTestCase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkRangeMapInsert();
    void checkEmptyRange();
    void checkOneItemRange();
    void checkTwoItemsRange();
    void checkMergedRange();
    void checkOverlappingRange();
    void checkBigOverlappingRange();
    void checkMissingRangeEmpty();
    void checkMissingRangeOneItem();
    void checkMissingRangeTwoItems();

    void testStreamingError();
};

void RangeMapTestCase::checkRangeMapInsert()
{
    RangeMap rangeMap;

    QCOMPARE(rangeMap.size(), 0);

    rangeMap.insert(Range::fromStartSize(40, 10));
    QCOMPARE(rangeMap.size(), 1);

    rangeMap.insert(Range::fromStartSize(20, 10));
    QCOMPARE(rangeMap.size(), 2);

    rangeMap.insert(Range::fromStartSize(0, 10));
    QCOMPARE(rangeMap.size(), 3);

    rangeMap.insert(Range::fromStartSize(45, 2));
    QCOMPARE(rangeMap.size(), 3);

    rangeMap.insert(Range::fromStartSize(60, 10));
    QCOMPARE(rangeMap.size(), 4);

    rangeMap.insert(Range::fromStartSize(12, 2));
    QCOMPARE(rangeMap.size(), 5);

    rangeMap.insert(Range::fromStartSize(0, 30));
    QCOMPARE(rangeMap.size(), 3);

    rangeMap.insert(Range::fromStartSize(0, 50));
    QCOMPARE(rangeMap.size(), 2);

    rangeMap.insert(Range::fromStartSize(40, 30));
    QCOMPARE(rangeMap.size(), 1);
}

void RangeMapTestCase::checkEmptyRange()
{
    RangeMap emptyRange;

    QCOMPARE(emptyRange.size(), 0);
    QVERIFY(!emptyRange.contains(Range::fromValue(0)));
    QVERIFY(!emptyRange.contains(Range::fromStartSize(1, 12)));
    QVERIFY(!emptyRange.contains(Range::fromStartSize(24, 48)));
}

void RangeMapTestCase::checkOneItemRange()
{
    RangeMap oneItemRange;
    oneItemRange.insert(Range::fromStartSize(20, 20));

    QCOMPARE(oneItemRange.size(), 1);
    QVERIFY(!oneItemRange.contains(Range::fromStartSize(0, 10)));
    QVERIFY(!oneItemRange.contains(Range::fromStartSize(0, 20)));
    QVERIFY(!oneItemRange.contains(Range::fromValue(19)));
    QVERIFY(oneItemRange.contains(Range::fromValue(20)));
    QVERIFY(oneItemRange.contains(Range::fromStartSize(20, 1)));
    QVERIFY(oneItemRange.contains(Range::fromStartSize(20, 20)));
    QVERIFY(!oneItemRange.contains(Range::fromStartSize(20, 21)));
    QVERIFY(!oneItemRange.contains(Range::fromStartSize(10, 10)));
    QVERIFY(!oneItemRange.contains(Range::fromStartSize(10, 11)));
    QVERIFY(oneItemRange.contains(Range::fromValue(38)));
    QVERIFY(oneItemRange.contains(Range::fromValue(39)));
}

void RangeMapTestCase::checkTwoItemsRange()
{
    RangeMap twoItemsRange;
    twoItemsRange.insert(Range::fromStartSize(5, 5));
    twoItemsRange.insert(Range::fromStartSize(20, 20));

    QCOMPARE(twoItemsRange.size(), 2);
    QVERIFY(!twoItemsRange.contains(Range::fromValue(4)));
    QVERIFY(twoItemsRange.contains(Range::fromValue(5)));
    QVERIFY(twoItemsRange.contains(Range::fromValue(9)));
    QVERIFY(!twoItemsRange.contains(Range::fromValue(10)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(4, 6)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(5, 6)));
    QVERIFY(twoItemsRange.contains(Range::fromStartSize(5, 5)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(6, 5)));
    QVERIFY(twoItemsRange.contains(Range::fromStartSize(6, 4)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(10, 20)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(10, 10)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(0, 10)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(0, 20)));
    QVERIFY(!twoItemsRange.contains(Range::fromValue(19)));
    QVERIFY(twoItemsRange.contains(Range::fromValue(20)));
    QVERIFY(twoItemsRange.contains(Range::fromStartSize(20, 1)));
    QVERIFY(twoItemsRange.contains(Range::fromStartSize(20, 20)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(20, 21)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(10, 10)));
    QVERIFY(!twoItemsRange.contains(Range::fromStartSize(10, 11)));
    QVERIFY(twoItemsRange.contains(Range::fromValue(38)));
    QVERIFY(twoItemsRange.contains(Range::fromValue(39)));
}

void RangeMapTestCase::checkMergedRange()
{
    RangeMap mergedRange;
    mergedRange.insert(Range::fromStartSize(20, 10));
    mergedRange.insert(Range::fromStartSize(30, 10));

    QCOMPARE(mergedRange.size(), 1);
    QVERIFY(!mergedRange.contains(Range::fromStartSize(0, 10)));
    QVERIFY(!mergedRange.contains(Range::fromStartSize(0, 20)));
    QVERIFY(!mergedRange.contains(Range::fromValue(19)));
    QVERIFY(mergedRange.contains(Range::fromValue(20)));
    QVERIFY(mergedRange.contains(Range::fromStartSize(20, 1)));
    QVERIFY(mergedRange.contains(Range::fromStartSize(20, 20)));
    QVERIFY(!mergedRange.contains(Range::fromStartSize(20, 21)));
    QVERIFY(!mergedRange.contains(Range::fromStartSize(10, 10)));
    QVERIFY(!mergedRange.contains(Range::fromStartSize(10, 11)));
    QVERIFY(mergedRange.contains(Range::fromValue(38)));
    QVERIFY(mergedRange.contains(Range::fromValue(39)));
}

void RangeMapTestCase::checkOverlappingRange()
{
    RangeMap overlappingRange;
    overlappingRange.insert(Range::fromStartSize(20, 15));
    overlappingRange.insert(Range::fromStartSize(25, 15));

    QCOMPARE(overlappingRange.size(), 1);
    QVERIFY(!overlappingRange.contains(Range::fromStartSize(0, 10)));
    QVERIFY(!overlappingRange.contains(Range::fromStartSize(0, 20)));
    QVERIFY(!overlappingRange.contains(Range::fromValue(19)));
    QVERIFY(overlappingRange.contains(Range::fromValue(20)));
    QVERIFY(overlappingRange.contains(Range::fromStartSize(20, 1)));
    QVERIFY(overlappingRange.contains(Range::fromStartSize(20, 20)));
    QVERIFY(!overlappingRange.contains(Range::fromStartSize(20, 21)));
    QVERIFY(!overlappingRange.contains(Range::fromStartSize(10, 10)));
    QVERIFY(!overlappingRange.contains(Range::fromStartSize(10, 11)));
    QVERIFY(overlappingRange.contains(Range::fromValue(38)));
    QVERIFY(overlappingRange.contains(Range::fromValue(39)));
}

void RangeMapTestCase::checkBigOverlappingRange()
{
    RangeMap bigOverlappingRange;
    bigOverlappingRange.insert(Range::fromStartSize(1, 1));
    QCOMPARE(bigOverlappingRange.size(), 1);

    bigOverlappingRange.insert(Range::fromStartSize(3, 1));
    QCOMPARE(bigOverlappingRange.size(), 2);

    bigOverlappingRange.insert(Range::fromStartSize(5, 1));
    QCOMPARE(bigOverlappingRange.size(), 3);

    bigOverlappingRange.insert(Range::fromStartSize(20, 15));
    QCOMPARE(bigOverlappingRange.size(), 4);

    bigOverlappingRange.insert(Range::fromStartSize(25, 15));
    QCOMPARE(bigOverlappingRange.size(), 4);

    bigOverlappingRange.insert(Range::fromStartSize(100, 100));
    QCOMPARE(bigOverlappingRange.size(), 5);

    bigOverlappingRange.insert(Range::fromStartSize(1, 1000));

    QCOMPARE(bigOverlappingRange.size(), 1);
}

void RangeMapTestCase::checkMissingRangeEmpty()
{
    RangeMap emptyRange;

    Range result;

    result = emptyRange.nextMissingRange(Range::fromStartSize(0, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 0u);
    QCOMPARE(result.size(), 10u);

    result = emptyRange.nextMissingRange(Range::fromStartSize(12, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 12u);
    QCOMPARE(result.size(), 10u);
}

void RangeMapTestCase::checkMissingRangeOneItem()
{
    RangeMap oneItemRange;
    oneItemRange.insert(Range::fromStartSize(20, 20));

    Range result;

    result = oneItemRange.nextMissingRange(Range::fromStartSize(0, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 0u);
    QCOMPARE(result.size(), 10u);

    result = oneItemRange.nextMissingRange(Range::fromStartSize(12, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 12u);
    QCOMPARE(result.size(), 8u);

    result = oneItemRange.nextMissingRange(Range::fromStartSize(30, 11));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 40u);
    QCOMPARE(result.size(), 1u);

    result = oneItemRange.nextMissingRange(Range::fromStartSize(19, 11));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 19u);
    QCOMPARE(result.size(), 1u);

    result = oneItemRange.nextMissingRange(Range::fromStartSize(20, 20));
    QVERIFY(!result.isValid());

    result = oneItemRange.nextMissingRange(Range::fromStartSize(20, 10));
    QVERIFY(!result.isValid());

    result = oneItemRange.nextMissingRange(Range::fromStartSize(30, 10));
    QVERIFY(!result.isValid());
}


void RangeMapTestCase::checkMissingRangeTwoItems()
{
    RangeMap twoItemsRange;
    twoItemsRange.insert(Range::fromStartSize(5, 5));
    twoItemsRange.insert(Range::fromStartSize(20, 20));

    Range result;

    result = twoItemsRange.nextMissingRange(Range::fromStartSize(0, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 0u);
    QCOMPARE(result.size(), 5u);

    result = twoItemsRange.nextMissingRange(Range::fromStartSize(8, 16));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 10u);
    QCOMPARE(result.size(), 10u);

    result = twoItemsRange.nextMissingRange(Range::fromStartSize(12, 10));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 12u);
    QCOMPARE(result.size(), 8u);

    result = twoItemsRange.nextMissingRange(Range::fromStartSize(12, 4));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 12u);
    QCOMPARE(result.size(), 4u);

    result = twoItemsRange.nextMissingRange(Range::fromStartSize(50, 19));
    QVERIFY(result.isValid());
    QCOMPARE(result.start(), 50u);
    QCOMPARE(result.size(), 19u);
}

void RangeMapTestCase::testStreamingError()
{
    RangeMap stramingErrorRangeMap;
    stramingErrorRangeMap.insert(Range::fromStartEnd(0, 81919));
    stramingErrorRangeMap.insert(Range::fromStartEnd(643425, 651616));
    stramingErrorRangeMap.insert(Range::fromStartEnd(1382819, 1478850));

    Range missingRange = Range::fromStartEnd(739425, 743520);

    QVERIFY(!stramingErrorRangeMap.contains(missingRange));
}

QTEST_MAIN(RangeMapTestCase)

#include "RangeMapTestCase.moc"
