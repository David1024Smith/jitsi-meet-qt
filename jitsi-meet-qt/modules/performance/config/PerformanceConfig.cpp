#include "PerformanceConfig.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

// 默认配置值
const QVariantMap PerformanceConfig::s_defaultConfig = {
    // 监控配置
    {"monitoring/enabled", true},
    {"monitoring/interval", 1000},
    {"monitoring/enabledMonitors", QStringList{"CPU", "Memory", "Network"}},
    
    // 优化配置
    {"optimization/autoEnabled", false},
    {"optimization/interval", 30000},
    {"optimization/enabledOptimizers", QStringList{"CPU", "Memory"}},
    
    // 阈值配置
    {"thresholds/cpu", 80.0},
    {"thresholds/memory", 4096}, // MB
    {"thresholds/networkLatency", 100.0}, // ms
    {"thresholds/frameRate", 24.0}, // fps
    
    // 存储配置
    {"storage/dataRetentionHours", 24},
    {"storage/maxStorageSize", 100}, // MB
    {"storage/storagePath", ""},
    
    // 报告配置
    {"reporting/enabled", false},
    {"reporting/interval", 24}, // hours
    {"reporting/format", "json"},
    
    // 界面配置
    {"ui/realTimeDisplayEnabled", true},
    {"ui/chartUpdateInterval", 1000},
    {"ui/displayedMetrics", QStringList{"CPU", "Memory", "Network", "Video"}}
};

PerformanceConfig::PerformanceConfig(QObject *parent)
    : QObject(parent)
{
    // 设置默认配置文件路径
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_configFilePath = dataPath + "/performance_config.json";
    
    // 初始化默认配置
    initializeDefaults();
}

PerformanceConfig::~PerformanceConfig()
{
    // 保存配置
    saveConfig();
}

bool PerformanceConfig::loadConfig(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    QString configPath = filePath.isEmpty() ? m_configFilePath : filePath;
    
    try {
        QFile file(configPath);
        if (!file.exists()) {
            qDebug() << "PerformanceConfig: Config file does not exist, using defaults:" << configPath;
            emit configLoaded(true);
            return true;
        }
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "PerformanceConfig: Failed to open config file:" << configPath;
            emit configLoaded(false);
            return false;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "PerformanceConfig: JSON parse error:" << error.errorString();
            emit configLoaded(false);
            return false;
        }
        
        QJsonObject jsonObj = doc.object();
        
        // 递归加载配置
        loadJsonObject(jsonObj, "");
        
        // 验证配置
        bool valid = validateConfig();
        
        emit configLoaded(valid);
        qDebug() << "PerformanceConfig: Configuration loaded successfully from:" << configPath;
        
        return valid;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceConfig: Exception during loadConfig:" << e.what();
        emit configLoaded(false);
        return false;
    }
}

bool PerformanceConfig::saveConfig(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    QString configPath = filePath.isEmpty() ? m_configFilePath : filePath;
    
    try {
        QJsonObject jsonObj;
        
        // 递归保存配置
        saveToJsonObject(jsonObj, m_config);
        
        QJsonDocument doc(jsonObj);
        QByteArray data = doc.toJson();
        
        QFile file(configPath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "PerformanceConfig: Failed to open config file for writing:" << configPath;
            // Note: Cannot emit from const method
            return false;
        }
        
        file.write(data);
        file.close();
        
        // Note: Cannot emit from const method
        qDebug() << "PerformanceConfig: Configuration saved successfully to:" << configPath;
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceConfig: Exception during saveConfig:" << e.what();
        // Note: Cannot emit from const method
        return false;
    }
}

void PerformanceConfig::resetToDefaults()
{
    QMutexLocker locker(&m_mutex);
    
    m_config = s_defaultConfig;
    
    qDebug() << "PerformanceConfig: Reset to default configuration";
}

