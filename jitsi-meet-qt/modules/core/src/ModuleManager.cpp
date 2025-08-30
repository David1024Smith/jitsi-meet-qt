#include "ModuleManager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

ModuleManager* ModuleManager::s_instance = nullptr;
QMutex ModuleManager::s_mutex;

ModuleManager* ModuleManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ModuleManager();
    }
    return s_instance;
}

ModuleManager::ModuleManager(QObject* parent)
    : IModuleManager(parent)
    , m_globalConfig(nullptr)
    , m_healthMonitor(nullptr)
    , m_versionManager(nullptr)
    , m_runtimeController(nullptr)
    , m_systemCheckTimer(new QTimer(this))
    , m_initialized(false)
    , m_shutdownInProgress(false)
{
    // 初始化子系统
    initializeSubsystems();
    
    // 连接信号
    connectSignals();
    
    // 设置系统检查定时器
    m_systemCheckTimer->setSingleShot(false);
    m_systemCheckTimer->setInterval(60000); // 1分钟
    connect(m_systemCheckTimer, &QTimer::timeout, this, &ModuleManager::performSystemCheck);
    
    qDebug() << "ModuleManager created";
}

ModuleManager::~ModuleManager()
{
    shutdown();
}

void ModuleManager::initializeSubsystems()
{
    // 创建全局配置管理器
    m_globalConfig = GlobalModuleConfig::instance();
    
    // 创建健康监控器
    m_healthMonitor = new ModuleHealthMonitor(this);
    
    // 创建版本管理器
    m_versionManager = new ModuleVersionManager(this);
    
    // 创建运行时控制器
    m_runtimeController = new RuntimeController(this);
    
    qDebug() << "ModuleManager subsystems initialized";
}

void ModuleManager::connectSignals()
{
    // 连接全局配置信号
    connect(m_globalConfig, &GlobalModuleConfig::moduleEnabled,
            this, [this](const QString& moduleName) {
                updateModuleStatus(moduleName, Ready);
                emit moduleLoaded(moduleName);
            });
    
    connect(m_globalConfig, &GlobalModuleConfig::moduleDisabled,
            this, [this](const QString& moduleName) {
                updateModuleStatus(moduleName, NotLoaded);
                emit moduleUnloaded(moduleName);
            });
    
    connect(m_globalConfig, &GlobalModuleConfig::configurationChanged,
            this, &ModuleManager::onConfigurationChanged);
    
    // 连接健康监控信号
    connect(m_healthMonitor, &ModuleHealthMonitor::healthStatusChanged,
            this, &ModuleManager::onModuleHealthChanged);
    
    // 连接版本管理信号
    connect(m_versionManager, &ModuleVersionManager::versionChanged,
            this, &ModuleManager::onModuleVersionChanged);
    
    // 连接运行时控制器信号
    connect(m_runtimeController, &RuntimeController::operationCompleted,
            this, [this](const QString& moduleName, RuntimeController::ControlAction action, bool success) {
                if (success) {
                    switch (action) {
                    case RuntimeController::Enable:
                        updateModuleStatus(moduleName, Ready);
                        break;
                    case RuntimeController::Disable:
                        updateModuleStatus(moduleName, NotLoaded);
                        break;
                    case RuntimeController::Reload:
                        updateModuleStatus(moduleName, Ready);
                        break;
                    default:
                        break;
                    }
                } else {
                    updateModuleStatus(moduleName, Error);
                }
            });
}

bool ModuleManager::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    try {
        // 加载配置
        if (!loadConfiguration()) {
            qWarning() << "Failed to load module configuration";
            return false;
        }
        
        // 注册内置模块
        registerBuiltinModules();
        
        // 启动系统监控
        startSystemMonitoring();
        
        m_initialized = true;
        
        qDebug() << "ModuleManager initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "ModuleManager initialization failed:" << e.what();
        return false;
    }
}

void ModuleManager::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized || m_shutdownInProgress) {
        return;
    }
    
    m_shutdownInProgress = true;
    
    // 停止系统监控
    stopSystemMonitoring();
    
    // 卸载所有模块
    unloadAllModules();
    
    // 保存配置
    saveConfiguration();
    
    m_initialized = false;
    m_shutdownInProgress = false;
    
    qDebug() << "ModuleManager shutdown completed";
}

bool ModuleManager::loadModule(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (isModuleLoaded(moduleName)) {
        qDebug() << "Module already loaded:" << moduleName;
        return true;
    }
    
    // 检查依赖关系
    if (!checkModuleDependencies(moduleName)) {
        QStringList missing = getModuleDependencies(moduleName);
        emit dependencyError(moduleName, missing);
        qWarning() << "Dependency check failed for module:" << moduleName;
        return false;
    }
    
    return loadModuleInternal(moduleName);
}

