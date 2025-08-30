#include "../include/UtilsModule.h"
#include "../include/Logger.h"
#include "../include/FileManager.h"
#include "../include/UtilsSingletonManager.h"
#include "../include/UtilsErrorHandler.h"
#include "../config/UtilsConfig.h"
#include <QCoreApplication>
#include <QDebug>

// 静态成员初始化
UtilsModule* UtilsModule::s_instance = nullptr;
QMutex UtilsModule::s_mutex;

UtilsModule::UtilsModule(QObject* parent)
    : QObject(parent)
    , m_status(NotInitialized)
{
    // 设置默认配置
    m_configuration["version"] = "1.0.0";
    m_configuration["debug"] = false;
    m_configuration["logLevel"] = "Info";
    m_configuration["enableFileLogging"] = true;
    m_configuration["enableConsoleLogging"] = true;
    m_configuration["enableNetworkLogging"] = false;
}

UtilsModule::~UtilsModule()
{
    cleanup();
}

UtilsModule* UtilsModule::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new UtilsModule();
    }
    return s_instance;
}

bool UtilsModule::initialize()
{
    QMutexLocker locker(&s_mutex);
    
    if (m_status == Ready) {
        return true;
    }
    
    setStatus(Initializing);
    
    try {
        // 1. 初始化错误处理器
        UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
        if (!errorHandler->initialize()) {
            setError("Failed to initialize error handler");
            setStatus(Error);
            return false;
        }
        
        // 2. 初始化配置系统
        UtilsConfig* config = UtilsConfig::instance();
        if (!config->initialize()) {
            setError("Failed to initialize configuration system");
            setStatus(Error);
            return false;
        }
        
        // 从配置更新模块配置
        m_configuration = config->getAllConfiguration();
        
        // 3. 初始化单例管理器
        UtilsSingletonManager* singletonManager = UtilsSingletonManager::instance();
        if (!singletonManager->initializeAll()) {
            setError("Failed to initialize singleton manager");
            setStatus(Error);
            return false;
        }
        
        // 4. 初始化各个子系统（通过单例管理器）
        if (!initializeLogging()) {
            setError("Failed to initialize logging system");
            setStatus(Error);
            return false;
        }
        
        if (!initializeFileSystem()) {
            setError("Failed to initialize file system");
            setStatus(Error);
            return false;
        }
        
        if (!initializeCrypto()) {
            setError("Failed to initialize crypto system");
            setStatus(Error);
            return false;
        }
        
        setStatus(Ready);
        emit initialized();
        
        // 记录成功初始化
        errorHandler->reportInfo("UtilsModule initialized successfully", "UtilsModule");
        qDebug() << "UtilsModule initialized successfully with integrated configuration and error handling";
        return true;
        
    } catch (const std::exception& e) {
        QString error = QString("Exception during initialization: %1").arg(e.what());
        setError(error);
        setStatus(Error);
        
        // 报告错误到错误处理器
        UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
        errorHandler->reportError(UtilsErrorHandler::Critical, 
                                 UtilsErrorHandler::SystemError,
                                 error, "UtilsModule");
        return false;
    }
}

void UtilsModule::cleanup()
{
    QMutexLocker locker(&s_mutex);
    
    if (m_status == NotInitialized) {
        return;
    }
    
    try {
        // 记录清理开始
        UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
        errorHandler->reportInfo("UtilsModule cleanup started", "UtilsModule");
        
        // 1. 清理单例管理器（这会清理所有子系统）
        UtilsSingletonManager* singletonManager = UtilsSingletonManager::instance();
        singletonManager->cleanupAll();
        
        // 2. 清理错误处理器
        errorHandler->cleanup();
        
        setStatus(NotInitialized);
        emit cleanedUp();
        
        qDebug() << "UtilsModule cleaned up successfully";
        
    } catch (const std::exception& e) {
        qWarning() << "Exception during cleanup:" << e.what();
        setStatus(NotInitialized);
        emit cleanedUp();
    }
}

UtilsModule::ModuleStatus UtilsModule::status() const
{
    return m_status;
}

QString UtilsModule::version() const
{
    return m_configuration.value("version", "1.0.0").toString();
}

QString UtilsModule::moduleName() const
{
    return "UtilsModule";
}