bool PerformanceConfig::validateConfig()
{
    QMutexLocker locker(&m_mutex);
    
    QStringList errors;
    bool valid = true;
    
    // 验证监控配置
    int interval = value(MonitoringConfig, "interval", 1000).toInt();
    if (interval < 100 || interval > 60000) {
        errors << "Monitoring interval must be between 100 and 60000 ms";
        valid = false;
    }
    
    // 验证阈值配置
    double cpuThreshold = value(ThresholdConfig, "cpu", 80.0).toDouble();
    if (cpuThreshold < 0 || cpuThreshold > 100) {
        errors << "CPU threshold must be between 0 and 100";
        valid = false;
    }
    
    qint64 memoryThreshold = value(ThresholdConfig, "memory", 4096).toLongLong();
    if (memoryThreshold < 0) {
        errors << "Memory threshold must be positive";
        valid = false;
    }
    
    // 验证存储配置
    int retentionHours = value(StorageConfig, "dataRetentionHours", 24).toInt();
    if (retentionHours < 1 || retentionHours > 8760) { // 1 hour to 1 year
        errors << "Data retention hours must be between 1 and 8760";
        valid = false;
    }
    
    // Note: Cannot emit from const method
    
    if (!valid) {
        qWarning() << "PerformanceConfig: Validation errors:" << errors;
    }
    
    return valid;
}

// 监控配置
void PerformanceConfig::setMonitoringEnabled(bool enabled)
{
    setValue(MonitoringConfig, "enabled", enabled);
}

bool PerformanceConfig::isMonitoringEnabled() const
{
    return value(MonitoringConfig, "enabled", true).toBool();
}

void PerformanceConfig::setMonitoringInterval(int interval)
{
    setValue(MonitoringConfig, "interval", interval);
}

int PerformanceConfig::monitoringInterval() const
{
    return value(MonitoringConfig, "interval", 1000).toInt();
}

void PerformanceConfig::setEnabledMonitors(const QStringList& monitors)
{
    setValue(MonitoringConfig, "enabledMonitors", monitors);
}

QStringList PerformanceConfig::enabledMonitors() const
{
    return value(MonitoringConfig, "enabledMonitors", QStringList{"CPU", "Memory", "Network"}).toStringList();
}

// 优化配置
void PerformanceConfig::setAutoOptimizationEnabled(bool enabled)
{
    setValue(OptimizationConfig, "autoEnabled", enabled);
}

bool PerformanceConfig::isAutoOptimizationEnabled() const
{
    return value(OptimizationConfig, "autoEnabled", false).toBool();
}

void PerformanceConfig::setOptimizationInterval(int interval)
{
    setValue(OptimizationConfig, "interval", interval);
}

int PerformanceConfig::optimizationInterval() const
{
    return value(OptimizationConfig, "interval", 30000).toInt();
}

void PerformanceConfig::setEnabledOptimizers(const QStringList& optimizers)
{
    setValue(OptimizationConfig, "enabledOptimizers", optimizers);
}

QStringList PerformanceConfig::enabledOptimizers() const
{
    return value(OptimizationConfig, "enabledOptimizers", QStringList{"CPU", "Memory"}).toStringList();
}

// 阈值配置
void PerformanceConfig::setCpuThreshold(double threshold)
{
    setValue(ThresholdConfig, "cpu", threshold);
}

double PerformanceConfig::cpuThreshold() const
{
    return value(ThresholdConfig, "cpu", 80.0).toDouble();
}

void PerformanceConfig::setMemoryThreshold(qint64 threshold)
{
    setValue(ThresholdConfig, "memory", threshold);
}

qint64 PerformanceConfig::memoryThreshold() const
{
    return value(ThresholdConfig, "memory", 4096).toLongLong();
}

void PerformanceConfig::setNetworkLatencyThreshold(double threshold)
{
    setValue(ThresholdConfig, "networkLatency", threshold);
}

double PerformanceConfig::networkLatencyThreshold() const
{
    return value(ThresholdConfig, "networkLatency", 100.0).toDouble();
}

