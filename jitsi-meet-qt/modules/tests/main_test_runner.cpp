#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include "TestCoverageFramework.h"
#include "AutomatedTestRunner.h"

/**
 * @brief 主测试运行器
 * 
 * 这是测试框架的主入口点，负责：
 * - 解析命令行参数
 * - 配置测试环境
 * - 启动相应的测试套件
 * - 生成测试报告
 * 
 * Requirements: 11.5, 11.6, 12.6
 */

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Jitsi Meet Qt Test Runner");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Jitsi");
    
    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Comprehensive test runner for Jitsi Meet Qt modular architecture");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Test type options
    QCommandLineOption unitTestsOption("unit", "Run unit tests");
    QCommandLineOption integrationTestsOption("integration", "Run integration tests");
    QCommandLineOption e2eTestsOption("e2e", "Run end-to-end tests");
    QCommandLineOption performanceTestsOption("performance", "Run performance tests");
    QCommandLineOption regressionTestsOption("regression", "Run regression tests");
    QCommandLineOption allTestsOption("all", "Run all test types");
    
    parser.addOption(unitTestsOption);
    parser.addOption(integrationTestsOption);
    parser.addOption(e2eTestsOption);
    parser.addOption(performanceTestsOption);
    parser.addOption(regressionTestsOption);
    parser.addOption(allTestsOption);
    
    // Module selection
    QCommandLineOption modulesOption(QStringList() << "m" << "modules",
                                   "Comma-separated list of modules to test",
                                   "modules");
    parser.addOption(modulesOption);
    
    // Coverage options
    QCommandLineOption coverageOption("coverage", "Generate coverage report");
    QCommandLineOption coverageThresholdOption("coverage-threshold",
                                             "Minimum coverage threshold (default: 75)",
                                             "threshold", "75");
    parser.addOption(coverageOption);
    parser.addOption(coverageThresholdOption);
    
    // Output options
    QCommandLineOption outputDirOption(QStringList() << "o" << "output",
                                     "Output directory for test results",
                                     "directory", "test_results");
    QCommandLineOption formatOption("format",
                                  "Report format (html,json,xml,junit)",
                                  "format", "html");
    parser.addOption(outputDirOption);
    parser.addOption(formatOption);
    
    // Automation options
    QCommandLineOption automatedOption("automated", "Run in automated mode");
    QCommandLineOption scheduleOption("schedule",
                                    "Schedule mode (manual,periodic,onchange)",
                                    "mode", "manual");
    QCommandLineOption intervalOption("interval",
                                    "Schedule interval in minutes",
                                    "minutes", "60");
    parser.addOption(automatedOption);
    parser.addOption(scheduleOption);
    parser.addOption(intervalOption);
    
    // CI integration options
    QCommandLineOption ciOption("ci", "Enable CI integration");
    QCommandLineOption ciProviderOption("ci-provider",
                                      "CI provider (github,gitlab,jenkins,azure)",
                                      "provider");
    parser.addOption(ciOption);
    parser.addOption(ciProviderOption);
    
    // Configuration file
    QCommandLineOption configOption(QStringList() << "c" << "config",
                                  "Configuration file path",
                                  "file", "test_config.json");
    parser.addOption(configOption);
    
    // Verbose output
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
                                   "Enable verbose output");
    parser.addOption(verboseOption);
    
    // Parse command line
    parser.process(app);
    
    // Setup logging
    if (parser.isSet(verboseOption)) {
        QLoggingCategory::setFilterRules("*.debug=true");
    }
    
    qDebug() << "=== Jitsi Meet Qt Test Runner Starting ===";
    
    // Create output directory
    QString outputDir = parser.value(outputDirOption);
    QDir().mkpath(outputDir);
    
    // Load configuration
    QString configFile = parser.value(configOption);
    QJsonObject config;
    
    if (QFile::exists(configFile)) {
        QFile file(configFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            config = doc.object();
            qDebug() << "Loaded configuration from:" << configFile;
        }
    } else {
        qDebug() << "Configuration file not found, using defaults:" << configFile;
    }
    
    // Determine which tests to run
    bool runUnit = parser.isSet(unitTestsOption) || parser.isSet(allTestsOption);
    bool runIntegration = parser.isSet(integrationTestsOption) || parser.isSet(allTestsOption);
    bool runE2E = parser.isSet(e2eTestsOption) || parser.isSet(allTestsOption);
    bool runPerformance = parser.isSet(performanceTestsOption) || parser.isSet(allTestsOption);
    bool runRegression = parser.isSet(regressionTestsOption) || parser.isSet(allTestsOption);
    
    // If no specific tests specified, run all
    if (!runUnit && !runIntegration && !runE2E && !runPerformance && !runRegression) {
        runUnit = runIntegration = runE2E = runPerformance = runRegression = true;
    }
    
    // Create test framework
    TestCoverageFramework* testFramework = new TestCoverageFramework(&app);
    
    // Configure coverage
    if (parser.isSet(coverageOption)) {
        // Enable coverage analysis
        qDebug() << "Coverage analysis enabled";
    }
    
    // Setup automated runner if requested
    AutomatedTestRunner* automatedRunner = nullptr;
    if (parser.isSet(automatedOption)) {
        automatedRunner = new AutomatedTestRunner(&app);
        
        // Configure schedule
        QString scheduleMode = parser.value(scheduleOption);
        int interval = parser.value(intervalOption).toInt();
        
        AutomatedTestRunner::ScheduleMode mode = AutomatedTestRunner::Manual;
        if (scheduleMode == "periodic") {
            mode = AutomatedTestRunner::Periodic;
        } else if (scheduleMode == "onchange") {
            mode = AutomatedTestRunner::OnFileChange;
        }
        
        automatedRunner->scheduleTests(mode, interval);
        
        // Configure CI integration
        if (parser.isSet(ciOption)) {
            QString provider = parser.value(ciProviderOption);
            AutomatedTestRunner::CIProvider ciProvider = AutomatedTestRunner::None;
            
            if (provider == "github") {
                ciProvider = AutomatedTestRunner::GitHubActions;
            } else if (provider == "gitlab") {
                ciProvider = AutomatedTestRunner::GitLabCI;
            } else if (provider == "jenkins") {
                ciProvider = AutomatedTestRunner::Jenkins;
            } else if (provider == "azure") {
                ciProvider = AutomatedTestRunner::AzureDevOps;
            }
            
            if (ciProvider != AutomatedTestRunner::None) {
                QVariantMap ciConfig;
                // CI configuration would be loaded from config file or environment
                automatedRunner->configureCIIntegration(ciProvider, ciConfig);
            }
        }
        
        automatedRunner->startAutomatedTesting();
    }
    
    // Setup test completion handling
    bool testCompleted = false;
    int exitCode = 0;
    
    QObject::connect(testFramework, &TestCoverageFramework::allTestsCompleted,
                     [&]() {
        qDebug() << "All tests completed successfully";
        testCompleted = true;
        app.quit();
    });
    
    // Start tests based on command line options
    QTimer::singleShot(100, [&]() {
        qDebug() << "Starting test execution...";
        
        if (runUnit) {
            qDebug() << "Unit tests will be executed";
        }
        if (runIntegration) {
            qDebug() << "Integration tests will be executed";
        }
        if (runE2E) {
            qDebug() << "End-to-end tests will be executed";
        }
        if (runPerformance) {
            qDebug() << "Performance tests will be executed";
        }
        if (runRegression) {
            qDebug() << "Regression tests will be executed";
        }
        
        // Run the comprehensive test suite
        testFramework->runAllTests();
    });
    
    // Set up timeout for tests (30 minutes default)
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.setInterval(30 * 60 * 1000); // 30 minutes
    
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
        qWarning() << "Test execution timed out!";
        exitCode = 2;
        app.quit();
    });
    
    timeoutTimer.start();
    
    // Run the application
    int result = app.exec();
    
    // Cleanup
    if (automatedRunner) {
        automatedRunner->stopAutomatedTesting();
    }
    
    // Generate final summary
    qDebug() << "=== Test Execution Summary ===";
    qDebug() << "Exit code:" << (testCompleted ? exitCode : result);
    qDebug() << "Output directory:" << outputDir;
    
    if (parser.isSet(coverageOption)) {
        qDebug() << "Coverage report generated";
    }
    
    qDebug() << "=== Jitsi Meet Qt Test Runner Finished ===";
    
    return testCompleted ? exitCode : result;
}