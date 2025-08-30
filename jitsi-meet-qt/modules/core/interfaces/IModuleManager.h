#ifndef IMODULEMANAGER_H
#define IMODULEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @brief 模块管理器接口
 * 
 * 定义模块管理的核心接口，包括模块加载、卸载、状态管理等功能
 */
class IModuleManager : public QObject
{
    Q_OBJECT

public:
    enum ModuleStatus {
        NotLoaded,      // 未加载
        Loading,        // 加载中
        Loaded,         // 已加载
        Initializing,   // 初始化中
        Ready,          // 就绪
        Error,          // 错误
        Unloading       // 卸载中
    };
    Q_ENUM(ModuleStatus)

    enum LoadPriority {
        Critical = 0,   // 关键模块
        High = 1,       // 高优先级
        Normal = 2,     // 普通优先级
        Low = 3         // 低优先级
    };
    Q_ENUM(LoadPriority)

    virtual ~IModuleManager() = default;

    // 模块生命周期管理
    virtual bool loadModule(const QString& moduleName) = 0;
    virtual bool unloadModule(const QString& moduleName) = 0;
    virtual bool reloadModule(const QString& moduleName) = 0;
    virtual bool isModuleLoaded(const QString& moduleName) const = 0;

    // 模块状态查询
    virtual ModuleStatus getModuleStatus(const QString& moduleName) const = 0;
    virtual QStringList getLoadedModules() const = 0;
    virtual QStringList getAvailableModules() const = 0;
    virtual QStringList getFailedModules() const = 0;

    // 模块配置管理
    virtual bool enableModule(const QString& moduleName, bool enabled = true) = 0;
    virtual bool isModuleEnabled(const QString& moduleName) const = 0;
    virtual void setModulePriority(const QString& moduleName, LoadPriority priority) = 0;
    virtual LoadPriority getModulePriority(const QString& moduleName) const = 0;

    // 依赖关系管理
    virtual QStringList getModuleDependencies(const QString& moduleName) const = 0;
    virtual QStringList getModuleDependents(const QString& moduleName) const = 0;
    virtual bool validateDependencies(const QString& moduleName) const = 0;

    // 批量操作
    virtual bool loadAllModules() = 0;
    virtual bool unloadAllModules() = 0;
    virtual void loadModulesByPriority() = 0;

signals:
    void moduleLoaded(const QString& moduleName);
    void moduleUnloaded(const QString& moduleName);
    void moduleStatusChanged(const QString& moduleName, ModuleStatus status);
    void moduleError(const QString& moduleName, const QString& error);
    void allModulesLoaded();
    void dependencyError(const QString& moduleName, const QStringList& missingDependencies);
};

#endif // IMODULEMANAGER_H