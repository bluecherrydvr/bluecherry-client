#include "core/Version.h"
#include <QtTest/QtTest>
#include <QDebug>

const char *jpegFormatName = "jpeg"; // hack

class VersionTestCase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkEmptyVersion();
    void checkEmptyStringVersion();
    void checkOneItemInvalidVersion();
    void checkOneTwoItemsInvalidVersion();
    void checkOneThreeItemsInvalidVersion();
    void checkOneFourItemsInvalidVersion();

    void checkAllZeros();
    void checkAllZerosWithSpec();
    void checkFirstNegative();
    void checkSecondNegative();
    void checkThirdNegative();

    void checkParser();
    void checkGreater();

};

void VersionTestCase::checkEmptyVersion()
{
    Version version;
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkEmptyStringVersion()
{
    Version version = Version::fromString(QString());
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkOneItemInvalidVersion()
{
    Version version = Version::fromString(QLatin1String("1"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkOneTwoItemsInvalidVersion()
{
    Version version = Version::fromString(QLatin1String("1.2"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkOneThreeItemsInvalidVersion()
{
    Version version = Version::fromString(QLatin1String("1.2.notanumber"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkOneFourItemsInvalidVersion()
{
    Version version = Version::fromString(QLatin1String("1.2.notanumber.4"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkAllZeros()
{
    Version version = Version(0, 0, 0, QString());
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkAllZerosWithSpec()
{
    Version version = Version(0, 0, 0, QLatin1String("git"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(!version.spec().isEmpty());
}

void VersionTestCase::checkFirstNegative()
{
    Version version = Version::fromString(QLatin1String("-1.0.0"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkSecondNegative()
{
    Version version = Version::fromString(QLatin1String("0.-1.0"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkThirdNegative()
{
    Version version = Version::fromString(QLatin1String("0.0.-1"));
    QVERIFY(!version.isValid());
    QCOMPARE(version.major(), (quint16)0);
    QCOMPARE(version.minor(), (quint16)0);
    QCOMPARE(version.fix(), (quint16)0);
    QVERIFY(version.spec().isEmpty());
}

void VersionTestCase::checkParser()
{
    Version version = Version::fromString(QLatin1String("1.12.123.git"));
    QVERIFY(version.isValid());
    QCOMPARE(version.major(), (quint16)1);
    QCOMPARE(version.minor(), (quint16)12);
    QCOMPARE(version.fix(), (quint16)123);
    QCOMPARE(version.spec(), QString::fromLatin1("git"));
}

void VersionTestCase::checkGreater()
{
    QVERIFY(Version::fromString(QLatin1String("1.0.0")) > Version::fromString(QLatin1String("0.0.1")));
    QVERIFY(Version::fromString(QLatin1String("1.0.0")) > Version::fromString(QLatin1String("0.1.0")));
    QVERIFY(Version::fromString(QLatin1String("1.0.0")) > Version::fromString(QLatin1String("0.1.1")));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0")) > Version::fromString(QLatin1String("1.0.0"))));
    QVERIFY(Version::fromString(QLatin1String("1.0.1")) > Version::fromString(QLatin1String("1.0.0")));
    QVERIFY(Version::fromString(QLatin1String("1.1.0")) > Version::fromString(QLatin1String("1.0.0")));
    QVERIFY(Version::fromString(QLatin1String("2.0.0")) > Version::fromString(QLatin1String("1.0.0")));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0")) > Version::fromString(QLatin1String("1.0.0.git"))));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0.git")) > Version::fromString(QLatin1String("1.0.0"))));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0.git")) > Version::fromString(QLatin1String("1.0.0.git"))));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0.git")) > Version::fromString(QLatin1String("1.0.0.svn"))));
    QVERIFY(!(Version::fromString(QLatin1String("1.0.0.svn")) > Version::fromString(QLatin1String("1.0.0.git"))));
}

QTEST_MAIN(VersionTestCase)

#include "VersionTestCase.moc"