void PerformanceConfig::setFrameRateThreshold(double threshold)
{
    setValue(ThresholdConfig, "frameRate", threshold);
}

double PerformanceConfig::frameRateThreshold() const
{
    return value(ThresholdConfig, "frameRate", 24.0).toDouble();
}

// 存储配置
void PerformanceConfig::setDataRetentionHours(int hours)
{
    setValue(StorageConfig, "dataRetentionHours", hours);
}

int PerformanceConfig::dataRetentionHours() const
{
    return value(StorageConfig, "dataRetentionHours", 24).toInt();
}

void PerformanceConfig::setMaxStorageSize(qint64 size)
{
    setValue(StorageConfig, "maxStorageSize", size);
}

qint64 PerformanceConfig::maxStorageSize() const
{
    return value(StorageConfig, "maxStorageSize", 100).toLongLong();
}

void PerformanceConfig::setStoragePath(const QString& path)
{
    setValue(StorageConfig, "storagePath", path);
}

QString PerformanceConfig::storagePath() const
{
    return value(StorageConfig, "storagePath", "").toString();
}

// 报告配置
void PerformanceConfig::setReportingEnabled(bool enabled)
{
    setValue(ReportingConfig, "enabled", enabled);
}

bool PerformanceConfig::isReportingEnabled() const
{
    return value(ReportingConfig, "enabled", false).toBool();
}

void PerformanceConfig::setReportingInterval(int interval)
{
    setValue(ReportingConfig, "interval", interval);
}

int PerformanceConfig::reportingInterval() const
{
    return value(ReportingConfig, "interval", 24).toInt();
}

void PerformanceConfig::setReportFormat(const QString& format)
{
    setValue(ReportingConfig, "format", format);
}

QString PerformanceConfig::reportFormat() const
{
    return value(ReportingConfig, "format", "json").toString();
}

// 界面配置
void PerformanceConfig::setRealTimeDisplayEnabled(bool enabled)
{
    setValue(UIConfig, "realTimeDisplayEnabled", enabled);
}

bool PerformanceConfig::isRealTimeDisplayEnabled() const
{
    return value(UIConfig, "realTimeDisplayEnabled", true).toBool();
}

void PerformanceConfig::setChartUpdateInterval(int interval)
{
    setValue(UIConfig, "chartUpdateInterval", interval);
}

int PerformanceConfig::chartUpdateInterval() const
{
    return value(UIConfig, "chartUpdateInterval", 1000).toInt();
}

void PerformanceConfig::setDisplayedMetrics(const QStringList& metrics)
{
    setValue(UIConfig, "displayedMetrics", metrics);
}

QStringList PerformanceConfig::displayedMetrics() const
{
    return value(UIConfig, "displayedMetrics", QStringList{"CPU", "Memory", "Network", "Video"}).toStringList();
}

// 通用配置方法
void PerformanceConfig::setValue(ConfigCategory category, const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    QString fullKey = getCategoryName(category) + "/" + key;
    
    if (validateValue(category, key, value)) {
        m_config[fullKey] = value;
        emit configChanged(category, key, value);
    } else {
        qWarning() << "PerformanceConfig: Invalid value for" << fullKey << ":" << value;
    }
}

QVariant PerformanceConfig::value(ConfigCategory category, const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    QString fullKey = getCategoryName(category) + "/" + key;
    return m_config.value(fullKey, defaultValue);
}

QVariantMap PerformanceConfig::getCategoryConfig(ConfigCategory category) const
{
    QMutexLocker locker(&m_mutex);
    
    QString categoryName = getCategoryName(category);
    QVariantMap categoryConfig;
    
    for (auto it = m_config.begin(); it != m_config.end(); ++it) {
        if (it.key().startsWith(categoryName + "/")) {
            QString key = it.key().mid(categoryName.length() + 1);
            categoryConfig[key] = it.value();
        }
    }
    
    return categoryConfig;
}