QVariantMap UtilsModule::configuration() const
{
    return m_configuration;
}

void UtilsModule::setConfiguration(const QVariantMap& config)
{
    m_configuration = config;
}

bool UtilsModule::isInitialized() const
{
    return m_status == Ready;
}

QString UtilsModule::lastError() const
{
    return m_lastError;
}

bool UtilsModule::initializeLogging()
{
    // 初始化Logger单例
    Logger* logger = Logger::instance();
    if (!logger->initialize()) {
        return false;
    }
    
    // 根据配置设置日志级别
    QString logLevelStr = m_configuration.value("logLevel", "Info").toString();
    Logger::LogLevel logLevel = Logger::stringToLevel(logLevelStr);
    logger->setGlobalLogLevel(logLevel);
    
    return true;
}

bool UtilsModule::initializeFileSystem()
{
    // 初始化FileManager单例
    FileManager* fileManager = FileManager::instance();
    if (!fileManager->initialize()) {
        return false;
    }
    
    return true;
}

bool UtilsModule::initializeCrypto()
{
    // 初始化加密系统
    // 这里可以初始化OpenSSL等加密库
    
    return true;
}

void UtilsModule::setStatus(ModuleStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void UtilsModule::setError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
    qWarning() << "UtilsModule error:" << error;
}

UtilsConfig* UtilsModule::getConfig() const
{
    return UtilsConfig::instance();
}

UtilsSingletonManager* UtilsModule::getSingletonManager() const
{
    return UtilsSingletonManager::instance();
}

UtilsErrorHandler* UtilsModule::getErrorHandler() const
{
    return UtilsErrorHandler::instance();
}

bool UtilsModule::reloadConfiguration()
{
    QMutexLocker locker(&s_mutex);
    
    try {
        UtilsConfig* config = UtilsConfig::instance();
        if (config->loadConfiguration()) {
            m_configuration = config->getAllConfiguration();
            
            UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
            errorHandler->reportInfo("Configuration reloaded successfully", "UtilsModule");
            
            qDebug() << "UtilsModule configuration reloaded";
            return true;
        } else {
            setError("Failed to reload configuration");
            return false;
        }
    } catch (const std::exception& e) {
        QString error = QString("Exception during configuration reload: %1").arg(e.what());
        setError(error);
        return false;
    }
}

bool UtilsModule::saveConfiguration()
{
    QMutexLocker locker(&s_mutex);
    
    try {
        UtilsConfig* config = UtilsConfig::instance();
        if (config->saveConfiguration()) {
            UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
            errorHandler->reportInfo("Configuration saved successfully", "UtilsModule");
            
            qDebug() << "UtilsModule configuration saved";
            return true;
        } else {
            setError("Failed to save configuration");
            return false;
        }
    } catch (const std::exception& e) {
        QString error = QString("Exception during configuration save: %1").arg(e.what());
        setError(error);
        return false;
    }
}

QVariantMap UtilsModule::getModuleStatistics() const
{
    QVariantMap stats;
    
    try {
        // 基本模块信息
        stats["moduleName"] = moduleName();
        stats["version"] = version();
        stats["status"] = static_cast<int>(m_status);
        stats["statusString"] = (m_status == Ready) ? "Ready" : 
                               (m_status == Initializing) ? "Initializing" :
                               (m_status == Error) ? "Error" : "NotInitialized";
        stats["initialized"] = isInitialized();
        stats["lastError"] = m_lastError;
        
        // 单例管理器统计
        UtilsSingletonManager* singletonManager = UtilsSingletonManager::instance();
        stats["singletonStatus"] = singletonManager->getAllSingletonStatus();
        
        // 错误处理器统计
        UtilsErrorHandler* errorHandler = UtilsErrorHandler::instance();
        stats["errorStatistics"] = errorHandler->getErrorStatistics();
        
        // 配置信息
        UtilsConfig* config = UtilsConfig::instance();
        stats["configFilePath"] = config->configFilePath();
        stats["configModified"] = config->isModified();
        
        stats["timestamp"] = QDateTime::currentDateTime();
        
    } catch (const std::exception& e) {
        stats["error"] = QString("Failed to collect statistics: %1").arg(e.what());
    }
    
    return stats;
}