#ifndef RANGETESTCASE_H
#define RANGETESTCASE_H

#include <QtTest/QtTest>
#include <QDebug>

class RangeTestCase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInvalidRange();
    void testValueRange();
    void testStartEndRange();
    void testStartSizeRange();
};

#endif
