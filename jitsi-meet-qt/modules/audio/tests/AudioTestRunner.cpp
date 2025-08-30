#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <QTimer>

#include "AudioTestSuite.h"
#include "AudioModuleTest.h"

/**
 * @brief 音频模块测试运行器
 * 
 * 提供命令行接口来运行音频模块的各种测试
 */
class AudioTestRunner : public QObject
{
    Q_OBJECT

public:
    AudioTestRunner(QObject *parent = nullptr) : QObject(parent) {}

    int run(const QStringList &arguments)
    {
        QCommandLineParser parser;
        parser.setApplicationDescription("Jitsi Meet Qt 音频模块测试运行器");
        parser.addHelpOption();
        parser.addVersionOption();

        // 添加命令行选项
        QCommandLineOption allTestsOption(QStringList() << "a" << "all",
                                         "运行所有测试");
        parser.addOption(allTestsOption);

        QCommandLineOption categoryOption(QStringList() << "c" << "category",
                                         "运行指定类别的测试 (basic|device|quality|latency|performance|stress|compatibility|integration)",
                                         "category");
        parser.addOption(categoryOption);

        QCommandLineOption singleTestOption(QStringList() << "t" << "test",
                                           "运行单个测试",
                                           "testname");
        parser.addOption(singleTestOption);

        QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
                                        "详细输出模式");
        parser.addOption(verboseOption);

        QCommandLineOption benchmarkOption(QStringList() << "b" << "benchmark",
                                          "性能基准测试模式");
        parser.addOption(benchmarkOption);

        QCommandLineOption reportOption(QStringList() << "r" << "report",
                                       "生成测试报告",
                                       "filepath");
        parser.addOption(reportOption);

        QCommandLineOption htmlReportOption("html-report",
                                           "生成HTML测试报告",
                                           "filepath");
        parser.addOption(htmlReportOption);

        QCommandLineOption timeoutOption("timeout",
                                        "设置测试超时时间(秒)",
                                        "seconds", "30");
        parser.addOption(timeoutOption);

        // 解析命令行参数
        parser.process(arguments);

        // 创建测试套件
        AudioTestSuite testSuite;

        // 设置选项
        if (parser.isSet(verboseOption)) {
            testSuite.setVerboseMode(true);
            qDebug() << "启用详细输出模式";
        }

        if (parser.isSet(benchmarkOption)) {
            testSuite.setBenchmarkMode(true);
            qDebug() << "启用性能基准测试模式";
        }

        int timeout = parser.value(timeoutOption).toInt() * 1000; // 转换为毫秒
        testSuite.setTestTimeout(timeout);

        // 连接信号
        connect(&testSuite, &AudioTestSuite::testStarted,
                this, &AudioTestRunner::onTestStarted);
        connect(&testSuite, &AudioTestSuite::testCompleted,
                this, &AudioTestRunner::onTestCompleted);
        connect(&testSuite, &AudioTestSuite::testProgress,
                this, &AudioTestRunner::onTestProgress);
        connect(&testSuite, &AudioTestSuite::testSuiteCompleted,
                this, &AudioTestRunner::onTestSuiteCompleted);

        bool success = false;

        // 执行测试
        if (parser.isSet(allTestsOption)) {
            qDebug() << "运行所有音频模块测试...";
            success = testSuite.runAllTests();
        }
        else if (parser.isSet(categoryOption)) {
            QString categoryName = parser.value(categoryOption);
            AudioTestSuite::TestCategory category = parseTestCategory(categoryName);
            
            if (category != AudioTestSuite::BasicTests || categoryName == "basic") {
                qDebug() << "运行测试类别:" << categoryName;
                success = testSuite.runTestCategory(category);
            } else {
                qWarning() << "未知的测试类别:" << categoryName;
                return 1;
            }
        }
        else if (parser.isSet(singleTestOption)) {
            QString testName = parser.value(singleTestOption);
            qDebug() << "运行单个测试:" << testName;
            success = testSuite.runSingleTest(testName);
        }
        else {
            // 默认运行基础测试
            qDebug() << "运行基础测试...";
            success = testSuite.runTestCategory(AudioTestSuite::BasicTests);
        }

