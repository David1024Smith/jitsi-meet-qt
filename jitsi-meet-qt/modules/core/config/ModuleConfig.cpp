#include "ModuleConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>

ModuleConfig::ModuleConfig(const QString& moduleName, QObject* parent)
    : IModuleConfig(parent)
    , m_moduleName(moduleName)
    , m_moduleVersion("1.0.0")
    , m_moduleDescription(QString("Configuration for %1 module").arg(moduleName))
    , m_enabled(true)
    , m_scope(Global)
    , m_settings(nullptr)
{
    initializeSettings();
    load();
}

ModuleConfig::~ModuleConfig()
{
    save();
}

QString ModuleConfig::moduleName() const
{
    return m_moduleName;
}

QString ModuleConfig::moduleVersion() const
{
    return m_moduleVersion;
}

QString ModuleConfig::moduleDescription() const
{
    return m_moduleDescription;
}

bool ModuleConfig::isEnabled() const
{
    return m_enabled;
}

void ModuleConfig::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit configChanged("enabled", enabled);
        notifyConfigChanged();
    }
}

QVariantMap ModuleConfig::toVariantMap() const
{
    QVariantMap map = m_configData;
    map["moduleName"] = m_moduleName;
    map["moduleVersion"] = m_moduleVersion;
    map["moduleDescription"] = m_moduleDescription;
    map["enabled"] = m_enabled;
    map["scope"] = static_cast<int>(m_scope);
    return map;
}

void ModuleConfig::fromVariantMap(const QVariantMap& map)
{
    m_moduleName = map.value("moduleName", m_moduleName).toString();
    m_moduleVersion = map.value("moduleVersion", m_moduleVersion).toString();
    m_moduleDescription = map.value("moduleDescription", m_moduleDescription).toString();
    m_enabled = map.value("enabled", m_enabled).toBool();
    m_scope = static_cast<ConfigScope>(map.value("scope", static_cast<int>(m_scope)).toInt());
    
    // 复制其他配置数据
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (!QStringList{"moduleName", "moduleVersion", "moduleDescription", "enabled", "scope"}.contains(it.key())) {
            m_configData[it.key()] = it.value();
        }
    }
    
    notifyConfigChanged();
}

QJsonObject ModuleConfig::toJsonObject() const
{
    QVariantMap map = toVariantMap();
    return QJsonObject::fromVariantMap(map);
}

void ModuleConfig::fromJsonObject(const QJsonObject& json)
{
    QVariantMap map = json.toVariantMap();
    fromVariantMap(map);
}

bool ModuleConfig::validate() const
{
    QStringList errors = getValidationErrors();
    return errors.isEmpty();
}

QStringList ModuleConfig::getValidationErrors() const
{
    QStringList errors;
    
    if (m_moduleName.isEmpty()) {
        errors.append("Module name cannot be empty");
    }
    
    if (m_moduleVersion.isEmpty()) {
        errors.append("Module version cannot be empty");
    }
    
    // 可以添加更多验证规则
    
    return errors;
}

bool ModuleConfig::hasRequiredFields() const
{
    return !m_moduleName.isEmpty() && !m_moduleVersion.isEmpty();
}

bool ModuleConfig::save()
{
    if (!m_settings) {
        return false;
    }
    
    try {
        m_settings->setValue("moduleVersion", m_moduleVersion);
        m_settings->setValue("moduleDescription", m_moduleDescription);
        m_settings->setValue("enabled", m_enabled);
        m_settings->setValue("scope", static_cast<int>(m_scope));
        
        // 保存配置数据
        m_settings->beginGroup("ConfigData");
        for (auto it = m_configData.begin(); it != m_configData.end(); ++it) {
            m_settings->setValue(it.key(), it.value());
        }
        m_settings->endGroup();
        
        m_settings->sync();
        emit configSaved();
        
        qDebug() << "Configuration saved for module:" << m_moduleName;
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to save configuration for module" << m_moduleName << ":" << e.what();
        return false;
    }
}

