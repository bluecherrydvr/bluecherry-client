#include "src/utils/RangeTestCase.h"

#include <QTest>

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    RangeTestCase rangeTestCase;

    QTest::qExec(&rangeTestCase);

    return 0;
}
