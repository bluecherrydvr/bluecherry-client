#ifndef RANGEMAPTESTCASE_H
#define RANGEMAPTESTCASE_H

#include <QtTest/QtTest>
#include <QDebug>

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
};

#endif