bool ModuleConfig::load()
{
    if (!m_settings) {
        return false;
    }
    
    try {
        m_moduleVersion = m_settings->value("moduleVersion", m_moduleVersion).toString();
        m_moduleDescription = m_settings->value("moduleDescription", m_moduleDescription).toString();
        m_enabled = m_settings->value("enabled", m_enabled).toBool();
        m_scope = static_cast<ConfigScope>(m_settings->value("scope", static_cast<int>(m_scope)).toInt());
        
        // 加载配置数据
        m_configData.clear();
        m_settings->beginGroup("ConfigData");
        QStringList keys = m_settings->childKeys();
        for (const QString& key : keys) {
            m_configData[key] = m_settings->value(key);
        }
        m_settings->endGroup();
        
        emit configLoaded();
        
        qDebug() << "Configuration loaded for module:" << m_moduleName;
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to load configuration for module" << m_moduleName << ":" << e.what();
        return false;
    }
}

bool ModuleConfig::reset()
{
    m_moduleVersion = "1.0.0";
    m_moduleDescription = QString("Configuration for %1 module").arg(m_moduleName);
    m_enabled = true;
    m_scope = Global;
    m_configData.clear();
    
    emit configReset();
    notifyConfigChanged();
    
    qDebug() << "Configuration reset for module:" << m_moduleName;
    return true;
}

bool ModuleConfig::backup()
{
    if (!m_settings) {
        return false;
    }
    
    QString backupPath = m_configFilePath + ".backup";
    QSettings backup(backupPath, QSettings::IniFormat);
    
    // 复制所有设置
    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        backup.setValue(key, m_settings->value(key));
    }
    
    backup.sync();
    qDebug() << "Configuration backup created for module:" << m_moduleName;
    return true;
}

bool ModuleConfig::restore()
{
    QString backupPath = m_configFilePath + ".backup";
    if (!QFile::exists(backupPath)) {
        qWarning() << "No backup file found for module:" << m_moduleName;
        return false;
    }
    
    QSettings backup(backupPath, QSettings::IniFormat);
    
    // 恢复所有设置
    QStringList keys = backup.allKeys();
    for (const QString& key : keys) {
        m_settings->setValue(key, backup.value(key));
    }
    
    m_settings->sync();
    load(); // 重新加载配置
    
    qDebug() << "Configuration restored for module:" << m_moduleName;
    return true;
}

IModuleConfig::ConfigScope ModuleConfig::getScope() const
{
    return m_scope;
}

void ModuleConfig::setScope(ConfigScope scope)
{
    if (m_scope != scope) {
        m_scope = scope;
        emit configChanged("scope", static_cast<int>(scope));
        notifyConfigChanged();
    }
}

void ModuleConfig::notifyConfigChanged()
{
    // 可以在这里添加额外的通知逻辑
    qDebug() << "Configuration changed for module:" << m_moduleName;
}

void ModuleConfig::setModuleVersion(const QString& version)
{
    if (m_moduleVersion != version) {
        m_moduleVersion = version;
        emit configChanged("moduleVersion", version);
        notifyConfigChanged();
    }
}

void ModuleConfig::setModuleDescription(const QString& description)
{
    if (m_moduleDescription != description) {
        m_moduleDescription = description;
        emit configChanged("moduleDescription", description);
        notifyConfigChanged();
    }
}

QVariant ModuleConfig::getValue(const QString& key, const QVariant& defaultValue) const
{
    return m_configData.value(key, defaultValue);
}

void ModuleConfig::setValue(const QString& key, const QVariant& value)
{
    if (m_configData.value(key) != value) {
        m_configData[key] = value;
        emit configChanged(key, value);
        notifyConfigChanged();
    }
}

QStringList ModuleConfig::getKeys() const
{
    return m_configData.keys();
}

void ModuleConfig::removeKey(const QString& key)
{
    if (m_configData.contains(key)) {
        m_configData.remove(key);
        emit configChanged(key, QVariant());
        notifyConfigChanged();
    }
}

void ModuleConfig::clear()
{
    if (!m_configData.isEmpty()) {
        m_configData.clear();
        notifyConfigChanged();
    }
}

void ModuleConfig::initializeSettings()
{
    m_configFilePath = getConfigFilePath();
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
}

QString ModuleConfig::getConfigFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    return configDir + QString("/module_%1.conf").arg(m_moduleName);
}