#include "ModuleManager.h"
#include "GlobalModuleConfig.h"
#include "ModuleHealthMonitor.h"
#include "ModuleVersionManager.h"
#include "management/RuntimeController.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

/**
 * @brief Core Module Management System Usage Example
 * 
 * 演示如何使用核心模块管理系统的各项功能
 */
class CoreSystemExample : public QObject
{
    Q_OBJECT

public:
    CoreSystemExample(QObject* parent = nullptr) : QObject(parent) {}

    void runExample()
    {
        qDebug() << "=== Core Module Management System Example ===";
        
        // 1. 初始化模块管理器
        initializeSystem();
        
        // 2. 演示配置管理
        demonstrateConfigManagement();
        
        // 3. 演示运行时控制
        demonstrateRuntimeControl();
        
        // 4. 演示健康监控
        demonstrateHealthMonitoring();
        
        // 5. 演示版本管理
        demonstrateVersionManagement();
        
        // 6. 演示系统集成
        demonstrateSystemIntegration();
    }

private:
    void initializeSystem()
    {
        qDebug() << "\n1. Initializing Module Management System...";
        
        // 获取模块管理器实例
        m_moduleManager = ModuleManager::instance();
        
        // 初始化系统
        if (!m_moduleManager->initialize()) {
            qCritical() << "Failed to initialize module manager!";
            return;
        }
        
        // 获取子系统引用
        m_globalConfig = m_moduleManager->getGlobalConfig();
        m_healthMonitor = m_moduleManager->getHealthMonitor();
        m_versionManager = m_moduleManager->getVersionManager();
        m_runtimeController = m_moduleManager->getRuntimeController();
        
        qDebug() << "✓ Module management system initialized successfully";
        qDebug() << "✓ Available modules:" << m_moduleManager->getAvailableModules();
    }

    void demonstrateConfigManagement()
    {
        qDebug() << "\n2. Demonstrating Configuration Management...";
        
        // 注册自定义模块
        GlobalModuleConfig::ModuleInfo customModule;
        customModule.name = "example";
        customModule.version = "1.0.0";
        customModule.description = "Example module for demonstration";
        customModule.enabled = true;
        customModule.priority = 2;
        
        m_globalConfig->registerModule("example", customModule);
        qDebug() << "✓ Custom module registered";
        
        // 设置模块配置
        m_globalConfig->setConfigValue("example", "setting1", "value1");
        m_globalConfig->setConfigValue("example", "setting2", 42);
        m_globalConfig->setConfigValue("example", "setting3", true);
        
        // 读取配置
        QString setting1 = m_globalConfig->getConfigValue("example", "setting1").toString();
        int setting2 = m_globalConfig->getConfigValue("example", "setting2").toInt();
        bool setting3 = m_globalConfig->getConfigValue("example", "setting3").toBool();
        
        qDebug() << "✓ Configuration values:";
        qDebug() << "  - setting1:" << setting1;
        qDebug() << "  - setting2:" << setting2;
        qDebug() << "  - setting3:" << setting3;
        
        // 保存配置
        if (m_globalConfig->saveConfiguration()) {
            qDebug() << "✓ Configuration saved successfully";
        }
    }

    void demonstrateRuntimeControl()
    {
        qDebug() << "\n3. Demonstrating Runtime Control...";
        
        // 连接信号
        connect(m_runtimeController, &RuntimeController::operationStarted,
                [](const QString& moduleName, RuntimeController::ControlAction action) {
            qDebug() << "  → Operation started:" << moduleName << "action:" << action;
        });
        
        connect(m_runtimeController, &RuntimeController::operationCompleted,
                [](const QString& moduleName, RuntimeController::ControlAction action, bool success) {
            qDebug() << "  ✓ Operation completed:" << moduleName << "success:" << success;
        });
        
        // 同步启用模块
        qDebug() << "Enabling module synchronously...";
        bool result = m_runtimeController->enableModule("example", RuntimeController::Synchronous);
        qDebug() << "✓ Synchronous enable result:" << result;
        
        // 异步禁用模块
        qDebug() << "Disabling module asynchronously...";
        m_runtimeController->disableModule("example", RuntimeController::Asynchronous);
        
        // 批量操作
        QStringList modules = {"audio", "network"};
        qDebug() << "Enabling modules in batch:" << modules;
        m_runtimeController->enableModules(modules);
        
        // 设置安全模式
        m_runtimeController->setSafeMode(true);
        m_runtimeController->setRequireConfirmation(false); // 为演示目的禁用确认
        qDebug() << "✓ Safe mode enabled";
    }

