#include "GlobalModuleConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>
#include <QDebug>

GlobalModuleConfig* GlobalModuleConfig::s_instance = nullptr;
QMutex GlobalModuleConfig::s_mutex;

GlobalModuleConfig* GlobalModuleConfig::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new GlobalModuleConfig();
    }
    return s_instance;
}

GlobalModuleConfig::GlobalModuleConfig(QObject* parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_autoSaveEnabled(true)
    , m_autoSaveInterval(30000) // 30秒
    , m_autoSaveTimer(new QTimer(this))
    , m_configurationChanged(false)
{
    // 设置默认配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_configFilePath = configDir + "/modules.conf";
    
    // 初始化设置对象
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    
    // 初始化默认值
    initializeDefaults();
    
    // 设置自动保存
    setupAutoSave();
    
    qDebug() << "GlobalModuleConfig initialized with config file:" << m_configFilePath;
}

GlobalModuleConfig::~GlobalModuleConfig()
{
    if (m_configurationChanged) {
        saveConfiguration();
    }
}

void GlobalModuleConfig::initializeDefaults()
{
    // 设置默认的全局配置
    if (!m_settings->contains("Global/Version")) {
        m_settings->setValue("Global/Version", "1.0.0");
        m_settings->setValue("Global/AutoSave", true);
        m_settings->setValue("Global/AutoSaveInterval", 30000);
        m_settings->setValue("Global/ValidateOnLoad", true);
        m_configurationChanged = true;
    }
}

void GlobalModuleConfig::setupAutoSave()
{
    connect(m_autoSaveTimer, &QTimer::timeout, this, &GlobalModuleConfig::onAutoSave);
    
    if (m_autoSaveEnabled) {
        m_autoSaveTimer->start(m_autoSaveInterval);
    }
}

bool GlobalModuleConfig::loadConfiguration()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        m_settings->sync();
        
        // 清空当前配置
        m_modules.clear();
        m_dependencies.clear();
        
        // 加载模块信息
        m_settings->beginGroup("Modules");
        QStringList moduleNames = m_settings->childGroups();
        
        for (const QString& moduleName : moduleNames) {
            m_settings->beginGroup(moduleName);
            
            ModuleInfo info;
            info.name = moduleName;
            info.version = m_settings->value("version", "1.0.0").toString();
            info.description = m_settings->value("description").toString();
            info.enabled = m_settings->value("enabled", true).toBool();
            info.priority = m_settings->value("priority", 2).toInt();
            info.dependencies = m_settings->value("dependencies").toStringList();
            info.lastModified = m_settings->value("lastModified", QDateTime::currentDateTime()).toDateTime();
            
            // 加载配置数据
            m_settings->beginGroup("Config");
            QStringList configKeys = m_settings->childKeys();
            for (const QString& key : configKeys) {
                info.configuration[key] = m_settings->value(key);
            }
            m_settings->endGroup();
            
            m_modules[moduleName] = info;
            m_settings->endGroup();
        }
        m_settings->endGroup();
        
        // 加载依赖关系
        m_settings->beginGroup("Dependencies");
        for (const QString& moduleName : moduleNames) {
            if (m_settings->contains(moduleName)) {
                QStringList depList = m_settings->value(moduleName).toStringList();
                QList<ModuleDependency> dependencies;
                
                for (const QString& depStr : depList) {
                    QStringList parts = depStr.split("|");
                    if (parts.size() >= 2) {
                        ModuleDependency dep;
                        dep.moduleName = parts[0];
                        dep.requiredVersion = parts[1];
                        dep.isOptional = parts.size() > 2 ? parts[2] == "optional" : false;
                        dep.description = parts.size() > 3 ? parts[3] : "";
                        dependencies.append(dep);
                    }
                }
                m_dependencies[moduleName] = dependencies;
            }
        }
        m_settings->endGroup();
        
        m_configurationChanged = false;
        emit configurationLoaded();
        
        qDebug() << "Configuration loaded successfully. Modules count:" << m_modules.size();
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to load configuration:" << e.what();
        emit configurationError(QString("Failed to load configuration: %1").arg(e.what()));
        return false;
    }
}

