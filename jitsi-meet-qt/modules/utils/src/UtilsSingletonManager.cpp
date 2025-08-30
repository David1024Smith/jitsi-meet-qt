#include "../include/UtilsSingletonManager.h"
#include "../include/Logger.h"
#include "../include/FileManager.h"
#include "../config/UtilsConfig.h"
#include <QDebug>
#include <QCoreApplication>

// 静态成员初始化
UtilsSingletonManager* UtilsSingletonManager::s_instance = nullptr;
QMutex UtilsSingletonManager::s_mutex;

UtilsSingletonManager::UtilsSingletonManager(QObject* parent)
    : QObject(parent)
    , m_logger(nullptr)
    , m_fileManager(nullptr)
    , m_config(nullptr)
{
    // 初始化单例状态
    m_enabledSingletons[LoggerSingleton] = true;
    m_enabledSingletons[FileManagerSingleton] = true;
    m_enabledSingletons[ConfigSingleton] = true;
    m_enabledSingletons[CryptoManagerSingleton] = true;
    m_enabledSingletons[StringUtilsSingleton] = true;
    m_enabledSingletons[ValidatorSingleton] = true;

    // 初始化单例初始化状态
    for (auto type : m_enabledSingletons.keys()) {
        m_initializedSingletons[type] = false;
    }
}

UtilsSingletonManager::~UtilsSingletonManager()
{
    cleanupAll();
}

UtilsSingletonManager* UtilsSingletonManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new UtilsSingletonManager();
    }
    return s_instance;
}

bool UtilsSingletonManager::initializeAll()
{
    QMutexLocker locker(&s_mutex);
    
    qDebug() << "Initializing all utility singletons...";
    
    bool allSuccess = true;
    
    // 按依赖顺序初始化单例
    QList<SingletonType> initOrder = {
        ConfigSingleton,        // 配置管理器最先初始化
        LoggerSingleton,        // 日志管理器
        FileManagerSingleton,   // 文件管理器
        CryptoManagerSingleton, // 加密管理器
        StringUtilsSingleton,   // 字符串工具
        ValidatorSingleton      // 验证器
    };
    
    for (SingletonType type : initOrder) {
        if (m_enabledSingletons.value(type, false)) {
            if (!initializeSingleton(type)) {
                allSuccess = false;
                qWarning() << "Failed to initialize singleton:" << singletonTypeToString(type);
            }
        }
    }
    
    if (allSuccess) {
        emit allSingletonsInitialized();
        qDebug() << "All utility singletons initialized successfully";
    } else {
        qWarning() << "Some utility singletons failed to initialize";
    }
    
    return allSuccess;
}

void UtilsSingletonManager::cleanupAll()
{
    QMutexLocker locker(&s_mutex);
    
    qDebug() << "Cleaning up all utility singletons...";
    
    // 按相反顺序清理单例
    QList<SingletonType> cleanupOrder = {
        ValidatorSingleton,
        StringUtilsSingleton,
        CryptoManagerSingleton,
        FileManagerSingleton,
        LoggerSingleton,
        ConfigSingleton
    };
    
    for (SingletonType type : cleanupOrder) {
        if (m_initializedSingletons.value(type, false)) {
            cleanupSingleton(type);
        }
    }
    
    emit allSingletonsCleaned();
    qDebug() << "All utility singletons cleaned up";
}

Logger* UtilsSingletonManager::getLogger()
{
    if (!m_logger && m_enabledSingletons.value(LoggerSingleton, false)) {
        if (initializeSingleton(LoggerSingleton)) {
            m_logger = Logger::instance();
        }
    }
    return m_logger;
}

FileManager* UtilsSingletonManager::getFileManager()
{
    if (!m_fileManager && m_enabledSingletons.value(FileManagerSingleton, false)) {
        if (initializeSingleton(FileManagerSingleton)) {
            m_fileManager = FileManager::instance();
        }
    }
    return m_fileManager;
}

UtilsConfig* UtilsSingletonManager::getConfig()
{
    if (!m_config && m_enabledSingletons.value(ConfigSingleton, false)) {
        if (initializeSingleton(ConfigSingleton)) {
            m_config = UtilsConfig::instance();
        }
    }
    return m_config;
}

