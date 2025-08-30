#include "../config/UtilsConfig.h"
#include "../include/UtilsSingletonManager.h"
#include "../include/UtilsErrorHandler.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

/**
 * @brief 工具模块配置使用示例
 * 
 * 本示例演示如何使用UtilsConfig进行配置管理，
 * 包括配置的加载、保存、验证和监听变更。
 */
class UtilsConfigExample : public QObject
{
    Q_OBJECT

public:
    explicit UtilsConfigExample(QObject* parent = nullptr) : QObject(parent) {}

    void runExample()
    {
        qDebug() << "=== Utils Configuration Example ===";
        
        // 1. 获取配置管理器实例
        UtilsConfig* config = UtilsConfig::instance();
        
        // 连接配置变更信号
        connect(config, &UtilsConfig::configurationChanged,
                this, &UtilsConfigExample::onConfigurationChanged);
        connect(config, &UtilsConfig::configurationError,
                this, &UtilsConfigExample::onConfigurationError);
        
        // 2. 初始化配置系统
        if (!config->initialize()) {
            qCritical() << "Failed to initialize configuration system";
            return;
        }
        
        qDebug() << "Configuration file path:" << config->configFilePath();
        
        // 3. 读取配置值
        demonstrateConfigReading(config);
        
        // 4. 修改配置值
        demonstrateConfigWriting(config);
        
        // 5. 配置验证
        demonstrateConfigValidation(config);
        
        // 6. 导入导出配置
        demonstrateImportExport(config);
        
        // 7. 重置配置
        demonstrateConfigReset(config);
        
        qDebug() << "=== Configuration Example Complete ===";
    }

private slots:
    void onConfigurationChanged(const QString& key, const QVariant& value)
    {
        qDebug() << "Configuration changed:" << key << "=" << value;
    }
    
    void onConfigurationError(const QString& error)
    {
        qWarning() << "Configuration error:" << error;
    }

private:
    void demonstrateConfigReading(UtilsConfig* config)
    {
        qDebug() << "\n--- Reading Configuration ---";
        
        // 使用枚举键读取
        QString version = config->getValue(UtilsConfig::ModuleVersion).toString();
        bool debugMode = config->getValue(UtilsConfig::DebugMode).toBool();
        QString logLevel = config->getValue(UtilsConfig::LogLevel).toString();
        
        qDebug() << "Module version:" << version;
        qDebug() << "Debug mode:" << debugMode;
        qDebug() << "Log level:" << logLevel;
        
        // 使用字符串键读取
        bool fileLogging = config->getValue("logging/enableFile").toBool();
        QString tempDir = config->getValue("filesystem/tempDirectory").toString();
        
        qDebug() << "File logging enabled:" << fileLogging;
        qDebug() << "Temp directory:" << tempDir;
        
        // 获取所有配置
        QVariantMap allConfig = config->getAllConfiguration();
        qDebug() << "Total configuration items:" << allConfig.size();
    }
    
    void demonstrateConfigWriting(UtilsConfig* config)
    {
        qDebug() << "\n--- Writing Configuration ---";
        
        // 使用枚举键设置
        config->setValue(UtilsConfig::DebugMode, true);
        config->setValue(UtilsConfig::LogLevel, "Debug");
        config->setValue(UtilsConfig::MaxConcurrentOperations, 20);
        
        // 使用字符串键设置
        config->setValue("logging/enableConsole", true);
        config->setValue("performance/enableMonitoring", true);
        
        qDebug() << "Configuration values updated";
        
        // 保存配置到文件
        if (config->saveConfiguration()) {
            qDebug() << "Configuration saved successfully";
        } else {
            qWarning() << "Failed to save configuration";
        }
    }
    
    void demonstrateConfigValidation(UtilsConfig* config)
    {
        qDebug() << "\n--- Configuration Validation ---";
        
        // 验证当前配置
        bool isValid = config->validateConfiguration();
        qDebug() << "Current configuration is valid:" << isValid;
        
        // 测试无效配置
        config->setValue(UtilsConfig::LogLevel, "InvalidLevel");
        isValid = config->validateConfiguration();
        qDebug() << "Configuration with invalid log level is valid:" << isValid;
        
        // 恢复有效值
        config->setValue(UtilsConfig::LogLevel, "Info");
    }
    
