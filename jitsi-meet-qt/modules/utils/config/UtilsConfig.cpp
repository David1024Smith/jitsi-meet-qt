#include "UtilsConfig.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonParseError>
#include <QFile>
#include <QTextStream>

// 静态成员初始化
UtilsConfig* UtilsConfig::s_instance = nullptr;
QMutex UtilsConfig::s_mutex;

UtilsConfig::UtilsConfig(QObject* parent)
    : QObject(parent)
    , m_modified(false)
    , m_settings(nullptr)
{
    initializeDefaults();
}

UtilsConfig::~UtilsConfig()
{
    if (m_settings) {
        delete m_settings;
    }
}

UtilsConfig* UtilsConfig::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new UtilsConfig();
    }
    return s_instance;
}

bool UtilsConfig::initialize(const QString& configFilePath)
{
    QMutexLocker locker(&s_mutex);
    
    // 确定配置文件路径
    if (!configFilePath.isEmpty()) {
        m_configFilePath = configFilePath;
    } else {
        QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        configDir += "/JitsiMeetQt/modules/utils";
        
        if (!createConfigDirectory(configDir)) {
            qWarning() << "Failed to create config directory:" << configDir;
            return false;
        }
        
        m_configFilePath = configDir + "/utils_config.ini";
    }
    
    // 初始化QSettings
    if (m_settings) {
        delete m_settings;
    }
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat);
    
    // 加载配置
    return loadConfiguration();
}

bool UtilsConfig::loadConfiguration(const QString& filePath)
{
    QString path = filePath.isEmpty() ? m_configFilePath : filePath;
    
    if (path.isEmpty()) {
        qWarning() << "No configuration file path specified";
        return false;
    }
    
    try {
        if (!QFile::exists(path)) {
            qDebug() << "Configuration file does not exist, using defaults:" << path;
            m_configuration = m_defaultConfiguration;
            return saveConfiguration(); // 保存默认配置
        }
        
        QSettings settings(path, QSettings::IniFormat);
        
        // 加载所有配置项
        for (auto it = m_defaultConfiguration.constBegin(); 
             it != m_defaultConfiguration.constEnd(); ++it) {
            QVariant value = settings.value(it.key(), it.value());
            m_configuration[it.key()] = value;
        }
        
        // 验证配置
        if (!validateConfiguration()) {
            qWarning() << "Configuration validation failed, resetting to defaults";
            resetToDefaults();
            return false;
        }
        
        m_modified = false;
        emit configurationLoaded();
        
        qDebug() << "Configuration loaded successfully from:" << path;
        return true;
        
    } catch (const std::exception& e) {
        QString error = QString("Failed to load configuration: %1").arg(e.what());
        emit configurationError(error);
        qWarning() << error;
        return false;
    }
}

bool UtilsConfig::saveConfiguration(const QString& filePath)
{
    QString path = filePath.isEmpty() ? m_configFilePath : filePath;
    
    if (path.isEmpty()) {
        qWarning() << "No configuration file path specified";
        return false;
    }
    
    try {
        QSettings settings(path, QSettings::IniFormat);
        
        // 保存所有配置项
        for (auto it = m_configuration.constBegin(); 
             it != m_configuration.constEnd(); ++it) {
            settings.setValue(it.key(), it.value());
        }
        
        settings.sync();
        
        if (settings.status() != QSettings::NoError) {
            QString error = "Failed to save configuration to file";
            emit configurationError(error);
            qWarning() << error << path;
            return false;
        }
        
        m_modified = false;
        emit configurationSaved();
        
        qDebug() << "Configuration saved successfully to:" << path;
        return true;
        
    } catch (const std::exception& e) {
        QString error = QString("Failed to save configuration: %1").arg(e.what());
        emit configurationError(error);
        qWarning() << error;
        return false;
    }
}

void UtilsConfig::resetToDefaults()
{
    QMutexLocker locker(&s_mutex);
    m_configuration = m_defaultConfiguration;
    m_modified = true;
    
    // 发送所有配置项的变更信号
    for (auto it = m_configuration.constBegin(); 
         it != m_configuration.constEnd(); ++it) {
        emit configurationChanged(it.key(), it.value());
    }
    
    qDebug() << "Configuration reset to defaults";
}