bool GlobalModuleConfig::saveConfiguration()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        // 保存模块信息
        m_settings->beginGroup("Modules");
        m_settings->remove(""); // 清空现有配置
        
        for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
            const QString& moduleName = it.key();
            const ModuleInfo& info = it.value();
            
            m_settings->beginGroup(moduleName);
            m_settings->setValue("version", info.version);
            m_settings->setValue("description", info.description);
            m_settings->setValue("enabled", info.enabled);
            m_settings->setValue("priority", info.priority);
            m_settings->setValue("dependencies", info.dependencies);
            m_settings->setValue("lastModified", QDateTime::currentDateTime());
            
            // 保存配置数据
            m_settings->beginGroup("Config");
            for (auto configIt = info.configuration.begin(); configIt != info.configuration.end(); ++configIt) {
                m_settings->setValue(configIt.key(), configIt.value());
            }
            m_settings->endGroup();
            
            m_settings->endGroup();
        }
        m_settings->endGroup();
        
        // 保存依赖关系
        m_settings->beginGroup("Dependencies");
        m_settings->remove(""); // 清空现有依赖
        
        for (auto it = m_dependencies.begin(); it != m_dependencies.end(); ++it) {
            const QString& moduleName = it.key();
            const QList<ModuleDependency>& deps = it.value();
            
            QStringList depStrings;
            for (const ModuleDependency& dep : deps) {
                QString depStr = QString("%1|%2|%3|%4")
                    .arg(dep.moduleName)
                    .arg(dep.requiredVersion)
                    .arg(dep.isOptional ? "optional" : "required")
                    .arg(dep.description);
                depStrings.append(depStr);
            }
            m_settings->setValue(moduleName, depStrings);
        }
        m_settings->endGroup();
        
        m_settings->sync();
        m_configurationChanged = false;
        emit configurationSaved();
        
        qDebug() << "Configuration saved successfully";
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to save configuration:" << e.what();
        emit configurationError(QString("Failed to save configuration: %1").arg(e.what()));
        return false;
    }
}

bool GlobalModuleConfig::isModuleEnabled(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_modules.value(moduleName).enabled;
}

void GlobalModuleConfig::setModuleEnabled(const QString& moduleName, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_modules.contains(moduleName)) {
        if (m_modules[moduleName].enabled != enabled) {
            m_modules[moduleName].enabled = enabled;
            m_modules[moduleName].lastModified = QDateTime::currentDateTime();
            m_configurationChanged = true;
            
            if (enabled) {
                emit moduleEnabled(moduleName);
            } else {
                emit moduleDisabled(moduleName);
            }
            emit configurationChanged();
        }
    }
}

QStringList GlobalModuleConfig::getEnabledModules() const
{
    QMutexLocker locker(&m_mutex);
    QStringList enabled;
    
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        if (it.value().enabled) {
            enabled.append(it.key());
        }
    }
    
    return enabled;
}

QStringList GlobalModuleConfig::getAvailableModules() const
{
    QMutexLocker locker(&m_mutex);
    return m_modules.keys();
}

GlobalModuleConfig::ModuleInfo GlobalModuleConfig::getModuleInfo(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_modules.value(moduleName);
}

void GlobalModuleConfig::registerModule(const QString& moduleName, const ModuleInfo& info)
{
    QMutexLocker locker(&m_mutex);
    
    m_modules[moduleName] = info;
    m_configurationChanged = true;
    
    emit moduleRegistered(moduleName);
    emit configurationChanged();
    
    qDebug() << "Module registered:" << moduleName << "version:" << info.version;
}

bool GlobalModuleConfig::validateDependencies(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    QStringList visited;
    return validateModuleDependencies(moduleName, visited);
}

bool GlobalModuleConfig::validateModuleDependencies(const QString& moduleName, QStringList& visited) const
{
    if (visited.contains(moduleName)) {
        // 检测到循环依赖
        return false;
    }
    
    visited.append(moduleName);
    
    if (!m_dependencies.contains(moduleName)) {
        return true; // 没有依赖，验证通过
    }
    
    const QList<ModuleDependency>& deps = m_dependencies[moduleName];
    for (const ModuleDependency& dep : deps) {
        if (!dep.isOptional && !m_modules.contains(dep.moduleName)) {
            return false; // 必需依赖不存在
        }
        
        if (m_modules.contains(dep.moduleName)) {
            if (!validateModuleDependencies(dep.moduleName, visited)) {
                return false; // 递归验证失败
            }
        }
    }
    
    return true;
}

void GlobalModuleConfig::onAutoSave()
{
    if (m_configurationChanged) {
        saveConfiguration();
    }
}

QVariant GlobalModuleConfig::getConfigValue(const QString& moduleName, const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_modules.contains(moduleName)) {
        return m_modules[moduleName].configuration.value(key, defaultValue);
    }
    
    return defaultValue;
}

void GlobalModuleConfig::setConfigValue(const QString& moduleName, const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_modules.contains(moduleName)) {
        m_modules[moduleName].configuration[key] = value;
        m_modules[moduleName].lastModified = QDateTime::currentDateTime();
        m_configurationChanged = true;
        emit configurationChanged();
    }
}