    void demonstrateImportExport(UtilsConfig* config)
    {
        qDebug() << "\n--- Import/Export Configuration ---";
        
        // 导出为JSON
        QJsonObject jsonConfig = config->exportToJson();
        qDebug() << "Exported configuration to JSON, items:" << jsonConfig.size();
        
        // 修改一些值
        config->setValue(UtilsConfig::DebugMode, false);
        config->setValue(UtilsConfig::LogLevel, "Warning");
        
        qDebug() << "Modified configuration values";
        
        // 从JSON导入
        if (config->importFromJson(jsonConfig)) {
            qDebug() << "Configuration imported from JSON successfully";
        } else {
            qWarning() << "Failed to import configuration from JSON";
        }
        
        // 验证恢复的值
        bool debugMode = config->getValue(UtilsConfig::DebugMode).toBool();
        QString logLevel = config->getValue(UtilsConfig::LogLevel).toString();
        qDebug() << "Restored debug mode:" << debugMode;
        qDebug() << "Restored log level:" << logLevel;
    }
    
    void demonstrateConfigReset(UtilsConfig* config)
    {
        qDebug() << "\n--- Configuration Reset ---";
        
        // 显示当前修改状态
        qDebug() << "Configuration is modified:" << config->isModified();
        
        // 重置为默认值
        config->resetToDefaults();
        qDebug() << "Configuration reset to defaults";
        
        // 验证默认值
        QString version = config->getValue(UtilsConfig::ModuleVersion).toString();
        bool debugMode = config->getValue(UtilsConfig::DebugMode).toBool();
        qDebug() << "Default version:" << version;
        qDebug() << "Default debug mode:" << debugMode;
    }
};

/**
 * @brief 单例管理器使用示例
 */
class SingletonManagerExample : public QObject
{
    Q_OBJECT

public:
    explicit SingletonManagerExample(QObject* parent = nullptr) : QObject(parent) {}

    void runExample()
    {
        qDebug() << "\n=== Singleton Manager Example ===";
        
        // 1. 获取单例管理器
        UtilsSingletonManager* manager = UtilsSingletonManager::instance();
        
        // 连接信号
        connect(manager, &UtilsSingletonManager::singletonInitialized,
                this, &SingletonManagerExample::onSingletonInitialized);
        connect(manager, &UtilsSingletonManager::allSingletonsInitialized,
                this, &SingletonManagerExample::onAllSingletonsInitialized);
        connect(manager, &UtilsSingletonManager::singletonError,
                this, &SingletonManagerExample::onSingletonError);
        
        // 2. 初始化所有单例
        if (manager->initializeAll()) {
            qDebug() << "All singletons initialized successfully";
        } else {
            qWarning() << "Some singletons failed to initialize";
        }
        
        // 3. 检查单例状态
        demonstrateSingletonStatus(manager);
        
        // 4. 获取单例实例
        demonstrateSingletonAccess(manager);
        
        // 5. 管理单例生命周期
        demonstrateSingletonLifecycle(manager);
        
        qDebug() << "=== Singleton Manager Example Complete ===";
    }

private slots:
    void onSingletonInitialized(UtilsSingletonManager::SingletonType type)
    {
        qDebug() << "Singleton initialized:" << UtilsSingletonManager::singletonTypeToString(type);
    }
    
    void onAllSingletonsInitialized()
    {
        qDebug() << "All singletons have been initialized";
    }
    
    void onSingletonError(UtilsSingletonManager::SingletonType type, const QString& error)
    {
        qWarning() << "Singleton error:" << UtilsSingletonManager::singletonTypeToString(type) << error;
    }

private:
    void demonstrateSingletonStatus(UtilsSingletonManager* manager)
    {
        qDebug() << "\n--- Singleton Status ---";
        
        // 获取所有单例状态
        QVariantMap allStatus = manager->getAllSingletonStatus();
        for (auto it = allStatus.constBegin(); it != allStatus.constEnd(); ++it) {
            QVariantMap status = it.value().toMap();
            qDebug() << QString("Singleton %1: enabled=%2, initialized=%3")
                       .arg(it.key())
                       .arg(status["enabled"].toBool())
                       .arg(status["initialized"].toBool());
        }
        
        // 获取已初始化的单例列表
        auto initialized = manager->getInitializedSingletons();
        qDebug() << "Initialized singletons count:" << initialized.size();
    }
    