        // 生成报告
        if (parser.isSet(reportOption)) {
            QString reportPath = parser.value(reportOption);
            if (testSuite.generateReport(reportPath)) {
                qDebug() << "测试报告已生成:" << reportPath;
            } else {
                qWarning() << "生成测试报告失败:" << reportPath;
            }
        }

        if (parser.isSet(htmlReportOption)) {
            QString htmlPath = parser.value(htmlReportOption);
            if (testSuite.generateHtmlReport(htmlPath)) {
                qDebug() << "HTML测试报告已生成:" << htmlPath;
            } else {
                qWarning() << "生成HTML测试报告失败:" << htmlPath;
            }
        }

        // 输出测试统计
        QVariantMap stats = testSuite.testStatistics();
        qDebug() << "\n=== 测试统计 ===";
        qDebug() << "总测试数:" << stats["total"].toInt();
        qDebug() << "通过:" << stats["passed"].toInt();
        qDebug() << "失败:" << stats["failed"].toInt();
        qDebug() << "跳过:" << stats["skipped"].toInt();
        qDebug() << "错误:" << stats["errors"].toInt();
        qDebug() << "成功率:" << QString::number(stats["successRate"].toDouble(), 'f', 1) << "%";
        qDebug() << "总时间:" << stats["totalTime"].toLongLong() << "ms";

        return success ? 0 : 1;
    }

private slots:
    void onTestStarted(const QString &testName)
    {
        qDebug() << "开始测试:" << testName;
    }

    void onTestCompleted(const QString &testName, AudioTestSuite::TestResult result)
    {
        QString resultText;
        switch (result) {
        case AudioTestSuite::Passed: resultText = "通过"; break;
        case AudioTestSuite::Failed: resultText = "失败"; break;
        case AudioTestSuite::Error: resultText = "错误"; break;
        case AudioTestSuite::Skipped: resultText = "跳过"; break;
        }
        
        qDebug() << "测试完成:" << testName << "-" << resultText;
    }

    void onTestProgress(int current, int total)
    {
        qDebug() << QString("测试进度: %1/%2 (%3%)")
                    .arg(current)
                    .arg(total)
                    .arg((double)current / total * 100.0, 0, 'f', 1);
    }

    void onTestSuiteCompleted(int passed, int failed)
    {
        qDebug() << QString("测试套件完成: 通过 %1, 失败 %2").arg(passed).arg(failed);
    }

private:
    AudioTestSuite::TestCategory parseTestCategory(const QString &categoryName)
    {
        if (categoryName == "basic") return AudioTestSuite::BasicTests;
        if (categoryName == "device") return AudioTestSuite::DeviceTests;
        if (categoryName == "quality") return AudioTestSuite::QualityTests;
        if (categoryName == "latency") return AudioTestSuite::LatencyTests;
        if (categoryName == "performance") return AudioTestSuite::PerformanceTests;
        if (categoryName == "stress") return AudioTestSuite::StressTests;
        if (categoryName == "compatibility") return AudioTestSuite::CompatibilityTests;
        if (categoryName == "integration") return AudioTestSuite::IntegrationTests;
        if (categoryName == "platform") return AudioTestSuite::PlatformTests;
        
        return AudioTestSuite::BasicTests; // 默认
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("AudioTestRunner");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi");
    app.setOrganizationDomain("jitsi.org");

    qDebug() << "Jitsi Meet Qt 音频模块测试运行器 v1.0.0";
    qDebug() << "Qt版本:" << QT_VERSION_STR;
    qDebug() << "平台:" << QSysInfo::prettyProductName();

    AudioTestRunner runner;
    int result = runner.run(app.arguments());

    // 延迟退出以确保所有输出完成
    QTimer::singleShot(100, &app, &QCoreApplication::quit);
    app.exec();

    return result;
}

#include "AudioTestRunner.moc"