void PerformanceConfig::setCategoryConfig(ConfigCategory category, const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    
    QString categoryName = getCategoryName(category);
    
    // 清除现有的类别配置
    auto it = m_config.begin();
    while (it != m_config.end()) {
        if (it.key().startsWith(categoryName + "/")) {
            it = m_config.erase(it);
        } else {
            ++it;
        }
    }
    
    // 设置新的配置
    for (auto configIt = config.begin(); configIt != config.end(); ++configIt) {
        QString fullKey = categoryName + "/" + configIt.key();
        m_config[fullKey] = configIt.value();
    }
}

QVariantMap PerformanceConfig::getAllConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

void PerformanceConfig::setAllConfig(const QVariantMap& config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

QString PerformanceConfig::configFilePath() const
{
    return m_configFilePath;
}

void PerformanceConfig::setConfigFilePath(const QString& filePath)
{
    m_configFilePath = filePath;
}

QString PerformanceConfig::exportToJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject jsonObj;
    saveToJsonObject(jsonObj, m_config);
    
    QJsonDocument doc(jsonObj);
    return doc.toJson();
}

bool PerformanceConfig::importFromJson(const QString& json)
{
    QMutexLocker locker(&m_mutex);
    
    try {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "PerformanceConfig: JSON import error:" << error.errorString();
            return false;
        }
        
        QJsonObject jsonObj = doc.object();
        m_config.clear();
        loadJsonObject(jsonObj, "");
        
        return validateConfig();
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceConfig: Exception during JSON import:" << e.what();
        return false;
    }
}

void PerformanceConfig::initializeDefaults()
{
    m_config = s_defaultConfig;
}

bool PerformanceConfig::validateValue(ConfigCategory category, const QString& key, const QVariant& value) const
{
    Q_UNUSED(category)
    Q_UNUSED(key)
    
    // 基本验证：检查值是否有效
    return value.isValid();
}

QString PerformanceConfig::getCategoryName(ConfigCategory category) const
{
    switch (category) {
    case MonitoringConfig: return "monitoring";
    case OptimizationConfig: return "optimization";
    case ThresholdConfig: return "thresholds";
    case StorageConfig: return "storage";
    case ReportingConfig: return "reporting";
    case UIConfig: return "ui";
    }
    
    return "unknown";
}

void PerformanceConfig::loadJsonObject(const QJsonObject& obj, const QString& prefix)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QString key = prefix.isEmpty() ? it.key() : prefix + "/" + it.key();
        
        if (it.value().isObject()) {
            loadJsonObject(it.value().toObject(), key);
        } else {
            m_config[key] = it.value().toVariant();
        }
    }
}

void PerformanceConfig::saveToJsonObject(QJsonObject& obj, const QVariantMap& config) const
{
    for (auto it = config.begin(); it != config.end(); ++it) {
        QStringList keyParts = it.key().split('/');
        
        // Build nested JSON structure
        QJsonObject* currentObj = &obj;
        QJsonObject tempObj;
        
        for (int i = 0; i < keyParts.size() - 1; ++i) {
            const QString& part = keyParts[i];
            if (!currentObj->contains(part) || !currentObj->value(part).isObject()) {
                currentObj->insert(part, QJsonObject());
            }
        }
        
        // Set the final value using a recursive approach
        setNestedValue(obj, keyParts, QJsonValue::fromVariant(it.value()));
    }
}

void PerformanceConfig::setNestedValue(QJsonObject& obj, const QStringList& keyParts, const QJsonValue& value) const
{
    if (keyParts.size() == 1) {
        obj[keyParts.first()] = value;
        return;
    }
    
    QString firstKey = keyParts.first();
    QStringList remainingKeys = keyParts.mid(1);
    
    if (!obj.contains(firstKey) || !obj[firstKey].isObject()) {
        obj[firstKey] = QJsonObject();
    }
    
    QJsonValue val = obj[firstKey];
    QJsonObject subObj = val.toObject();
    setNestedValue(subObj, remainingKeys, value);
    obj[firstKey] = subObj;
}