    void demonstrateSingletonAccess(UtilsSingletonManager* manager)
    {
        qDebug() << "\n--- Singleton Access ---";
        
        // 获取Logger实例
        Logger* logger = manager->getLogger();
        if (logger) {
            qDebug() << "Logger singleton obtained successfully";
            logger->info("Test message from singleton manager example");
        }
        
        // 获取FileManager实例
        FileManager* fileManager = manager->getFileManager();
        if (fileManager) {
            qDebug() << "FileManager singleton obtained successfully";
        }
        
        // 获取Config实例
        UtilsConfig* config = manager->getConfig();
        if (config) {
            qDebug() << "UtilsConfig singleton obtained successfully";
            qDebug() << "Config version:" << config->getValue(UtilsConfig::ModuleVersion).toString();
        }
    }
    
    void demonstrateSingletonLifecycle(UtilsSingletonManager* manager)
    {
        qDebug() << "\n--- Singleton Lifecycle ---";
        
        // 禁用某个单例
        manager->setSingletonEnabled(UtilsSingletonManager::ValidatorSingleton, false);
        qDebug() << "Validator singleton disabled";
        
        // 重新启用
        manager->setSingletonEnabled(UtilsSingletonManager::ValidatorSingleton, true);
        qDebug() << "Validator singleton re-enabled";
        
        // 重新初始化单例
        if (manager->reinitializeSingleton(UtilsSingletonManager::LoggerSingleton)) {
            qDebug() << "Logger singleton reinitialized successfully";
        }
        
        // 设置单例参数
        QVariantMap params;
        params["customParam"] = "exampleValue";
        manager->setSingletonParameters(UtilsSingletonManager::LoggerSingleton, params);
        qDebug() << "Logger singleton parameters set";
    }
};

/**
 * @brief 错误处理器使用示例
 */
class ErrorHandlerExample : public QObject
{
    Q_OBJECT

public:
    explicit ErrorHandlerExample(QObject* parent = nullptr) : QObject(parent) {}

    void runExample()
    {
        qDebug() << "\n=== Error Handler Example ===";
        
        // 1. 获取错误处理器实例
        UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
        
        // 连接信号
        connect(errorHandler, &UtilsErrorHandler::errorReported,
                this, &ErrorHandlerExample::onErrorReported);
        connect(errorHandler, &UtilsErrorHandler::criticalErrorOccurred,
                this, &ErrorHandlerExample::onCriticalError);
        
        // 2. 初始化错误处理器
        if (!errorHandler->initialize()) {
            qCritical() << "Failed to initialize error handler";
            return;
        }
        
        // 3. 报告各种类型的错误
        demonstrateErrorReporting(errorHandler);
        
        // 4. 错误查询和统计
        demonstrateErrorQuerying(errorHandler);
        
        // 5. 错误恢复
        demonstrateErrorRecovery(errorHandler);
        
        // 6. 错误导出
        demonstrateErrorExport(errorHandler);
        
        qDebug() << "=== Error Handler Example Complete ===";
    }

private slots:
    void onErrorReported(const UtilsErrorHandler::ErrorInfo& errorInfo)
    {
        qDebug() << QString("Error reported: [%1] %2 - %3")
                   .arg(UtilsErrorHandler::errorLevelToString(errorInfo.level))
                   .arg(errorInfo.source)
                   .arg(errorInfo.message);
    }
    
