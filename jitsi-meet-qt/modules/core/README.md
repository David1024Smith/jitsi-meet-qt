# Core Module Management System

## 概述

Core模块管理系统是Jitsi Meet Qt模块化架构的核心组件，提供统一的模块配置、健康监控、版本管理和运行时控制功能。

## 主要组件

### 1. ModuleManager (模块管理器)
- 统一的模块生命周期管理
- 模块加载、卸载、重载功能
- 依赖关系管理和验证
- 模块状态监控

### 2. GlobalModuleConfig (全局模块配置)
- 统一的配置管理接口
- 模块启用/禁用控制
- 配置持久化和同步
- 依赖关系管理

### 3. ModuleHealthMonitor (健康监控器)
- 实时健康状态检查
- 性能监控和资源跟踪
- 自动恢复机制
- 健康报告和历史记录

### 4. ModuleVersionManager (版本管理器)
- 版本兼容性检查
- 自动更新检测
- 升级和回滚机制
- 依赖版本管理

### 5. RuntimeController (运行时控制器)
- 运行时模块启用/禁用
- 异步操作队列
- 安全控制和回滚
- 批量操作支持

## 功能特性

### 模块配置管理
- ✅ 全局模块配置统一管理
- ✅ 模块启用/禁用的运行时控制
- ✅ 配置验证和默认值管理
- ✅ 配置文件的导入导出
- ✅ 自动保存和备份机制

### 健康检查和监控
- ✅ 多类型健康检查（基础、性能、资源、连接、功能）
- ✅ 实时状态监控和阈值管理
- ✅ 自动恢复和故障处理
- ✅ 健康历史记录和统计
- ✅ 异步检查和线程池支持

### 版本管理和升级
- ✅ 版本兼容性验证
- ✅ 自动更新检测和通知
- ✅ 渐进式升级策略
- ✅ 版本回滚机制
- ✅ 依赖版本冲突解决

### 运行时控制
- ✅ 热插拔模块支持
- ✅ 异步操作队列
- ✅ 操作确认和安全模式
- ✅ 批量操作和优先级控制
- ✅ 操作超时和错误处理

## 使用示例

### 基本使用

```cpp
#include "ModuleManager.h"

// 获取模块管理器实例
ModuleManager* manager = ModuleManager::instance();

// 初始化系统
if (!manager->initialize()) {
    qCritical() << "Failed to initialize module manager";
    return -1;
}

// 加载模块
if (manager->loadModule("audio")) {
    qDebug() << "Audio module loaded successfully";
}

// 检查模块状态
if (manager->isModuleLoaded("audio")) {
    auto status = manager->getModuleStatus("audio");
    qDebug() << "Audio module status:" << status;
}

// 启用/禁用模块
manager->enableModule("network", true);
manager->enableModule("chat", false);

// 按优先级加载所有模块
manager->loadModulesByPriority();
```

### 配置管理

```cpp
// 获取全局配置
GlobalModuleConfig* config = manager->getGlobalConfig();

// 设置模块配置
config->setConfigValue("audio", "sampleRate", 44100);
config->setConfigValue("audio", "channels", 2);

// 获取配置值
int sampleRate = config->getConfigValue("audio", "sampleRate", 48000).toInt();

// 保存配置
config->saveConfiguration();
```

### 健康监控

```cpp
// 获取健康监控器
ModuleHealthMonitor* monitor = manager->getHealthMonitor();

// 开始监控模块
monitor->startMonitoring("network");

// 设置健康阈值
monitor->setHealthThreshold("network", IHealthMonitor::Warning);
monitor->setPerformanceThreshold("network", 70.0);

// 启用自动恢复
monitor->enableAutoRecovery("network", true);

// 手动检查健康状态
auto report = monitor->checkModuleHealth("network");
qDebug() << "Network health:" << report.status << "Score:" << report.score;
```

### 版本管理

```cpp
// 获取版本管理器
ModuleVersionManager* versionMgr = manager->getVersionManager();

// 检查当前版本
QVersionNumber currentVersion = versionMgr->getModuleVersion("audio");
qDebug() << "Current audio version:" << currentVersion.toString();

// 检查更新
auto updates = versionMgr->checkForUpdates();
for (const auto& update : updates) {
    qDebug() << "Update available for" << update.moduleName 
             << ":" << update.targetVersion.toString();
}

// 启用自动升级
versionMgr->setAutoUpgrade("audio", true);
versionMgr->setUpgradePolicy("audio", IVersionManager::Minor);
```

