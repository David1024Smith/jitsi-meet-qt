#include <QtTest/QtTest>
#include <QObject>

/**
 * @brief Simple unit test to verify test framework works
 */
class TestSimple : public QObject
{
    Q_OBJECT

private slots:
    void testBasicAssertion();
    void testStringComparison();
    void testNumericComparison();
};

void TestSimple::testBasicAssertion()
{
    QVERIFY(true);
    QVERIFY(1 == 1);
    QVERIFY(!false);
}

void TestSimple::testStringComparison()
{
    QString str1 = "Hello";
    QString str2 = "Hello";
    QString str3 = "World";
    
    QCOMPARE(str1, str2);
    QVERIFY(str1 != str3);
    QVERIFY(!str1.isEmpty());
}

void TestSimple::testNumericComparison()
{
    int a = 5;
    int b = 10;
    
    QCOMPARE(a + b, 15);
    QVERIFY(a < b);
    QVERIFY(b > a);
}

QTEST_MAIN(TestSimple)
#include "test_simple_unit.moc"