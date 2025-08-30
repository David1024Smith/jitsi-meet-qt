#include "ReplacementConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

ReplacementConfig::ReplacementConfig(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
{
    loadDefaultConfiguration();
}

ReplacementConfig::~ReplacementConfig()
{
}

bool ReplacementConfig::loadConfiguration(const QString& configPath)
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open replacement config file:" << configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in replacement config:" << error.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "Invalid replacement config format";
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 加载策略配置
    if (root.contains("strategies")) {
        QJsonObject strategies = root["strategies"].toObject();
        loadStrategies(strategies);
    }
    
    // 加载模块配置
    if (root.contains("modules")) {
        QJsonObject modules = root["modules"].toObject();
        loadModuleConfigurations(modules);
    }
    
    // 加载安全配置
    if (root.contains("safety")) {
        QJsonObject safety = root["safety"].toObject();
        loadSafetyConfiguration(safety);
    }
    
    m_loaded = true;
    return true;
}

bool ReplacementConfig::saveConfiguration(const QString& configPath) const
{
    QJsonObject root;
    
    // 保存策略配置
    QJsonObject strategies;
    saveStrategies(strategies);
    root["strategies"] = strategies;
    
    // 保存模块配置
    QJsonObject modules;
    saveModuleConfigurations(modules);
    root["modules"] = modules;
    
    // 保存安全配置
    QJsonObject safety;
    saveSafetyConfiguration(safety);
    root["safety"] = safety;
    
    QJsonDocument doc(root);
    
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write replacement config file:" << configPath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

QVariantMap ReplacementConfig::getStrategyConfiguration(const QString& strategyName) const
{
    return m_strategyConfigurations.value(strategyName);
}

void ReplacementConfig::setStrategyConfiguration(const QString& strategyName, const QVariantMap& config)
{
    m_strategyConfigurations[strategyName] = config;
}

QVariantMap ReplacementConfig::getModuleConfiguration(const QString& moduleName) const
{
    return m_moduleConfigurations.value(moduleName);
}

void ReplacementConfig::setModuleConfiguration(const QString& moduleName, const QVariantMap& config)
{
    m_moduleConfigurations[moduleName] = config;
}

QVariantMap ReplacementConfig::getSafetyConfiguration() const
{
    return m_safetyConfiguration;
}

void ReplacementConfig::setSafetyConfiguration(const QVariantMap& config)
{
    m_safetyConfiguration = config;
}

QStringList ReplacementConfig::getAvailableStrategies() const
{
    return m_strategyConfigurations.keys();
}

QStringList ReplacementConfig::getConfiguredModules() const
{
    return m_moduleConfigurations.keys();
}

bool ReplacementConfig::isLoaded() const
{
    return m_loaded;
}

void ReplacementConfig::loadDefaultConfiguration()
{
    // 保守策略配置
    QVariantMap conservativeConfig;
    conservativeConfig["validation_required"] = true;
    conservativeConfig["performance_test_required"] = true;
    conservativeConfig["parallel_execution_time"] = 3600;
    conservativeConfig["rollback_on_failure"] = true;
    conservativeConfig["max_performance_degradation"] = 0.05; // 5%
    conservativeConfig["validation_timeout"] = 300; // 5分钟
    conservativeConfig["steps"] = QStringList{
        "prepare_environment", "validate_functionality", 
        "run_parallel_test", "validate_functionality",
        "switch_implementation", "validate_functionality",
        "cleanup_legacy"
    };
    m_strategyConfigurations["conservative"] = conservativeConfig;
    
    // 平衡策略配置
    QVariantMap balancedConfig;
    balancedConfig["validation_required"] = true;
    balancedConfig["performance_test_required"] = true;
    balancedConfig["parallel_execution_time"] = 1800;
    balancedConfig["rollback_on_failure"] = true;
    balancedConfig["max_performance_degradation"] = 0.1; // 10%
    balancedConfig["validation_timeout"] = 180; // 3分钟
    balancedConfig["steps"] = QStringList{
        "prepare_environment", "validate_functionality",
        "switch_implementation", "validate_functionality",
        "cleanup_legacy"
    };
    m_strategyConfigurations["balanced"] = balancedConfig;
    
    // 激进策略配置
    QVariantMap aggressiveConfig;
    aggressiveConfig["validation_required"] = false;
    aggressiveConfig["performance_test_required"] = false;
    aggressiveConfig["parallel_execution_time"] = 300;
    aggressiveConfig["rollback_on_failure"] = false;
    aggressiveConfig["max_performance_degradation"] = 0.2; // 20%
    aggressiveConfig["validation_timeout"] = 60; // 1分钟
    aggressiveConfig["steps"] = QStringList{
        "prepare_environment", "switch_implementation", 
        "cleanup_legacy"
    };
    m_strategyConfigurations["aggressive"] = aggressiveConfig;
    
    // 默认安全配置
    m_safetyConfiguration["max_concurrent_replacements"] = 3;
    m_safetyConfiguration["system_load_threshold"] = 0.8;
    m_safetyConfiguration["memory_usage_threshold"] = 0.9;
    m_safetyConfiguration["error_rate_threshold"] = 0.01;
    m_safetyConfiguration["checkpoint_interval"] = 300; // 5分钟
    m_safetyConfiguration["emergency_rollback_enabled"] = true;
}

void ReplacementConfig::loadStrategies(const QJsonObject& strategies)
{
    for (auto it = strategies.begin(); it != strategies.end(); ++it) {
        QString strategyName = it.key();
        QJsonObject strategyObj = it.value().toObject();
        
        QVariantMap config;
        for (auto configIt = strategyObj.begin(); configIt != strategyObj.end(); ++configIt) {
            config[configIt.key()] = configIt.value().toVariant();
        }
        
        m_strategyConfigurations[strategyName] = config;
    }
}

void ReplacementConfig::loadModuleConfigurations(const QJsonObject& modules)
{
    for (auto it = modules.begin(); it != modules.end(); ++it) {
        QString moduleName = it.key();
        QJsonObject moduleObj = it.value().toObject();
        
        QVariantMap config;
        for (auto configIt = moduleObj.begin(); configIt != moduleObj.end(); ++configIt) {
            config[configIt.key()] = configIt.value().toVariant();
        }
        
        m_moduleConfigurations[moduleName] = config;
    }
}

void ReplacementConfig::loadSafetyConfiguration(const QJsonObject& safety)
{
    for (auto it = safety.begin(); it != safety.end(); ++it) {
        m_safetyConfiguration[it.key()] = it.value().toVariant();
    }
}

void ReplacementConfig::saveStrategies(QJsonObject& strategies) const
{
    for (auto it = m_strategyConfigurations.begin(); it != m_strategyConfigurations.end(); ++it) {
        QJsonObject strategyObj = QJsonObject::fromVariantMap(it.value());
        strategies[it.key()] = strategyObj;
    }
}

void ReplacementConfig::saveModuleConfigurations(QJsonObject& modules) const
{
    for (auto it = m_moduleConfigurations.begin(); it != m_moduleConfigurations.end(); ++it) {
        QJsonObject moduleObj = QJsonObject::fromVariantMap(it.value());
        modules[it.key()] = moduleObj;
    }
}

void ReplacementConfig::saveSafetyConfiguration(QJsonObject& safety) const
{
    safety = QJsonObject::fromVariantMap(m_safetyConfiguration);
}