bool UtilsSingletonManager::isSingletonInitialized(SingletonType type) const
{
    return m_initializedSingletons.value(type, false);
}

QList<UtilsSingletonManager::SingletonType> UtilsSingletonManager::getInitializedSingletons() const
{
    QList<SingletonType> initialized;
    for (auto it = m_initializedSingletons.constBegin(); 
         it != m_initializedSingletons.constEnd(); ++it) {
        if (it.value()) {
            initialized.append(it.key());
        }
    }
    return initialized;
}

bool UtilsSingletonManager::reinitializeSingleton(SingletonType type)
{
    QMutexLocker locker(&s_mutex);
    
    if (m_initializedSingletons.value(type, false)) {
        cleanupSingleton(type);
    }
    
    return initializeSingleton(type);
}

QVariantMap UtilsSingletonManager::getSingletonStatus(SingletonType type) const
{
    QVariantMap status;
    status["type"] = singletonTypeToString(type);
    status["enabled"] = m_enabledSingletons.value(type, false);
    status["initialized"] = m_initializedSingletons.value(type, false);
    status["error"] = m_singletonErrors.value(type, QString());
    status["hasParameters"] = m_singletonParameters.contains(type);
    
    return status;
}

QVariantMap UtilsSingletonManager::getAllSingletonStatus() const
{
    QVariantMap allStatus;
    
    for (auto it = m_enabledSingletons.constBegin(); 
         it != m_enabledSingletons.constEnd(); ++it) {
        QString typeName = singletonTypeToString(it.key());
        allStatus[typeName] = getSingletonStatus(it.key());
    }
    
    return allStatus;
}

void UtilsSingletonManager::setSingletonParameters(SingletonType type, const QVariantMap& parameters)
{
    QMutexLocker locker(&s_mutex);
    m_singletonParameters[type] = parameters;
}

QVariantMap UtilsSingletonManager::getSingletonParameters(SingletonType type) const
{
    return m_singletonParameters.value(type, QVariantMap());
}

void UtilsSingletonManager::setSingletonEnabled(SingletonType type, bool enabled)
{
    QMutexLocker locker(&s_mutex);
    
    if (m_enabledSingletons.value(type, false) != enabled) {
        m_enabledSingletons[type] = enabled;
        
        if (!enabled && m_initializedSingletons.value(type, false)) {
            cleanupSingleton(type);
        }
    }
}

bool UtilsSingletonManager::isSingletonEnabled(SingletonType type) const
{
    return m_enabledSingletons.value(type, false);
}

QString UtilsSingletonManager::singletonTypeToString(SingletonType type)
{
    switch (type) {
        case LoggerSingleton: return "Logger";
        case FileManagerSingleton: return "FileManager";
        case ConfigSingleton: return "Config";
        case CryptoManagerSingleton: return "CryptoManager";
        case StringUtilsSingleton: return "StringUtils";
        case ValidatorSingleton: return "Validator";
        default: return "Unknown";
    }
}

UtilsSingletonManager::SingletonType UtilsSingletonManager::stringToSingletonType(const QString& typeName)
{
    static QMap<QString, SingletonType> typeMap;
    if (typeMap.isEmpty()) {
        typeMap["Logger"] = LoggerSingleton;
        typeMap["FileManager"] = FileManagerSingleton;
        typeMap["Config"] = ConfigSingleton;
        typeMap["CryptoManager"] = CryptoManagerSingleton;
        typeMap["StringUtils"] = StringUtilsSingleton;
        typeMap["Validator"] = ValidatorSingleton;
    }
    
    return typeMap.value(typeName, LoggerSingleton);
}