bool ModuleManager::loadModuleInternal(const QString& moduleName)
{
    updateModuleStatus(moduleName, Loading);
    
    try {
        // 启用模块
        if (!m_globalConfig->isModuleEnabled(moduleName)) {
            m_globalConfig->setModuleEnabled(moduleName, true);
        }
        
        // 启动健康监控
        m_healthMonitor->startMonitoring(moduleName);
        
        // 更新状态
        updateModuleStatus(moduleName, Ready);
        m_loadedModules.append(moduleName);
        m_failedModules.removeAll(moduleName);
        
        emit moduleLoaded(moduleName);
        
        qDebug() << "Module loaded successfully:" << moduleName;
        return true;
        
    } catch (const std::exception& e) {
        updateModuleStatus(moduleName, Error);
        m_failedModules.append(moduleName);
        
        emit moduleError(moduleName, QString("Load failed: %1").arg(e.what()));
        
        qWarning() << "Failed to load module" << moduleName << ":" << e.what();
        return false;
    }
}

bool ModuleManager::unloadModule(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isModuleLoaded(moduleName)) {
        qDebug() << "Module not loaded:" << moduleName;
        return true;
    }
    
    return unloadModuleInternal(moduleName);
}

bool ModuleManager::unloadModuleInternal(const QString& moduleName)
{
    updateModuleStatus(moduleName, Unloading);
    
    try {
        // 停止健康监控
        m_healthMonitor->stopMonitoring(moduleName);
        
        // 禁用模块
        m_globalConfig->setModuleEnabled(moduleName, false);
        
        // 更新状态
        updateModuleStatus(moduleName, NotLoaded);
        m_loadedModules.removeAll(moduleName);
        
        emit moduleUnloaded(moduleName);
        
        qDebug() << "Module unloaded successfully:" << moduleName;
        return true;
        
    } catch (const std::exception& e) {
        updateModuleStatus(moduleName, Error);
        
        emit moduleError(moduleName, QString("Unload failed: %1").arg(e.what()));
        
        qWarning() << "Failed to unload module" << moduleName << ":" << e.what();
        return false;
    }
}

bool ModuleManager::reloadModule(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    // 先卸载再加载
    if (isModuleLoaded(moduleName)) {
        if (!unloadModuleInternal(moduleName)) {
            return false;
        }
    }
    
    return loadModuleInternal(moduleName);
}

bool ModuleManager::isModuleLoaded(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedModules.contains(moduleName);
}

IModuleManager::ModuleStatus ModuleManager::getModuleStatus(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    return m_moduleStatuses.value(moduleName, NotLoaded);
}

QStringList ModuleManager::getLoadedModules() const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedModules;
}

QStringList ModuleManager::getAvailableModules() const
{
    return m_globalConfig->getAvailableModules();
}

QStringList ModuleManager::getFailedModules() const
{
    QMutexLocker locker(&m_mutex);
    return m_failedModules;
}

bool ModuleManager::enableModule(const QString& moduleName, bool enabled)
{
    if (enabled) {
        return m_runtimeController->enableModule(moduleName);
    } else {
        return m_runtimeController->disableModule(moduleName);
    }
}

bool ModuleManager::isModuleEnabled(const QString& moduleName) const
{
    return m_globalConfig->isModuleEnabled(moduleName);
}

void ModuleManager::setModulePriority(const QString& moduleName, LoadPriority priority)
{
    m_globalConfig->setModulePriority(moduleName, static_cast<int>(priority));
}

IModuleManager::LoadPriority ModuleManager::getModulePriority(const QString& moduleName) const
{
    int priority = m_globalConfig->getModulePriority(moduleName);
    return static_cast<LoadPriority>(priority);
}

QStringList ModuleManager::getModuleDependencies(const QString& moduleName) const
{
    auto deps = m_globalConfig->getModuleDependencies(moduleName);
    QStringList result;
    for (const auto& dep : deps) {
        result.append(dep.moduleName);
    }
    return result;
}

bool ModuleManager::validateDependencies(const QString& moduleName) const
{
    return m_globalConfig->validateDependencies(moduleName);
}

bool ModuleManager::loadAllModules()
{
    QStringList modules = getAvailableModules();
    return loadModules(modules);
}

bool ModuleManager::unloadAllModules()
{
    QStringList modules = getLoadedModules();
    return unloadModules(modules);
}

void ModuleManager::loadModulesByPriority()
{
    QStringList modules = getAvailableModules();
    
    // 按优先级排序
    std::sort(modules.begin(), modules.end(), [this](const QString& a, const QString& b) {
        return getModulePriority(a) < getModulePriority(b);
    });
    
    // 按优先级加载
    for (const QString& moduleName : modules) {
        if (isModuleEnabled(moduleName)) {
            loadModule(moduleName);
        }
    }
}