    void demonstrateHealthMonitoring()
    {
        qDebug() << "\n4. Demonstrating Health Monitoring...";
        
        // 连接健康监控信号
        connect(m_healthMonitor, &ModuleHealthMonitor::healthStatusChanged,
                [](const QString& moduleName, IHealthMonitor::HealthStatus status) {
            qDebug() << "  → Health status changed:" << moduleName << "status:" << status;
        });
        
        connect(m_healthMonitor, &ModuleHealthMonitor::healthCheckCompleted,
                [](const QString& moduleName, const IHealthMonitor::HealthReport& report) {
            qDebug() << "  ✓ Health check completed:" << moduleName 
                     << "score:" << report.score << "status:" << report.status;
        });
        
        // 开始监控模块
        m_healthMonitor->startMonitoring("audio");
        m_healthMonitor->startMonitoring("network");
        qDebug() << "✓ Started monitoring modules: audio, network";
        
        // 设置健康阈值
        m_healthMonitor->setHealthThreshold("audio", IHealthMonitor::Warning);
        m_healthMonitor->setPerformanceThreshold("audio", 70.0);
        qDebug() << "✓ Health thresholds configured";
        
        // 启用自动恢复
        m_healthMonitor->enableAutoRecovery("audio", true);
        qDebug() << "✓ Auto-recovery enabled for audio module";
        
        // 执行健康检查
        auto report = m_healthMonitor->checkModuleHealth("audio");
        qDebug() << "✓ Manual health check - Audio module:";
        qDebug() << "  - Status:" << report.status;
        qDebug() << "  - Score:" << report.score;
        qDebug() << "  - Message:" << report.message;
        
        // 执行性能检查
        auto perfReport = m_healthMonitor->performHealthCheck("network", IHealthMonitor::Performance);
        qDebug() << "✓ Performance check - Network module:";
        qDebug() << "  - Status:" << perfReport.status;
        qDebug() << "  - Score:" << perfReport.score;
    }

    void demonstrateVersionManagement()
    {
        qDebug() << "\n5. Demonstrating Version Management...";
        
        // 连接版本管理信号
        connect(m_versionManager, &ModuleVersionManager::versionChanged,
                [](const QString& moduleName, const QVersionNumber& oldVersion, const QVersionNumber& newVersion) {
            qDebug() << "  → Version changed:" << moduleName 
                     << "from" << oldVersion.toString() << "to" << newVersion.toString();
        });
        
        connect(m_versionManager, &ModuleVersionManager::upgradeAvailable,
                [](const QString& moduleName, const QVersionNumber& newVersion) {
            qDebug() << "  → Upgrade available:" << moduleName << "version:" << newVersion.toString();
        });
        
        // 获取当前版本信息
        QVersionNumber audioVersion = m_versionManager->getModuleVersion("audio");
        qDebug() << "✓ Current audio version:" << audioVersion.toString();
        
        auto versionInfo = m_versionManager->getVersionInfo("audio");
        qDebug() << "✓ Audio version info:";
        qDebug() << "  - Version:" << versionInfo.version.toString();
        qDebug() << "  - Description:" << versionInfo.description;
        qDebug() << "  - Stable:" << versionInfo.isStable;
        qDebug() << "  - Compatible:" << versionInfo.isCompatible;
        
        // 检查版本兼容性
        QVersionNumber testVersion(1, 1, 0);
        bool compatible = m_versionManager->isVersionCompatible("audio", testVersion);
        qDebug() << "✓ Version" << testVersion.toString() << "compatibility:" << compatible;
        
        // 配置自动升级
        m_versionManager->setAutoUpgrade("audio", false); // 演示中禁用自动升级
        m_versionManager->setUpgradePolicy("audio", IVersionManager::Minor);
        qDebug() << "✓ Auto-upgrade policy configured";
        
        // 检查更新（异步）
        QTimer::singleShot(100, this, [this]() {
            auto updates = m_versionManager->checkForUpdates();
            qDebug() << "✓ Update check completed. Available updates:" << updates.size();
            
            for (const auto& update : updates) {
                qDebug() << "  - Module:" << update.moduleName 
                         << "Current:" << update.currentVersion.toString()
                         << "Target:" << update.targetVersion.toString();
            }
        });
    }

    void demonstrateSystemIntegration()
    {
        qDebug() << "\n6. Demonstrating System Integration...";
        
        // 获取系统统计信息
        int totalModules = m_moduleManager->getTotalModuleCount();
        int loadedModules = m_moduleManager->getLoadedModuleCount();
        int enabledModules = m_moduleManager->getEnabledModuleCount();
        int failedModules = m_moduleManager->getFailedModuleCount();
        
        qDebug() << "✓ System Statistics:";
        qDebug() << "  - Total modules:" << totalModules;
        qDebug() << "  - Loaded modules:" << loadedModules;
        qDebug() << "  - Enabled modules:" << enabledModules;
        qDebug() << "  - Failed modules:" << failedModules;
        
        // 按优先级加载所有模块
        qDebug() << "Loading all modules by priority...";
        m_moduleManager->loadModulesByPriority();
        
        // 显示加载的模块
        QStringList loaded = m_moduleManager->getLoadedModules();
        qDebug() << "✓ Loaded modules:" << loaded;
        
        // 保存系统配置
        if (m_moduleManager->saveConfiguration()) {
            qDebug() << "✓ System configuration saved";
        }
        
        // 演示完成
        qDebug() << "\n=== Core System Example Completed Successfully ===";
        
        // 延迟退出以观察异步操作
        QTimer::singleShot(2000, []() {
            QCoreApplication::quit();
        });
    }

private:
    ModuleManager* m_moduleManager = nullptr;
    GlobalModuleConfig* m_globalConfig = nullptr;
    ModuleHealthMonitor* m_healthMonitor = nullptr;
    ModuleVersionManager* m_versionManager = nullptr;
    RuntimeController* m_runtimeController = nullptr;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("CoreSystemExample");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    // 运行示例
    CoreSystemExample example;
    example.runExample();
    
    return app.exec();
}

#include "CoreSystemExample.moc"