    void onCriticalError(const UtilsErrorHandler::ErrorInfo& errorInfo)
    {
        qCritical() << "Critical error occurred:" << errorInfo.message;
    }

private:
    void demonstrateErrorReporting(UtilsErrorHandler* errorHandler)
    {
        qDebug() << "\n--- Error Reporting ---";
        
        // 报告不同级别的错误
        QString infoId = errorHandler->reportInfo("Application started", "Main");
        QString warningId = errorHandler->reportWarning("Low disk space", "FileSystem");
        QString errorId = errorHandler->reportError("Failed to connect to server", "Network");
        
        // 报告详细错误
        QVariantMap context;
        context["serverUrl"] = "https://example.com";
        context["timeout"] = 5000;
        context["retryCount"] = 3;
        
        QString detailedErrorId = errorHandler->reportError(
            UtilsErrorHandler::Error,
            UtilsErrorHandler::NetworkError,
            "Connection timeout",
            "NetworkManager",
            "Failed to establish connection within timeout period",
            context
        );
        
        qDebug() << "Reported errors with IDs:" << infoId << warningId << errorId << detailedErrorId;
    }
    
    void demonstrateErrorQuerying(UtilsErrorHandler* errorHandler)
    {
        qDebug() << "\n--- Error Querying ---";
        
        // 获取所有错误
        auto allErrors = errorHandler->getAllErrors();
        qDebug() << "Total errors:" << allErrors.size();
        
        // 按级别查询
        auto warnings = errorHandler->getErrorsByLevel(UtilsErrorHandler::Warning);
        auto errors = errorHandler->getErrorsByLevel(UtilsErrorHandler::Error);
        qDebug() << "Warnings:" << warnings.size() << "Errors:" << errors.size();
        
        // 按类别查询
        auto networkErrors = errorHandler->getErrorsByCategory(UtilsErrorHandler::NetworkError);
        qDebug() << "Network errors:" << networkErrors.size();
        
        // 获取统计信息
        QVariantMap stats = errorHandler->getErrorStatistics();
        qDebug() << "Error statistics:" << stats;
    }
    
    void demonstrateErrorRecovery(UtilsErrorHandler* errorHandler)
    {
        qDebug() << "\n--- Error Recovery ---";
        
        // 获取未恢复的错误
        auto unrecovered = errorHandler->getUnrecoveredErrors();
        qDebug() << "Unrecovered errors:" << unrecovered.size();
        
        if (!unrecovered.isEmpty()) {
            // 尝试自动恢复第一个错误
            const auto& error = unrecovered.first();
            qDebug() << "Attempting auto recovery for error:" << error.id;
            
            if (errorHandler->attemptAutoRecovery(error.id)) {
                qDebug() << "Auto recovery successful";
            } else {
                qDebug() << "Auto recovery failed, marking as manually recovered";
                errorHandler->markErrorRecovered(error.id, "Manual intervention");
            }
        }
        
        // 设置恢复策略
        errorHandler->setRecoveryStrategy(UtilsErrorHandler::NetworkError, UtilsErrorHandler::Retry);
        errorHandler->setRecoveryStrategy(UtilsErrorHandler::FileSystemError, UtilsErrorHandler::Fallback);
        
        qDebug() << "Recovery strategies configured";
    }
    
    void demonstrateErrorExport(UtilsErrorHandler* errorHandler)
    {
        qDebug() << "\n--- Error Export ---";
        
        // 导出为不同格式
        QString jsonFile = "error_log.json";
        QString csvFile = "error_log.csv";
        QString txtFile = "error_log.txt";
        
        if (errorHandler->exportErrorLog(jsonFile, "json")) {
            qDebug() << "Error log exported to JSON:" << jsonFile;
        }
        
        if (errorHandler->exportErrorLog(csvFile, "csv")) {
            qDebug() << "Error log exported to CSV:" << csvFile;
        }
        
        if (errorHandler->exportErrorLog(txtFile, "txt")) {
            qDebug() << "Error log exported to TXT:" << txtFile;
        }
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 运行配置示例
    UtilsConfigExample configExample;
    configExample.runExample();
    
    // 运行单例管理器示例
    SingletonManagerExample singletonExample;
    singletonExample.runExample();
    
    // 运行错误处理器示例
    ErrorHandlerExample errorExample;
    errorExample.runExample();
    
    // 延迟退出以查看所有输出
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    
    return app.exec();
}

#include "UtilsConfigExample.moc"