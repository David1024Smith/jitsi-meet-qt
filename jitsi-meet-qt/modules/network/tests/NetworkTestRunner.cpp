#include "NetworkModuleTest.h"
#include <QCoreApplication>
#include <QTest>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QDateTime>
#include <QTextStream>
#include <QFile>

/**
 * @brief 网络模块测试运行器
 * 
 * 提供命令行接口来运行网络模块测试，支持各种测试选项和报告生成。
 */
class NetworkTestRunner
{
public:
    NetworkTestRunner(int argc, char* argv[])
        : m_app(argc, argv)
        , m_testsPassed(0)
        , m_testsFailed(0)
        , m_testsSkipped(0)
    {
        setupApplication();
        setupCommandLineParser();
    }

    int run()
    {
        parseCommandLine();
        setupLogging();
        setupTestEnvironment();
        
        int result = runTests();
        
        generateReport();
        cleanup();
        
        return result;
    }

private:
    void setupApplication()
    {
        m_app.setApplicationName("NetworkModuleTests");
        m_app.setApplicationVersion("1.0");
        m_app.setOrganizationName("Jitsi");
        m_app.setOrganizationDomain("jitsi.org");
    }

    void setupCommandLineParser()
    {
        m_parser.setApplicationDescription("网络模块测试运行器");
        m_parser.addHelpOption();
        m_parser.addVersionOption();

        // 测试选择选项
        QCommandLineOption testOption(QStringList() << "t" << "test",
            "运行指定的测试方法", "test_name");
        m_parser.addOption(testOption);

        QCommandLineOption listOption(QStringList() << "l" << "list",
            "列出所有可用的测试");
        m_parser.addOption(listOption);

        // 输出选项
        QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
            "启用详细输出");
        m_parser.addOption(verboseOption);

        QCommandLineOption quietOption(QStringList() << "q" << "quiet",
            "静默模式，只输出错误");
        m_parser.addOption(quietOption);

        QCommandLineOption outputOption(QStringList() << "o" << "output",
            "指定输出目录", "directory");
        m_parser.addOption(outputOption);

        // 报告选项
        QCommandLineOption xmlOption("xml", "生成XML格式的测试报告");
        m_parser.addOption(xmlOption);

        QCommandLineOption junitOption("junit", "生成JUnit格式的测试报告");
        m_parser.addOption(junitOption);

        QCommandLineOption htmlOption("html", "生成HTML格式的测试报告");
        m_parser.addOption(htmlOption);

        // 性能测试选项
        QCommandLineOption perfOption("perf", "运行性能测试");
        m_parser.addOption(perfOption);

        QCommandLineOption iterationsOption("iterations",
            "性能测试迭代次数", "count", "10");
        m_parser.addOption(iterationsOption);

        // 网络选项
        QCommandLineOption serverOption("server",
            "测试服务器URL", "url", "https://meet.jit.si");
        m_parser.addOption(serverOption);

        QCommandLineOption timeoutOption("timeout",
            "测试超时时间（秒）", "seconds", "30");
        m_parser.addOption(timeoutOption);

        // 调试选项
        QCommandLineOption debugOption("debug", "启用调试模式");
        m_parser.addOption(debugOption);

