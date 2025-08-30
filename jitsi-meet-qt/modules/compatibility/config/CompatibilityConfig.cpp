#include "CompatibilityConfig.h"
#include <QDebug>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

CompatibilityConfig::CompatibilityConfig(QObject *parent)
    : QObject(parent)
{
    // 设置默认配置文件路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_configFilePath = QDir(appDataPath).absoluteFilePath("compatibility_config.json");
    
    setupDefaultConfiguration();
}

CompatibilityConfig::~CompatibilityConfig()
{
}

bool CompatibilityConfig::loadConfiguration()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "Loading compatibility configuration from:" << m_configFilePath;
    
    if (!QFile::exists(m_configFilePath)) {
        qDebug() << "Configuration file does not exist, using defaults";
        emit configurationLoaded();
        return true;
    }
    
    if (loadFromFile(m_configFilePath)) {
        emit configurationLoaded();
        qDebug() << "Configuration loaded successfully";
        return true;
    } else {
        qWarning() << "Failed to load configuration, using defaults";
        setupDefaultConfiguration();
        return false;
    }
}

bool CompatibilityConfig::saveConfiguration()
{
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "Saving compatibility configuration to:" << m_configFilePath;
    
    // 确保目录存在
    QDir dir = QFileInfo(m_configFilePath).dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create configuration directory";
            return false;
        }
    }
    
    if (saveToFile(m_configFilePath)) {
        emit configurationSaved();
        qDebug() << "Configuration saved successfully";
        return true;
    } else {
        qWarning() << "Failed to save configuration";
        return false;
    }
}

bool CompatibilityConfig::isValidationEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["validation_enabled"].toBool();
}

void CompatibilityConfig::setValidationEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (m_configuration["validation_enabled"].toBool() != enabled) {
        m_configuration["validation_enabled"] = enabled;
        emit configurationChanged();
    }
}

bool CompatibilityConfig::isPerformanceCheckEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["performance_check_enabled"].toBool();
}

void CompatibilityConfig::setPerformanceCheckEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (m_configuration["performance_check_enabled"].toBool() != enabled) {
        m_configuration["performance_check_enabled"] = enabled;
        emit configurationChanged();
    }
}

bool CompatibilityConfig::isAutoRollbackEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["auto_rollback_enabled"].toBool();
}

void CompatibilityConfig::setAutoRollbackEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (m_configuration["auto_rollback_enabled"].toBool() != enabled) {
        m_configuration["auto_rollback_enabled"] = enabled;
        emit configurationChanged();
    }
}

int CompatibilityConfig::getCheckpointRetentionDays() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["checkpoint_retention_days"].toInt();
}

void CompatibilityConfig::setCheckpointRetentionDays(int days)
{
    QMutexLocker locker(&m_mutex);
    if (m_configuration["checkpoint_retention_days"].toInt() != days) {
        m_configuration["checkpoint_retention_days"] = days;
        emit configurationChanged();
    }
}

int CompatibilityConfig::getMaxRollbackAttempts() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["max_rollback_attempts"].toInt();
}

void CompatibilityConfig::setMaxRollbackAttempts(int attempts)
{
    QMutexLocker locker(&m_mutex);
    if (m_configuration["max_rollback_attempts"].toInt() != attempts) {
        m_configuration["max_rollback_attempts"] = attempts;
        emit configurationChanged();
    }
}

QVariantMap CompatibilityConfig::getAdapterConfig(const QString& adapterName) const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap adapters = m_configuration["adapters"].toMap();
    return adapters.value(adapterName).toMap();
}

void CompatibilityConfig::setAdapterConfig(const QString& adapterName, const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap adapters = m_configuration["adapters"].toMap();
    adapters[adapterName] = config;
    m_configuration["adapters"] = adapters;
    
    emit configurationChanged();
}

QVariantMap CompatibilityConfig::getValidatorConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["validator"].toMap();
}

void CompatibilityConfig::setValidatorConfig(const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    m_configuration["validator"] = config;
    emit configurationChanged();
}

QVariantMap CompatibilityConfig::getRollbackConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration["rollback"].toMap();
}

void CompatibilityConfig::setRollbackConfig(const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    m_configuration["rollback"] = config;
    emit configurationChanged();
}

QString CompatibilityConfig::getConfigFilePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_configFilePath;
}

void CompatibilityConfig::setConfigFilePath(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    m_configFilePath = filePath;
}