### 运行时控制

```cpp
// 获取运行时控制器
RuntimeController* controller = manager->getRuntimeController();

// 异步启用模块
controller->enableModule("screenshare", RuntimeController::Asynchronous);

// 批量操作
QStringList modules = {"chat", "meeting", "screenshare"};
controller->enableModules(modules);

// 设置安全模式
controller->setSafeMode(true);
controller->setRequireConfirmation(true);
```

## 配置文件

### 模块配置文件 (modules.conf)
```ini
[Global]
Version=1.0.0
AutoSave=true
AutoSaveInterval=30000

[Modules/audio]
version=1.0.0
description=Audio processing module
enabled=true
priority=1
dependencies=utils

[Modules/audio/Config]
sampleRate=44100
channels=2
bufferSize=1024
```

### 版本配置文件 (versions.conf)
```ini
[Versions/audio]
autoUpgrade=false
upgradePolicy=1
versionHistory=1.0.0,1.0.1,1.1.0

[Repositories/official]
url=https://updates.jitsi-meet-qt.org/api/versions
enabled=true
```

## 信号和事件

### ModuleManager信号
```cpp
// 模块生命周期
void moduleLoaded(const QString& moduleName);
void moduleUnloaded(const QString& moduleName);
void moduleStatusChanged(const QString& moduleName, ModuleStatus status);
void moduleError(const QString& moduleName, const QString& error);

// 依赖管理
void dependencyError(const QString& moduleName, const QStringList& missingDependencies);
void allModulesLoaded();
```

### 健康监控信号
```cpp
void healthStatusChanged(const QString& moduleName, HealthStatus status);
void healthCheckCompleted(const QString& moduleName, const HealthReport& report);
void recoveryTriggered(const QString& moduleName);
```

### 版本管理信号
```cpp
void versionChanged(const QString& moduleName, const QVersionNumber& oldVersion, const QVersionNumber& newVersion);
void upgradeAvailable(const QString& moduleName, const QVersionNumber& newVersion);
void upgradeCompleted(const QString& moduleName, bool success);
```

## 最佳实践

### 1. 模块初始化顺序
```cpp
// 1. 首先初始化ModuleManager
ModuleManager* manager = ModuleManager::instance();
manager->initialize();

// 2. 配置模块优先级
manager->setModulePriority("utils", IModuleManager::Critical);
manager->setModulePriority("audio", IModuleManager::High);
manager->setModulePriority("ui", IModuleManager::Normal);

// 3. 按优先级加载
manager->loadModulesByPriority();
```

### 2. 错误处理
```cpp
// 连接错误信号
connect(manager, &ModuleManager::moduleError, 
        [](const QString& moduleName, const QString& error) {
    qWarning() << "Module error:" << moduleName << error;
    
    // 尝试重新加载
    ModuleManager::instance()->reloadModule(moduleName);
});
```

### 3. 优雅关闭
```cpp
// 应用程序退出时
void Application::shutdown() {
    ModuleManager* manager = ModuleManager::instance();
    
    // 卸载所有模块
    manager->unloadAllModules();
    
    // 保存配置
    manager->saveConfiguration();
    
    // 关闭系统
    manager->shutdown();
}
```

## 性能考虑

- 健康检查使用线程池避免阻塞主线程
- 配置变更使用延迟保存减少I/O操作
- 版本检查支持缓存机制
- 模块加载支持异步操作

## 扩展性

系统设计支持：
- 自定义健康检查类型
- 插件式版本仓库
- 自定义升级策略
- 模块间通信机制

## 故障排除

### 常见问题

1. **模块加载失败**
   - 检查依赖关系是否满足
   - 验证模块配置是否正确
   - 查看错误日志获取详细信息

2. **健康检查失败**
   - 检查模块是否正常响应
   - 验证性能阈值设置
   - 查看健康报告详细信息

3. **版本升级失败**
   - 检查网络连接
   - 验证版本兼容性
   - 确认升级权限

### 调试模式

```cpp
// 启用详细日志
QLoggingCategory::setFilterRules("*.debug=true");

// 启用健康检查调试
monitor->setMonitoringInterval(5000); // 5秒检查一次

// 启用版本检查调试
versionMgr->setUpdateCheckInterval(60000); // 1分钟检查一次
```