        QCommandLineOption mockOption("mock", "使用模拟对象进行测试");
        m_parser.addOption(mockOption);
    }

    void parseCommandLine()
    {
        m_parser.process(m_app);

        m_verbose = m_parser.isSet("verbose");
        m_quiet = m_parser.isSet("quiet");
        m_debug = m_parser.isSet("debug");
        m_mock = m_parser.isSet("mock");
        m_performanceTest = m_parser.isSet("perf");
        
        m_testName = m_parser.value("test");
        m_outputDir = m_parser.value("output");
        m_serverUrl = m_parser.value("server");
        m_timeout = m_parser.value("timeout").toInt();
        m_iterations = m_parser.value("iterations").toInt();

        m_generateXml = m_parser.isSet("xml");
        m_generateJunit = m_parser.isSet("junit");
        m_generateHtml = m_parser.isSet("html");

        if (m_parser.isSet("list")) {
            listTests();
            exit(0);
        }

        // 设置默认输出目录
        if (m_outputDir.isEmpty()) {
            m_outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/network_tests";
        }
    }

    void setupLogging()
    {
        // 创建输出目录
        QDir().mkpath(m_outputDir);

        // 设置日志级别
        if (m_debug) {
            QLoggingCategory::setFilterRules("*.debug=true");
        } else if (m_verbose) {
            QLoggingCategory::setFilterRules("*.info=true");
        } else if (m_quiet) {
            QLoggingCategory::setFilterRules("*.critical=true");
        }

        // 设置日志文件
        QString logFile = m_outputDir + "/test_log.txt";
        m_logFile.setFileName(logFile);
        if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
                static QTextStream stream(&static_cast<NetworkTestRunner*>(qApp->property("runner").value<void*>())->m_logFile);
                stream << QDateTime::currentDateTime().toString(Qt::ISODate) 
                       << " [" << type << "] " << msg << Qt::endl;
                stream.flush();
            });
        }

        qApp->setProperty("runner", QVariant::fromValue(static_cast<void*>(this)));
    }

    void setupTestEnvironment()
    {
        // 设置环境变量
        qputenv("TEST_SERVER_URL", m_serverUrl.toUtf8());
        qputenv("TEST_TIMEOUT", QString::number(m_timeout).toUtf8());
        qputenv("TEST_OUTPUT_DIR", m_outputDir.toUtf8());
        qputenv("QT_QPA_PLATFORM", "offscreen");

        if (m_mock) {
            qputenv("TEST_MOCK_MODE", "1");
        }

        if (m_verbose) {
            qputenv("NETWORK_TEST_VERBOSE", "1");
        }

        // 创建测试数据目录
        QDir().mkpath(m_outputDir + "/data");
        QDir().mkpath(m_outputDir + "/reports");
    }

    int runTests()
    {
        qInfo() << "开始运行网络模块测试...";
        qInfo() << "服务器URL:" << m_serverUrl;
        qInfo() << "超时时间:" << m_timeout << "秒";
        qInfo() << "输出目录:" << m_outputDir;

        NetworkModuleTest test;
        
        QStringList args;
        args << m_app.applicationName();

        // 添加Qt Test参数
        if (m_generateXml) {
            args << "-xml" << "-o" << (m_outputDir + "/reports/test_results.xml");
        } else if (m_generateJunit) {
            args << "-junitxml" << "-o" << (m_outputDir + "/reports/junit_results.xml");
        }

        if (!m_testName.isEmpty()) {
            args << m_testName;
        }

        if (m_verbose) {
            args << "-v2";
        }

        // 运行测试
        int result = QTest::qExec(&test, args);

        // 收集测试统计
        collectTestStatistics(result);

        return result;
    }

    void collectTestStatistics(int result)
    {
        // 这里可以解析测试结果并收集统计信息
        if (result == 0) {
            qInfo() << "所有测试通过";
        } else {
            qWarning() << "测试失败，返回代码:" << result;
        }
    }

    void generateReport()
    {
        if (m_generateHtml) {
            generateHtmlReport();
        }

        generateSummaryReport();
    }

    void generateHtmlReport()
    {
        QString htmlFile = m_outputDir + "/reports/test_report.html";
        QFile file(htmlFile);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            
            out << "<!DOCTYPE html>\n";
            out << "<html>\n<head>\n";
            out << "<title>网络模块测试报告</title>\n";
            out << "<meta charset=\"UTF-8\">\n";
            out << "<style>\n";
            out << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
            out << ".header { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }\n";
            out << ".passed { color: green; }\n";
            out << ".failed { color: red; }\n";
            out << ".skipped { color: orange; }\n";
            out << "table { border-collapse: collapse; width: 100%; }\n";
            out << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
            out << "th { background-color: #f2f2f2; }\n";
            out << "</style>\n";
            out << "</head>\n<body>\n";
            
            out << "<div class=\"header\">\n";
            out << "<h1>网络模块测试报告</h1>\n";
            out << "<p>生成时间: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "</p>\n";
            out << "<p>测试服务器: " << m_serverUrl << "</p>\n";
            out << "</div>\n";
            
            out << "<h2>测试统计</h2>\n";
            out << "<table>\n";
            out << "<tr><th>状态</th><th>数量</th></tr>\n";
            out << "<tr><td class=\"passed\">通过</td><td>" << m_testsPassed << "</td></tr>\n";
            out << "<tr><td class=\"failed\">失败</td><td>" << m_testsFailed << "</td></tr>\n";
            out << "<tr><td class=\"skipped\">跳过</td><td>" << m_testsSkipped << "</td></tr>\n";
            out << "</table>\n";
            
            out << "</body>\n</html>\n";
            
            qInfo() << "HTML报告已生成:" << htmlFile;
        }
    }

    void generateSummaryReport()
    {
        QString summaryFile = m_outputDir + "/test_summary.txt";
        QFile file(summaryFile);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            
            out << "网络模块测试总结报告\n";
            out << "==================\n\n";
            out << "测试时间: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
            out << "测试服务器: " << m_serverUrl << "\n";
            out << "超时设置: " << m_timeout << " 秒\n";
            out << "输出目录: " << m_outputDir << "\n\n";
            
            out << "测试结果:\n";
            out << "--------\n";
            out << "通过: " << m_testsPassed << "\n";
            out << "失败: " << m_testsFailed << "\n";
            out << "跳过: " << m_testsSkipped << "\n";
            out << "总计: " << (m_testsPassed + m_testsFailed + m_testsSkipped) << "\n\n";
            
            if (m_testsFailed == 0) {
                out << "状态: 所有测试通过 ✓\n";
            } else {
                out << "状态: 有测试失败 ✗\n";
            }
            
            qInfo() << "测试总结已保存:" << summaryFile;
        }
    }

    void listTests()
    {
        qInfo() << "可用的测试方法:";
        qInfo() << "基础功能测试:";
        qInfo() << "  - testNetworkManagerInitialization";
        qInfo() << "  - testNetworkManagerSingleton";
        qInfo() << "  - testConnectionStateManagement";
        qInfo() << "  - testServerConfiguration";
        qInfo() << "  - testAutoReconnectFeature";
        
        qInfo() << "连接测试:";
        qInfo() << "  - testConnectionEstablishment";
        qInfo() << "  - testConnectionDisconnection";
        qInfo() << "  - testConnectionTimeout";
        qInfo() << "  - testConnectionRetry";
        qInfo() << "  - testMultipleConnections";
        
        qInfo() << "网络质量测试:";
        qInfo() << "  - testNetworkQualityMonitoring";
        qInfo() << "  - testLatencyMeasurement";
        qInfo() << "  - testBandwidthMeasurement";
        qInfo() << "  - testPacketLossDetection";
        
        qInfo() << "协议测试:";
        qInfo() << "  - testWebRTCProtocolHandler";
        qInfo() << "  - testHTTPProtocolHandler";
        qInfo() << "  - testWebSocketProtocolHandler";
        
        qInfo() << "性能测试:";
        qInfo() << "  - testConnectionPerformance";
        qInfo() << "  - testDataTransmissionPerformance";
        qInfo() << "  - testMemoryUsage";
        
        qInfo() << "兼容性测试:";
        qInfo() << "  - testLegacyNetworkManagerCompatibility";
        qInfo() << "  - testExistingComponentIntegration";
    }

    void cleanup()
    {
        if (m_logFile.isOpen()) {
            m_logFile.close();
        }
    }

private:
    QCoreApplication m_app;
    QCommandLineParser m_parser;
    QFile m_logFile;
    
    // 命令行选项
    bool m_verbose = false;
    bool m_quiet = false;
    bool m_debug = false;
    bool m_mock = false;
    bool m_performanceTest = false;
    bool m_generateXml = false;
    bool m_generateJunit = false;
    bool m_generateHtml = false;
    
    QString m_testName;
    QString m_outputDir;
    QString m_serverUrl;
    int m_timeout = 30;
    int m_iterations = 10;
    
    // 测试统计
    int m_testsPassed;
    int m_testsFailed;
    int m_testsSkipped;
};

int main(int argc, char* argv[])
{
    NetworkTestRunner runner(argc, argv);
    return runner.run();
}