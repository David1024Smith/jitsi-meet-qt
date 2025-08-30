#include "CompatibilityValidator.h"
#include "FunctionValidator.h"
#include "PerformanceValidator.h"

#include <QDebug>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QThread>

CompatibilityValidator::CompatibilityValidator(QObject *parent)
    : ICompatibilityValidator(parent)
    , m_initialized(false)
    , m_testTimeout(30000) // 30秒默认超时
    , m_parallelTestingEnabled(false)
    , m_functionValidator(nullptr)
    , m_performanceValidator(nullptr)
    , m_timeoutTimer(nullptr)
{
    setupDefaultTests();
}

CompatibilityValidator::~CompatibilityValidator()
{
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        m_timeoutTimer->deleteLater();
    }
    
    if (m_functionValidator) {
        m_functionValidator->deleteLater();
    }
    
    if (m_performanceValidator) {
        m_performanceValidator->deleteLater();
    }
}

bool CompatibilityValidator::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing CompatibilityValidator...";

    // 初始化功能验证器
    m_functionValidator = new FunctionValidator(this);
    if (!m_functionValidator->initialize()) {
        qWarning() << "Failed to initialize FunctionValidator";
        return false;
    }

    // 初始化性能验证器
    m_performanceValidator = new PerformanceValidator(this);
    if (!m_performanceValidator->initialize()) {
        qWarning() << "Failed to initialize PerformanceValidator";
        return false;
    }

    // 设置超时定时器
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &CompatibilityValidator::onTestTimeout);

    // 设置默认配置
    m_config["strict_mode"] = false;
    m_config["performance_threshold"] = 0.8;
    m_config["max_test_duration"] = 30000;
    m_config["parallel_tests"] = false;

    m_initialized = true;
    qDebug() << "CompatibilityValidator initialized successfully";
    qDebug() << "Available tests:" << m_tests.size();
    
    return true;
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateFunctionality(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "CompatibilityValidator not initialized";
        return Failed;
    }

    qDebug() << "Validating functionality for module:" << moduleName;
    
    emit validationStarted(moduleName);
    
    QElapsedTimer timer;
    timer.start();
    
    ValidationResult result = Failed;
    QString message;
    
    try {
        if (moduleName == "audio") {
            result = validateAudioModule();
        } else if (moduleName == "network") {
            result = validateNetworkModule();
        } else if (moduleName == "ui") {
            result = validateUIModule();
        } else if (moduleName == "chat") {
            result = validateChatModule();
        } else if (moduleName == "screenshare") {
            result = validateScreenShareModule();
        } else if (moduleName == "meeting") {
            result = validateMeetingModule();
        } else if (moduleName == "performance") {
            result = validatePerformanceModule();
        } else if (moduleName == "settings") {
            result = validateSettingsModule();
        } else if (moduleName == "utils") {
            result = validateUtilsModule();
        } else {
            message = QString("Unknown module: %1").arg(moduleName);
            result = Failed;
        }
    } catch (const std::exception& e) {
        message = QString("Exception during validation: %1").arg(e.what());
        result = Failed;
    } catch (...) {
        message = "Unknown exception during validation";
        result = Failed;
    }
    
    double executionTime = timer.elapsed();
    
    if (message.isEmpty()) {
        switch (result) {
        case Passed:
            message = QString("Module %1 validation passed").arg(moduleName);
            break;
        case Warning:
            message = QString("Module %1 validation passed with warnings").arg(moduleName);
            break;
        case Failed:
            message = QString("Module %1 validation failed").arg(moduleName);
            break;
        case Skipped:
            message = QString("Module %1 validation skipped").arg(moduleName);
            break;
        }
    }
    
    // 创建验证报告
    ValidationReport report = createReport(moduleName, result, message, executionTime);
    m_reports.append(report);
    
    emit validationCompleted(moduleName, result);
    
    qDebug() << "Validation completed for module:" << moduleName 
             << "Result:" << result << "Time:" << executionTime << "ms";
    
    return result;
}Q
StringList CompatibilityValidator::runCompatibilityTests()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return QStringList() << "CompatibilityValidator not initialized";
    }

    qDebug() << "Running compatibility tests...";
    
    clearReports();
    QStringList results;
    
    emit progressUpdated(0);
    
    int totalTests = m_tests.size();
    int completedTests = 0;
    
    for (auto it = m_tests.begin(); it != m_tests.end(); ++it) {
        const TestInfo& testInfo = it.value();
        
        if (!testInfo.enabled) {
            results.append(QString("SKIPPED: %1 - Test disabled").arg(testInfo.name));
            continue;
        }
        
        emit validationStarted(testInfo.name);
        
        QElapsedTimer timer;
        timer.start();
        
        ValidationResult result = Failed;
        QString message;
        
        try {
            if (runSingleTest(testInfo)) {
                result = testInfo.function();
            } else {
                result = Failed;
                message = "Test execution failed";
            }
        } catch (const std::exception& e) {
            result = Failed;
            message = QString("Exception: %1").arg(e.what());
        } catch (...) {
            result = Failed;
            message = "Unknown exception";
        }
        
        double executionTime = timer.elapsed();
        
        QString resultStr;
        switch (result) {
        case Passed:
            resultStr = "PASSED";
            break;
        case Warning:
            resultStr = "WARNING";
            break;
        case Failed:
            resultStr = "FAILED";
            break;
        case Skipped:
            resultStr = "SKIPPED";
            break;
        }
        
        QString testResult = QString("%1: %2").arg(resultStr, testInfo.name);
        if (!message.isEmpty()) {
            testResult += QString(" - %1").arg(message);
        }
        testResult += QString(" (%.2f ms)").arg(executionTime);
        
        results.append(testResult);
        
        // 创建详细报告
        ValidationReport report = createReport(testInfo.name, result, message, executionTime);
        report.details["module"] = testInfo.module;
        report.details["timeout"] = testInfo.timeout;
        m_reports.append(report);
        
        emit validationCompleted(testInfo.name, result);
        
        completedTests++;
        int progress = (completedTests * 100) / totalTests;
        emit progressUpdated(progress);
    }
    
    emit progressUpdated(100);
    
    qDebug() << "Compatibility tests completed. Results:" << results.size();
    return results;
}