void ModuleManager::updateModuleStatus(const QString& moduleName, ModuleStatus status)
{
    QMutexLocker locker(&m_mutex);
    
    ModuleStatus oldStatus = m_moduleStatuses.value(moduleName, NotLoaded);
    if (oldStatus != status) {
        m_moduleStatuses[moduleName] = status;
        emit moduleStatusChanged(moduleName, status);
    }
}

bool ModuleManager::checkModuleDependencies(const QString& moduleName) const
{
    QStringList dependencies = getModuleDependencies(moduleName);
    
    for (const QString& dep : dependencies) {
        if (!isModuleLoaded(dep)) {
            // 尝试加载依赖模块
            const_cast<ModuleManager*>(this)->loadModule(dep);
            
            // 再次检查
            if (!isModuleLoaded(dep)) {
                return false;
            }
        }
    }
    
    return true;
}

void ModuleManager::registerBuiltinModules()
{
    // 注册内置模块
    QStringList builtinModules = {
        "audio", "network", "ui", "performance", 
        "utils", "settings", "chat", "screenshare", "meeting"
    };
    
    for (const QString& moduleName : builtinModules) {
        GlobalModuleConfig::ModuleInfo info;
        info.name = moduleName;
        info.version = "1.0.0";
        info.description = QString("%1 module").arg(moduleName);
        info.enabled = true;
        info.priority = 2; // Normal priority
        
        m_globalConfig->registerModule(moduleName, info);
        updateModuleStatus(moduleName, NotLoaded);
    }
    
    qDebug() << "Builtin modules registered:" << builtinModules.size();
}

void ModuleManager::startSystemMonitoring()
{
    m_systemCheckTimer->start();
    qDebug() << "System monitoring started";
}

void ModuleManager::stopSystemMonitoring()
{
    m_systemCheckTimer->stop();
    qDebug() << "System monitoring stopped";
}

void ModuleManager::performSystemCheck()
{
    // 检查所有加载的模块状态
    updateModuleStatuses();
    
    // 检查失败的模块是否需要重启
    QStringList failedModules = getFailedModules();
    for (const QString& moduleName : failedModules) {
        if (m_healthMonitor->isAutoRecoveryEnabled(moduleName)) {
            qDebug() << "Attempting auto-recovery for failed module:" << moduleName;
            m_healthMonitor->triggerRecovery(moduleName);
        }
    }
}

void ModuleManager::updateModuleStatuses()
{
    QStringList modules = getAvailableModules();
    
    for (const QString& moduleName : modules) {
        if (isModuleEnabled(moduleName)) {
            // 检查模块健康状态
            auto healthReport = m_healthMonitor->checkModuleHealth(moduleName);
            
            if (healthReport.status == IHealthMonitor::Failure) {
                updateModuleStatus(moduleName, Error);
                if (!m_failedModules.contains(moduleName)) {
                    m_failedModules.append(moduleName);
                }
            } else if (healthReport.status == IHealthMonitor::Healthy) {
                updateModuleStatus(moduleName, Ready);
                m_failedModules.removeAll(moduleName);
            }
        }
    }
}

bool ModuleManager::saveConfiguration()
{
    return m_globalConfig->saveConfiguration();
}

bool ModuleManager::loadConfiguration()
{
    return m_globalConfig->loadConfiguration();
}

void ModuleManager::onModuleHealthChanged(const QString& moduleName, IHealthMonitor::HealthStatus status)
{
    if (status == IHealthMonitor::Failure || status == IHealthMonitor::Critical) {
        updateModuleStatus(moduleName, Error);
        emit moduleError(moduleName, "Health check failed");
    } else if (status == IHealthMonitor::Healthy) {
        updateModuleStatus(moduleName, Ready);
    }
}

void ModuleManager::onModuleVersionChanged(const QString& moduleName, const QVersionNumber& oldVersion, const QVersionNumber& newVersion)
{
    Q_UNUSED(oldVersion)
    Q_UNUSED(newVersion)
    
    // 版本变更后可能需要重新加载模块
    if (isModuleLoaded(moduleName)) {
        reloadModule(moduleName);
    }
}

void ModuleManager::onConfigurationChanged()
{
    // 配置变更，可能需要重新评估模块状态
    QTimer::singleShot(1000, this, &ModuleManager::updateModuleStatuses);
}

// 获取子系统的访问器
GlobalModuleConfig* ModuleManager::getGlobalConfig() const
{
    return m_globalConfig;
}

ModuleHealthMonitor* ModuleManager::getHealthMonitor() const
{
    return m_healthMonitor;
}

ModuleVersionManager* ModuleManager::getVersionManager() const
{
    return m_versionManager;
}

RuntimeController* ModuleManager::getRuntimeController() const
{
    return m_runtimeController;
}