bool UtilsConfig::validateConfiguration() const
{
    // 验证所有配置项
    for (int i = 0; i < static_cast<int>(EnableNetworkCache) + 1; ++i) {
        ConfigKey key = static_cast<ConfigKey>(i);
        QString keyName = keyToString(key);
        
        if (m_configuration.contains(keyName)) {
            if (!validateConfigItem(key, m_configuration[keyName])) {
                qWarning() << "Invalid configuration for key:" << keyName;
                return false;
            }
        }
    }
    
    return true;
}

QVariant UtilsConfig::getValue(ConfigKey key) const
{
    QString keyName = keyToString(key);
    return m_configuration.value(keyName, getDefaultValue(key));
}

QVariant UtilsConfig::getValue(const QString& keyName) const
{
    return m_configuration.value(keyName);
}

void UtilsConfig::setValue(ConfigKey key, const QVariant& value)
{
    QString keyName = keyToString(key);
    setValue(keyName, value);
}

void UtilsConfig::setValue(const QString& keyName, const QVariant& value)
{
    QMutexLocker locker(&s_mutex);
    
    if (m_configuration.value(keyName) != value) {
        m_configuration[keyName] = value;
        m_modified = true;
        emit configurationChanged(keyName, value);
    }
}

QVariantMap UtilsConfig::getAllConfiguration() const
{
    return m_configuration;
}

void UtilsConfig::setAllConfiguration(const QVariantMap& config)
{
    QMutexLocker locker(&s_mutex);
    
    m_configuration = config;
    m_modified = true;
    
    // 发送所有配置项的变更信号
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        emit configurationChanged(it.key(), it.value());
    }
}

QString UtilsConfig::keyToString(ConfigKey key)
{
    switch (key) {
        case ModuleVersion: return "module/version";
        case ModuleEnabled: return "module/enabled";
        case DebugMode: return "module/debug";
        
        case LogLevel: return "logging/level";
        case EnableFileLogging: return "logging/enableFile";
        case EnableConsoleLogging: return "logging/enableConsole";
        case EnableNetworkLogging: return "logging/enableNetwork";
        case LogFilePath: return "logging/filePath";
        case LogFileMaxSize: return "logging/maxFileSize";
        case LogFileMaxCount: return "logging/maxFileCount";
        
        case TempDirectory: return "filesystem/tempDirectory";
        case ConfigDirectory: return "filesystem/configDirectory";
        case CacheDirectory: return "filesystem/cacheDirectory";
        case MaxTempFileSize: return "filesystem/maxTempFileSize";
        case AutoCleanupTempFiles: return "filesystem/autoCleanupTempFiles";
        
        case DefaultEncryptionAlgorithm: return "crypto/defaultAlgorithm";
        case KeySize: return "crypto/keySize";
        case EnableSecureRandom: return "crypto/enableSecureRandom";
        
        case MaxConcurrentOperations: return "performance/maxConcurrentOps";
        case OperationTimeout: return "performance/operationTimeout";
        case EnablePerformanceMonitoring: return "performance/enableMonitoring";
        
        case NetworkTimeout: return "network/timeout";
        case MaxRetryAttempts: return "network/maxRetryAttempts";
        case EnableNetworkCache: return "network/enableCache";
        
        default: return QString();
    }
}

UtilsConfig::ConfigKey UtilsConfig::stringToKey(const QString& keyName)
{
    static QMap<QString, ConfigKey> keyMap;
    if (keyMap.isEmpty()) {
        for (int i = 0; i < static_cast<int>(EnableNetworkCache) + 1; ++i) {
            ConfigKey key = static_cast<ConfigKey>(i);
            keyMap[keyToString(key)] = key;
        }
    }
    
    return keyMap.value(keyName, ModuleVersion);
}