QList<ICompatibilityValidator::ValidationReport> CompatibilityValidator::getDetailedReport() const
{
    QMutexLocker locker(&m_mutex);
    return m_reports;
}

bool CompatibilityValidator::validatePerformance(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized || !m_performanceValidator) {
        return false;
    }

    return m_performanceValidator->validateModulePerformance(moduleName);
}

void CompatibilityValidator::setValidationConfig(const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    
    m_config = config;
    
    if (config.contains("test_timeout")) {
        m_testTimeout = config["test_timeout"].toInt();
    }
    
    if (config.contains("parallel_tests")) {
        m_parallelTestingEnabled = config["parallel_tests"].toBool();
    }
}

void CompatibilityValidator::addCustomTest(const QString& testName, std::function<ValidationResult()> testFunction)
{
    QMutexLocker locker(&m_mutex);
    
    TestInfo testInfo;
    testInfo.name = testName;
    testInfo.module = "custom";
    testInfo.function = testFunction;
    testInfo.timeout = m_testTimeout;
    testInfo.enabled = true;
    
    m_tests[testName] = testInfo;
    
    qDebug() << "Added custom test:" << testName;
}

void CompatibilityValidator::removeCustomTest(const QString& testName)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_tests.remove(testName) > 0) {
        qDebug() << "Removed custom test:" << testName;
    }
}

QStringList CompatibilityValidator::getAvailableTests() const
{
    QMutexLocker locker(&m_mutex);
    return m_tests.keys();
}

void CompatibilityValidator::setTestTimeout(int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    m_testTimeout = timeoutMs;
}

int CompatibilityValidator::getTestTimeout() const
{
    QMutexLocker locker(&m_mutex);
    return m_testTimeout;
}

void CompatibilityValidator::enableParallelTesting(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_parallelTestingEnabled = enabled;
}

bool CompatibilityValidator::isParallelTestingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_parallelTestingEnabled;
}

void CompatibilityValidator::onTestCompleted(const QString& testName, ValidationResult result)
{
    qDebug() << "Test completed:" << testName << "Result:" << result;
}

