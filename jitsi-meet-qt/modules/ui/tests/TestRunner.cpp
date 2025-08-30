#include <QtTest/QtTest>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QTimer>

#include "UIModuleTest.h"
#include "widgets/UIComponentsTest.cpp"

/**
 * @brief Test runner for UI Module tests
 * 
 * This class coordinates the execution of all UI module tests
 * and provides consolidated reporting.
 */
class UITestRunner : public QObject
{
    Q_OBJECT

public:
    UITestRunner(QObject* parent = nullptr);
    ~UITestRunner();

    int runAllTests(int argc, char* argv[]);

private slots:
    void runUIModuleTests();
    void runUIComponentsTests();

private:
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    void printTestSummary();

    int m_totalTests;
    int m_passedTests;
    int m_failedTests;
    QStringList m_failedTestNames;
};

UITestRunner::UITestRunner(QObject* parent)
    : QObject(parent)
    , m_totalTests(0)
    , m_passedTests(0)
    , m_failedTests(0)
{
}

UITestRunner::~UITestRunner()
{
}

int UITestRunner::runAllTests(int argc, char* argv[])
{
    qDebug() << "Starting UI Module Test Suite...";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Test Framework: Qt Test";
    
    setupTestEnvironment();
    
    // Run UI Module Tests
    {
        UIModuleTest uiModuleTest;
        int result = QTest::qExec(&uiModuleTest, argc, argv);
        m_totalTests++;
        if (result == 0) {
            m_passedTests++;
            qDebug() << "✓ UIModuleTest: PASSED";
        } else {
            m_failedTests++;
            m_failedTestNames << "UIModuleTest";
            qDebug() << "✗ UIModuleTest: FAILED";
        }
    }
    
    // Run UI Components Tests
    {
        UIComponentsTest uiComponentsTest;
        int result = QTest::qExec(&uiComponentsTest, argc, argv);
        m_totalTests++;
        if (result == 0) {
            m_passedTests++;
            qDebug() << "✓ UIComponentsTest: PASSED";
        } else {
            m_failedTests++;
            m_failedTestNames << "UIComponentsTest";
            qDebug() << "✗ UIComponentsTest: FAILED";
        }
    }
    
    cleanupTestEnvironment();
    printTestSummary();
    
    return m_failedTests > 0 ? 1 : 0;
}

void UITestRunner::setupTestEnvironment()
{
    // Set up test environment
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    
    // Create test data directory if it doesn't exist
    QDir testDataDir("data");
    if (!testDataDir.exists()) {
        testDataDir.mkpath(".");
    }
    
    // Set environment variables for headless testing
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "*.debug=false");
    
    qDebug() << "Test environment set up successfully";
}

void UITestRunner::cleanupTestEnvironment()
{
    // Clean up any temporary test files
    QDir tempDir("temp");
    if (tempDir.exists()) {
        tempDir.removeRecursively();
    }
    
    qDebug() << "Test environment cleaned up";
}

void UITestRunner::printTestSummary()
{
    qDebug() << "\n" << QString(50, '=');
    qDebug() << "UI MODULE TEST SUMMARY";
    qDebug() << QString(50, '=');
    qDebug() << "Total Tests:" << m_totalTests;
    qDebug() << "Passed:" << m_passedTests;
    qDebug() << "Failed:" << m_failedTests;
    
    if (m_failedTests > 0) {
        qDebug() << "\nFailed Tests:";
        for (const QString& testName : m_failedTestNames) {
            qDebug() << "  -" << testName;
        }
    }
    
    qDebug() << QString(50, '=');
    
    if (m_failedTests == 0) {
        qDebug() << "🎉 ALL TESTS PASSED! 🎉";
    } else {
        qDebug() << "❌ SOME TESTS FAILED ❌";
    }
    
    qDebug() << QString(50, '=') << "\n";
}

// Main function for standalone test execution
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("UI Module Tests");
    app.setApplicationVersion("1.0.0");
    
    // Parse command line arguments
    QStringList arguments = app.arguments();
    bool verbose = arguments.contains("-v") || arguments.contains("--verbose");
    bool help = arguments.contains("-h") || arguments.contains("--help");
    
    if (help) {
        qDebug() << "UI Module Test Runner";
        qDebug() << "Usage:" << argv[0] << "[OPTIONS]";
        qDebug() << "";
        qDebug() << "Options:";
        qDebug() << "  -h, --help     Show this help message";
        qDebug() << "  -v, --verbose  Enable verbose output";
        qDebug() << "  -o file        Output results to file";
        qDebug() << "";
        qDebug() << "Test Categories:";
        qDebug() << "  UIModuleTest      - Core UI module functionality";
        qDebug() << "  UIComponentsTest  - UI component functionality";
        return 0;
    }
    
    if (verbose) {
        qDebug() << "Verbose mode enabled";
    }
    
    UITestRunner runner;
    int result = runner.runAllTests(argc, argv);
    
    // Exit after a short delay to ensure all output is flushed
    QTimer::singleShot(100, &app, &QApplication::quit);
    app.exec();
    
    return result;
}

#include "TestRunner.moc"