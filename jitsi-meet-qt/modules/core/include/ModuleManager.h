#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "interfaces/IModuleManager.h"
#include "GlobalModuleConfig.h"
#include "ModuleHealthMonitor.h"
#include "ModuleVersionManager.h"
#include "management/RuntimeController.h"
#include <QObject>
#include <QTimer>
#include <QMutex>

/**
 * @brief 主模块管理器
 * 
 * 统一管理所有模块的生命周期、配置、健康状态和版本
 */
class ModuleManager : public IModuleManager
{
    Q_OBJECT

public:
    static ModuleManager* instance();
    ~ModuleManager();

    // IModuleManager接口实现
    bool loadModule(const QString& moduleName) override;
    bool unloadModule(const QString& moduleName) override;
    bool reloadModule(const QString& moduleName) override;
    bool isModuleLoaded(const QString& moduleName) const override;

    ModuleStatus getModuleStatus(const QString& moduleName) const override;
    QStringList getLoadedModules() const override;
    QStringList getAvailableModules() const override;
    QStringList getFailedModules() const override;

    bool enableModule(const QString& moduleName, bool enabled = true) override;
    bool isModuleEnabled(const QString& moduleName) const override;
    void setModulePriority(const QString& moduleName, LoadPriority priority) override;
    LoadPriority getModulePriority(const QString& moduleName) const override;

    QStringList getModuleDependencies(const QString& moduleName) const override;
    QStringList getModuleDependents(const QString& moduleName) const override;
    bool validateDependencies(const QString& moduleName) const override;

    bool loadAllModules() override;
    bool unloadAllModules() override;
    void loadModulesByPriority() override;

    // 扩展功能
    GlobalModuleConfig* getGlobalConfig() const;
    ModuleHealthMonitor* getHealthMonitor() const;
    ModuleVersionManager* getVersionManager() const;
    RuntimeController* getRuntimeController() const;

    // 系统管理
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // 统计信息
    int getTotalModuleCount() const;
    int getLoadedModuleCount() const;
    int getEnabledModuleCount() const;
    int getFailedModuleCount() const;
    
    // 批量操作
    bool enableModules(const QStringList& moduleNames, bool enabled = true);
    bool loadModules(const QStringList& moduleNames);
    bool unloadModules(const QStringList& moduleNames);
    
    // 配置管理
    bool saveConfiguration();
    bool loadConfiguration();
    bool resetConfiguration();
    bool exportConfiguration(const QString& filePath);
    bool importConfiguration(const QString& filePath);

public slots:
    void onModuleHealthChanged(const QString& moduleName, IHealthMonitor::HealthStatus status);
    void onModuleVersionChanged(const QString& moduleName, const QVersionNumber& oldVersion, const QVersionNumber& newVersion);
    void onConfigurationChanged();

private slots:
    void performSystemCheck();
    void updateModuleStatuses();

private:
    explicit ModuleManager(QObject* parent = nullptr);
    
    void initializeSubsystems();
    void connectSignals();
    void registerBuiltinModules();
    
    bool loadModuleInternal(const QString& moduleName);
    bool unloadModuleInternal(const QString& moduleName);
    void updateModuleStatus(const QString& moduleName, ModuleStatus status);
    
    bool checkModuleDependencies(const QString& moduleName) const;
    QStringList resolveDependencyOrder(const QStringList& moduleNames) const;
    bool hasCircularDependency(const QString& moduleName, QStringList& visited) const;
    
    void startSystemMonitoring();
    void stopSystemMonitoring();

    static ModuleManager* s_instance;
    static QMutex s_mutex;

    GlobalModuleConfig* m_globalConfig;
    ModuleHealthMonitor* m_healthMonitor;
    ModuleVersionManager* m_versionManager;
    RuntimeController* m_runtimeController;
    
    QMap<QString, ModuleStatus> m_moduleStatuses;
    QStringList m_loadedModules;
    QStringList m_failedModules;
    
    QTimer* m_systemCheckTimer;
    bool m_initialized;
    bool m_shutdownInProgress;
    
    mutable QMutex m_mutex;
};

#endif // MODULEMANAGER_H