void CompatibilityValidator::onTestTimeout()
{
    qWarning() << "Test timeout for:" << m_currentTest;
    emit validationCompleted(m_currentTest, Failed);
}

bool CompatibilityValidator::runSingleTest(const TestInfo& testInfo)
{
    if (!testInfo.function) {
        return false;
    }
    
    m_currentTest = testInfo.name;
    
    if (testInfo.timeout > 0) {
        m_timeoutTimer->start(testInfo.timeout);
    }
    
    return true;
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateAudioModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    // 验证音频模块的基本功能
    QStringList audioTests = {
        "audio_device_enumeration",
        "audio_device_selection",
        "audio_volume_control",
        "audio_mute_control",
        "audio_quality_settings"
    };
    
    int passedTests = 0;
    for (const QString& test : audioTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / audioTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateNetworkModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList networkTests = {
        "network_connection_establishment",
        "network_data_transmission",
        "network_quality_monitoring",
        "network_protocol_handling",
        "network_error_recovery"
    };
    
    int passedTests = 0;
    for (const QString& test : networkTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / networkTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateUIModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList uiTests = {
        "ui_theme_switching",
        "ui_layout_management",
        "ui_widget_rendering",
        "ui_event_handling",
        "ui_responsiveness"
    };
    
    int passedTests = 0;
    for (const QString& test : uiTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / uiTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateChatModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList chatTests = {
        "chat_message_sending",
        "chat_message_receiving",
        "chat_history_management",
        "chat_participant_management",
        "chat_file_sharing"
    };
    
    int passedTests = 0;
    for (const QString& test : chatTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / chatTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateScreenShareModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList screenShareTests = {
        "screenshare_capture_initialization",
        "screenshare_screen_enumeration",
        "screenshare_capture_start_stop",
        "screenshare_quality_adjustment",
        "screenshare_encoding_performance"
    };
    
    int passedTests = 0;
    for (const QString& test : screenShareTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / screenShareTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateMeetingModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList meetingTests = {
        "meeting_link_parsing",
        "meeting_creation",
        "meeting_joining",
        "meeting_authentication",
        "meeting_room_management"
    };
    
    int passedTests = 0;
    for (const QString& test : meetingTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / meetingTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validatePerformanceModule()
{
    if (!m_performanceValidator) {
        return Failed;
    }
    
    return m_performanceValidator->validateModulePerformance("performance") ? Passed : Failed;
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateSettingsModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList settingsTests = {
        "settings_load_save",
        "settings_validation",
        "settings_synchronization",
        "settings_backup_restore",
        "settings_ui_integration"
    };
    
    int passedTests = 0;
    for (const QString& test : settingsTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / settingsTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

ICompatibilityValidator::ValidationResult CompatibilityValidator::validateUtilsModule()
{
    if (!m_functionValidator) {
        return Failed;
    }
    
    QStringList utilsTests = {
        "utils_logging_functionality",
        "utils_file_operations",
        "utils_encryption_decryption",
        "utils_string_processing",
        "utils_configuration_management"
    };
    
    int passedTests = 0;
    for (const QString& test : utilsTests) {
        if (m_functionValidator->runTest(test)) {
            passedTests++;
        }
    }
    
    double passRate = static_cast<double>(passedTests) / utilsTests.size();
    
    if (passRate >= 0.9) {
        return Passed;
    } else if (passRate >= 0.7) {
        return Warning;
    } else {
        return Failed;
    }
}

void CompatibilityValidator::setupDefaultTests()
{
    // 这里可以设置默认的测试用例
    // 实际实现中会根据具体需求添加测试
}

void CompatibilityValidator::clearReports()
{
    m_reports.clear();
}

ICompatibilityValidator::ValidationReport CompatibilityValidator::createReport(const QString& testName, 
                                                                             ValidationResult result, 
                                                                             const QString& message, 
                                                                             double executionTime)
{
    ValidationReport report;
    report.testName = testName;
    report.result = result;
    report.message = message;
    report.executionTime = executionTime;
    report.details["timestamp"] = QDateTime::currentDateTime();
    report.details["validator_version"] = "1.0.0";
    
    return report;
}