QVariant UtilsConfig::getDefaultValue(ConfigKey key)
{
    switch (key) {
        case ModuleVersion: return "1.0.0";
        case ModuleEnabled: return true;
        case DebugMode: return false;
        
        case LogLevel: return "Info";
        case EnableFileLogging: return true;
        case EnableConsoleLogging: return true;
        case EnableNetworkLogging: return false;
        case LogFilePath: return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs/utils.log";
        case LogFileMaxSize: return 10 * 1024 * 1024; // 10MB
        case LogFileMaxCount: return 5;
        
        case TempDirectory: return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/JitsiMeetQt";
        case ConfigDirectory: return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/JitsiMeetQt";
        case CacheDirectory: return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/JitsiMeetQt";
        case MaxTempFileSize: return 100 * 1024 * 1024; // 100MB
        case AutoCleanupTempFiles: return true;
        
        case DefaultEncryptionAlgorithm: return "AES-256";
        case KeySize: return 256;
        case EnableSecureRandom: return true;
        
        case MaxConcurrentOperations: return 10;
        case OperationTimeout: return 30000; // 30 seconds
        case EnablePerformanceMonitoring: return false;
        
        case NetworkTimeout: return 10000; // 10 seconds
        case MaxRetryAttempts: return 3;
        case EnableNetworkCache: return true;
        
        default: return QVariant();
    }
}

QString UtilsConfig::configFilePath() const
{
    return m_configFilePath;
}

bool UtilsConfig::isModified() const
{
    return m_modified;
}

QJsonObject UtilsConfig::exportToJson() const
{
    QJsonObject json;
    
    for (auto it = m_configuration.constBegin(); 
         it != m_configuration.constEnd(); ++it) {
        json[it.key()] = QJsonValue::fromVariant(it.value());
    }
    
    return json;
}

bool UtilsConfig::importFromJson(const QJsonObject& json)
{
    try {
        QVariantMap config;
        
        for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
            config[it.key()] = it.value().toVariant();
        }
        
        setAllConfiguration(config);
        return true;
        
    } catch (const std::exception& e) {
        QString error = QString("Failed to import configuration from JSON: %1").arg(e.what());
        emit configurationError(error);
        qWarning() << error;
        return false;
    }
}

void UtilsConfig::initializeDefaults()
{
    // 初始化所有默认配置
    for (int i = 0; i < static_cast<int>(EnableNetworkCache) + 1; ++i) {
        ConfigKey key = static_cast<ConfigKey>(i);
        QString keyName = keyToString(key);
        QVariant defaultValue = getDefaultValue(key);
        m_defaultConfiguration[keyName] = defaultValue;
        m_configuration[keyName] = defaultValue;
    }
}

bool UtilsConfig::validateConfigItem(ConfigKey key, const QVariant& value) const
{
    switch (key) {
        case ModuleVersion:
            return value.toString().contains(QRegExp("^\\d+\\.\\d+\\.\\d+$"));
            
        case ModuleEnabled:
        case DebugMode:
        case EnableFileLogging:
        case EnableConsoleLogging:
        case EnableNetworkLogging:
        case AutoCleanupTempFiles:
        case EnableSecureRandom:
        case EnablePerformanceMonitoring:
        case EnableNetworkCache:
            return value.canConvert<bool>();
            
        case LogLevel:
            return QStringList({"Debug", "Info", "Warning", "Error", "Critical"}).contains(value.toString());
            
        case LogFilePath:
        case TempDirectory:
        case ConfigDirectory:
        case CacheDirectory:
            return !value.toString().isEmpty();
            
        case LogFileMaxSize:
        case LogFileMaxCount:
        case MaxTempFileSize:
        case KeySize:
        case MaxConcurrentOperations:
        case OperationTimeout:
        case NetworkTimeout:
        case MaxRetryAttempts:
            return value.canConvert<int>() && value.toInt() > 0;
            
        case DefaultEncryptionAlgorithm:
            return QStringList({"AES-128", "AES-192", "AES-256", "RSA-2048", "RSA-4096"}).contains(value.toString());
            
        default:
            return true;
    }
}

bool UtilsConfig::createConfigDirectory(const QString& path)
{
    QDir dir;
    if (!dir.exists(path)) {
        return dir.mkpath(path);
    }
    return true;
}