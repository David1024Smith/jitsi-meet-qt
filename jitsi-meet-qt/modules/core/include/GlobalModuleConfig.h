#ifndef GLOBALMODULECONFIG_H
#define GLOBALMODULECONFIG_H

#include "interfaces/IModuleConfig.h"
#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QTimer>

/**
 * @brief 全局模块配置管理器
 * 
 * 管理所有模块的全局配置，提供统一的配置接口和持久化机制
 */
class GlobalModuleConfig : public QObject
{
    Q_OBJECT

public:
    struct ModuleDependency {
        QString moduleName;
        QString requiredVersion;
        bool isOptional;
        QString description;
    };

    struct ModuleInfo {
        QString name;
        QString version;
        QString description;
        bool enabled;
        int priority;
        QStringList dependencies;
        QVariantMap configuration;
        QDateTime lastModified;
    };

    static GlobalModuleConfig* instance();
    ~GlobalModuleConfig();

    // 配置文件管理
    bool loadConfiguration();
    bool saveConfiguration();
    bool resetConfiguration();
    bool backupConfiguration();
    bool restoreConfiguration();

    // 模块状态管理
    bool isModuleEnabled(const QString& moduleName) const;
    void setModuleEnabled(const QString& moduleName, bool enabled);
    QStringList getEnabledModules() const;
    QStringList getAvailableModules() const;
    QStringList getDisabledModules() const;

    // 模块信息管理
    ModuleInfo getModuleInfo(const QString& moduleName) const;
    void setModuleInfo(const QString& moduleName, const ModuleInfo& info);
    bool hasModule(const QString& moduleName) const;
    void registerModule(const QString& moduleName, const ModuleInfo& info);
    void unregisterModule(const QString& moduleName);

    // 依赖关系管理
    QList<ModuleDependency> getModuleDependencies(const QString& moduleName) const;
    void setModuleDependencies(const QString& moduleName, const QList<ModuleDependency>& dependencies);
    bool validateDependencies(const QString& moduleName) const;
    QStringList getMissingDependencies(const QString& moduleName) const;
    QStringList getCircularDependencies() const;

    // 优先级管理
    void setModulePriority(const QString& moduleName, int priority);
    int getModulePriority(const QString& moduleName) const;
    QStringList getModulesByPriority() const;

    // 配置值管理
    QVariant getConfigValue(const QString& moduleName, const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setConfigValue(const QString& moduleName, const QString& key, const QVariant& value);
    QVariantMap getModuleConfig(const QString& moduleName) const;
    void setModuleConfig(const QString& moduleName, const QVariantMap& config);

    // 全局设置
    QString getConfigFilePath() const;
    void setConfigFilePath(const QString& path);
    bool isAutoSaveEnabled() const;
    void setAutoSaveEnabled(bool enabled);
    int getAutoSaveInterval() const;
    void setAutoSaveInterval(int intervalMs);

    // 验证和检查
    bool validateConfiguration() const;
    QStringList getConfigurationErrors() const;
    bool hasConfigurationChanged() const;
    void markConfigurationClean();

    // 导入导出
    bool exportConfiguration(const QString& filePath) const;
    bool importConfiguration(const QString& filePath);
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject& json);

public slots:
    void onAutoSave();
    void onModuleRegistered(const QString& moduleName);
    void onModuleUnregistered(const QString& moduleName);

signals:
    void configurationLoaded();
    void configurationSaved();
    void configurationChanged();
    void moduleEnabled(const QString& moduleName);
    void moduleDisabled(const QString& moduleName);
    void moduleRegistered(const QString& moduleName);
    void moduleUnregistered(const QString& moduleName);
    void dependencyError(const QString& moduleName, const QStringList& missingDependencies);
    void configurationError(const QString& error);

private:
    explicit GlobalModuleConfig(QObject* parent = nullptr);
    void initializeDefaults();
    void setupAutoSave();
    bool validateModuleDependencies(const QString& moduleName, QStringList& visited) const;

    static GlobalModuleConfig* s_instance;
    static QMutex s_mutex;

    QSettings* m_settings;
    QMap<QString, ModuleInfo> m_modules;
    QMap<QString, QList<ModuleDependency>> m_dependencies;
    QString m_configFilePath;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    QTimer* m_autoSaveTimer;
    bool m_configurationChanged;
    mutable QMutex m_mutex;
};

Q_DECLARE_METATYPE(GlobalModuleConfig::ModuleDependency)
Q_DECLARE_METATYPE(GlobalModuleConfig::ModuleInfo)

#endif // GLOBALMODULECONFIG_H