void CompatibilityConfig::resetToDefaults()
{
    QMutexLocker locker(&m_mutex);
    setupDefaultConfiguration();
    emit configurationChanged();
}

bool CompatibilityConfig::validateConfiguration() const
{
    QMutexLocker locker(&m_mutex);
    
    // 验证必要的配置项是否存在
    QStringList requiredKeys = {
        "validation_enabled",
        "performance_check_enabled",
        "auto_rollback_enabled",
        "checkpoint_retention_days",
        "max_rollback_attempts"
    };
    
    for (const QString& key : requiredKeys) {
        if (!m_configuration.contains(key)) {
            qWarning() << "Missing required configuration key:" << key;
            return false;
        }
    }
    
    // 验证数值范围
    int retentionDays = m_configuration["checkpoint_retention_days"].toInt();
    if (retentionDays < 1 || retentionDays > 365) {
        qWarning() << "Invalid checkpoint retention days:" << retentionDays;
        return false;
    }
    
    int maxAttempts = m_configuration["max_rollback_attempts"].toInt();
    if (maxAttempts < 1 || maxAttempts > 10) {
        qWarning() << "Invalid max rollback attempts:" << maxAttempts;
        return false;
    }
    
    return true;
}

void CompatibilityConfig::setupDefaultConfiguration()
{
    m_configuration.clear();
    
    // 全局设置
    m_configuration["validation_enabled"] = true;
    m_configuration["performance_check_enabled"] = true;
    m_configuration["auto_rollback_enabled"] = false;
    m_configuration["checkpoint_retention_days"] = 30;
    m_configuration["max_rollback_attempts"] = 3;
    
    // 适配器配置
    QVariantMap adapters;
    
    // 媒体适配器配置
    QVariantMap mediaAdapter;
    mediaAdapter["enabled"] = true;
    mediaAdapter["compatibility_mode"] = "full";
    mediaAdapter["enable_audio"] = true;
    mediaAdapter["enable_video"] = true;
    adapters["MediaManagerAdapter"] = mediaAdapter;
    
    // 聊天适配器配置
    QVariantMap chatAdapter;
    chatAdapter["enabled"] = true;
    chatAdapter["compatibility_mode"] = "full";
    chatAdapter["enable_file_sharing"] = true;
    chatAdapter["enable_history"] = true;
    adapters["ChatManagerAdapter"] = chatAdapter;
    
    // 屏幕共享适配器配置
    QVariantMap screenShareAdapter;
    screenShareAdapter["enabled"] = true;
    screenShareAdapter["compatibility_mode"] = "full";
    screenShareAdapter["enable_region_capture"] = true;
    screenShareAdapter["enable_window_capture"] = true;
    adapters["ScreenShareManagerAdapter"] = screenShareAdapter;
    
    // 会议适配器配置
    QVariantMap conferenceAdapter;
    conferenceAdapter["enabled"] = true;
    conferenceAdapter["compatibility_mode"] = "full";
    conferenceAdapter["enable_authentication"] = true;
    conferenceAdapter["enable_room_management"] = true;
    adapters["ConferenceManagerAdapter"] = conferenceAdapter;
    
    m_configuration["adapters"] = adapters;
    
    // 验证器配置
    QVariantMap validator;
    validator["strict_mode"] = false;
    validator["performance_threshold"] = 0.8;
    validator["max_test_duration"] = 30000;
    validator["parallel_tests"] = false;
    validator["test_timeout"] = 10000;
    m_configuration["validator"] = validator;
    
    // 回滚管理器配置
    QVariantMap rollback;
    rollback["max_checkpoints"] = 50;
    rollback["auto_cleanup_enabled"] = true;
    rollback["auto_cleanup_interval"] = 7;
    rollback["compression_enabled"] = false;
    m_configuration["rollback"] = rollback;
    
    qDebug() << "Default compatibility configuration set up";
}

bool CompatibilityConfig::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open configuration file for reading:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in configuration file:" << filePath;
        return false;
    }
    
    QJsonObject obj = doc.object();
    m_configuration = obj.toVariantMap();
    
    // 验证加载的配置
    if (!validateConfiguration()) {
        qWarning() << "Loaded configuration is invalid";
        return false;
    }
    
    return true;
}

bool CompatibilityConfig::saveToFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open configuration file for writing:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromVariant(m_configuration);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    qint64 bytesWritten = file.write(data);
    file.close();
    
    if (bytesWritten != data.size()) {
        qWarning() << "Failed to write complete configuration file";
        return false;
    }
    
    return true;
}