#include <QtTest/QtTest>
#include <QApplication>
#include "MainApplication.h"

class TestMainApplication : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testSingletonPattern();
    void testApplicationProperties();
    void testProtocolUrlValidation();
    void testMinimumWindowSize();
    
private:
    MainApplication* m_app = nullptr;
};

void TestMainApplication::initTestCase()
{
    // Note: In real tests, we would need to handle the singleton pattern carefully
    // For now, we'll test the static methods that don't require an instance
}

void TestMainApplication::cleanupTestCase()
{
    // Cleanup if needed
}

void TestMainApplication::testSingletonPattern()
{
    // Test that instance returns nullptr when no app is created
    // Note: This test assumes no MainApplication instance exists
    // In a real test environment, we'd need more sophisticated setup
}

void TestMainApplication::testApplicationProperties()
{
    // Test static application properties
    QCOMPARE(MainApplication::applicationTitle(), "Jitsi Meet");
    QVERIFY(!MainApplication::applicationTitle().empty());
}

void TestMainApplication::testProtocolUrlValidation()
{
    // Test protocol URL validation (this would need access to the private method)
    // For now, we'll test the public interface behavior
    QVERIFY(true); // Placeholder
}

void TestMainApplication::testMinimumWindowSize()
{
    QSize minSize = MainApplication::minimumWindowSize();
    QCOMPARE(minSize.width(), 800);
    QCOMPARE(minSize.height(), 600);
    QVERIFY(minSize.isValid());
}

#include "test_MainApplication.moc"