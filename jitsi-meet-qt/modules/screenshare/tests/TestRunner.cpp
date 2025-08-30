#include "ScreenShareModuleTest.h"
#include <QApplication>
#include <QTest>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

/**
 * @brief 屏幕共享模块测试运行器
 * Screen Share Module Test Runner
 */
class ScreenShareTestRunner
{
public:
    static int runAllTests(int argc, char *argv[])
    {
        QApplication app(argc, argv);
        
        // 设置测试环境
        setupTestEnvironment();
        
        qDebug() << "==========================================";
        qDebug() << "屏幕共享模块测试套件";
        qDebug() << "Screen Share Module Test Suite";
        qDebug() << "==========================================";
        
        int totalTests = 0;
        int passedTests = 0;
        int failedTests = 0;
        
        // 运行测试
        ScreenShareModuleTest test;
        int result = QTest::qExec(&test, argc, argv);
        
        totalTests++;
        if (result == 0) {
            passedTests++;
            qDebug() << "✅ ScreenShareModuleTest: PASSED";
        } else {
            failedTests++;
            qDebug() << "❌ ScreenShareModuleTest: FAILED";
        }
        
        // 输出测试结果摘要
        printTestSummary(totalTests, passedTests, failedTests);
        
        return (failedTests > 0) ? 1 : 0;
    }

private:
    static void setupTestEnvironment()
    {
        // 设置测试环境变量
        qputenv("QT_QPA_PLATFORM", "offscreen");
        qputenv("QT_LOGGING_RULES", "*.debug=false");
        
        // 创建测试输出目录
        QString testOutputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/screenshare_tests";
        QDir().mkpath(testOutputDir);
        
        qDebug() << "Test environment setup complete";
        qDebug() << "Test output directory:" << testOutputDir;
    }
    
    static void printTestSummary(int total, int passed, int failed)
    {
        qDebug() << "==========================================";
        qDebug() << "测试结果摘要 / Test Results Summary";
        qDebug() << "==========================================";
        qDebug() << "总测试数 / Total Tests:" << total;
        qDebug() << "通过测试 / Passed Tests:" << passed;
        qDebug() << "失败测试 / Failed Tests:" << failed;
        
        if (failed == 0) {
            qDebug() << "✅ 所有测试通过! / All tests passed!";
        } else {
            qDebug() << "❌ 有测试失败 / Some tests failed";
        }
        qDebug() << "==========================================";
    }
};

int main(int argc, char *argv[])
{
    return ScreenShareTestRunner::runAllTests(argc, argv);
}