bool UtilsSingletonManager::initializeSingleton(SingletonType type)
{
    if (m_initializedSingletons.value(type, false)) {
        return true; // 已经初始化
    }
    
    if (!m_enabledSingletons.value(type, false)) {
        return false; // 未启用
    }
    
    try {
        bool success = false;
        QString errorMsg;
        
        switch (type) {
            case ConfigSingleton:
                if (createSingletonInstance(type)) {
                    m_config = UtilsConfig::instance();
                    success = m_config->initialize();
                    if (!success) {
                        errorMsg = "Failed to initialize UtilsConfig";
                    }
                }
                break;
                
            case LoggerSingleton:
                if (createSingletonInstance(type)) {
                    m_logger = Logger::instance();
                    success = m_logger->initialize();
                    if (!success) {
                        errorMsg = "Failed to initialize Logger";
                    }
                }
                break;
                
            case FileManagerSingleton:
                if (createSingletonInstance(type)) {
                    m_fileManager = FileManager::instance();
                    success = m_fileManager->initialize();
                    if (!success) {
                        errorMsg = "Failed to initialize FileManager";
                    }
                }
                break;
                
            case CryptoManagerSingleton:
            case StringUtilsSingleton:
            case ValidatorSingleton:
                // 这些是静态工具类，不需要特殊初始化
                success = true;
                break;
                
            default:
                errorMsg = "Unknown singleton type";
                break;
        }
        
        if (success) {
            m_initializedSingletons[type] = true;
            m_singletonErrors.remove(type);
            emit singletonInitialized(type);
            
            qDebug() << "Singleton initialized:" << singletonTypeToString(type);
            return true;
        } else {
            m_singletonErrors[type] = errorMsg;
            emit singletonError(type, errorMsg);
            qWarning() << "Failed to initialize singleton:" << singletonTypeToString(type) << errorMsg;
            return false;
        }
        
    } catch (const std::exception& e) {
        QString error = QString("Exception during singleton initialization: %1").arg(e.what());
        m_singletonErrors[type] = error;
        emit singletonError(type, error);
        qWarning() << "Exception initializing singleton:" << singletonTypeToString(type) << error;
        return false;
    }
}

void UtilsSingletonManager::cleanupSingleton(SingletonType type)
{
    if (!m_initializedSingletons.value(type, false)) {
        return; // 未初始化
    }
    
    try {
        switch (type) {
            case ConfigSingleton:
                if (m_config) {
                    // UtilsConfig 没有显式的cleanup方法，依赖析构函数
                    m_config = nullptr;
                }
                break;
                
            case LoggerSingleton:
                if (m_logger) {
                    m_logger->cleanup();
                    m_logger = nullptr;
                }
                break;
                
            case FileManagerSingleton:
                if (m_fileManager) {
                    m_fileManager->cleanup();
                    m_fileManager = nullptr;
                }
                break;
                
            case CryptoManagerSingleton:
            case StringUtilsSingleton:
            case ValidatorSingleton:
                // 静态工具类，不需要特殊清理
                break;
                
            default:
                break;
        }
        
        m_initializedSingletons[type] = false;
        m_singletonErrors.remove(type);
        emit singletonCleaned(type);
        
        qDebug() << "Singleton cleaned up:" << singletonTypeToString(type);
        
    } catch (const std::exception& e) {
        QString error = QString("Exception during singleton cleanup: %1").arg(e.what());
        qWarning() << "Exception cleaning up singleton:" << singletonTypeToString(type) << error;
    }
}

bool UtilsSingletonManager::createSingletonInstance(SingletonType type)
{
    // 对于使用Qt单例模式的类，这里主要是确保实例存在
    // 实际的实例创建由各自的instance()方法处理
    
    switch (type) {
        case ConfigSingleton:
            return UtilsConfig::instance() != nullptr;
            
        case LoggerSingleton:
            return Logger::instance() != nullptr;
            
        case FileManagerSingleton:
            return FileManager::instance() != nullptr;
            
        case CryptoManagerSingleton:
        case StringUtilsSingleton:
        case ValidatorSingleton:
            return true; // 静态工具类
            
        default:
            return false;
    }
}

void UtilsSingletonManager::destroySingletonInstance(SingletonType type)
{
    // Qt单例通常在应用程序结束时自动销毁
    // 这里主要是重置本地指针
    
    switch (type) {
        case ConfigSingleton:
            m_config = nullptr;
            break;
            
        case LoggerSingleton:
            m_logger = nullptr;
            break;
            
        case FileManagerSingleton:
            m_fileManager = nullptr;
            break;
            
        default:
            break;
    }
}