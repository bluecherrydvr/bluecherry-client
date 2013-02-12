#include "src/utils/RangeTestCase.h"
#include "src/utils/RangeMapTestCase.h"

#include <QTest>

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    RangeTestCase rangeTestCase;
    RangeMapTestCase rangeMapTestCase;

    QTest::qExec(&rangeTestCase);
    QTest::qExec(&rangeMapTestCase);

    return 0;
}
