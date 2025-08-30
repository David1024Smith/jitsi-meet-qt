#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include "PerformanceModule.h"
#include "PerformanceManager.h"
#include "MetricsCollector.h"
#include "PerformanceConfig.h"

/**
 * @brief 性能模块使用示例
 * 
 * 本示例演示如何使用性能模块的各种功能：
 * - 模块初始化和配置
 * - 性能监控和数据收集
 * - 自动优化和手动优化
 * - 数据导出和报告生成
 */
class PerformanceExample : public QObject
{
    Q_OBJECT

public:
    explicit PerformanceExample(QObject *parent = nullptr)
        : QObject(parent)
        , m_performanceModule(nullptr)
        , m_performanceManager(nullptr)
        , m_config(nullptr)
        , m_demoTimer(new QTimer(this))
    {
        connect(m_demoTimer, &QTimer::timeout, this, &PerformanceExample::runDemo);
    }

    void start()
    {
        qDebug() << "========================================";
        qDebug() << "Performance Module Example";
        qDebug() << "========================================";

        // 步骤1: 初始化性能模块
        initializePerformanceModule();

        // 步骤2: 配置性能监控
        configurePerformanceMonitoring();

        // 步骤3: 启动性能监控
        startPerformanceMonitoring();

        // 步骤4: 运行演示
        m_demoTimer->start(5000); // 每5秒运行一次演示
    }

private slots:
    void runDemo()
    {
        static int demoStep = 0;
        demoStep++;

        qDebug() << "\n--- Demo Step" << demoStep << "---";

        switch (demoStep) {
        case 1:
            demonstrateBasicMonitoring();
            break;
        case 2:
            demonstrateOptimization();
            break;
        case 3:
            demonstrateDataExport();
            break;
        case 4:
            demonstrateConfigurationManagement();
            break;
        case 5:
            demonstrateAdvancedFeatures();
            break;
        default:
            qDebug() << "Demo completed!";
            QCoreApplication::quit();
            break;
        }
    }

    void onMetricsUpdated(const PerformanceMetrics& metrics)
    {
        qDebug() << "Performance Metrics Updated:";
        qDebug() << "  CPU Usage:" << QString::number(metrics.system.cpuUsage, 'f', 1) << "%";
        qDebug() << "  Memory Usage:" << metrics.system.memoryUsage << "MB";
        qDebug() << "  Network Latency:" << QString::number(metrics.network.latency, 'f', 1) << "ms";
        qDebug() << "  Audio Latency:" << QString::number(metrics.audio.latency, 'f', 1) << "ms";
        qDebug() << "  Video Frame Rate:" << QString::number(metrics.video.frameRate, 'f', 1) << "fps";
    }

    void onPerformanceLevelChanged(PerformanceManager::PerformanceLevel level)
    {
        QString levelStr;
        switch (level) {
        case PerformanceManager::Excellent:
            levelStr = "Excellent";
            break;
        case PerformanceManager::Good:
            levelStr = "Good";
            break;
        case PerformanceManager::Fair:
            levelStr = "Fair";
            break;
        case PerformanceManager::Poor:
            levelStr = "Poor";
            break;
        case PerformanceManager::Critical:
            levelStr = "Critical";
            break;
        }
        qDebug() << "Performance Level Changed to:" << levelStr;
    }

    void onOptimizationCompleted(bool success, const QVariantMap& improvements)
    {
        qDebug() << "Optimization Completed:" << (success ? "Success" : "Failed");
        if (success && !improvements.isEmpty()) {
            qDebug() << "Improvements:";
            for (auto it = improvements.begin(); it != improvements.end(); ++it) {
                qDebug() << "  " << it.key() << ":" << it.value().toString();
            }
        }
    }

    void onThresholdExceeded(const QString& metricName, double value, double threshold)
    {
        qDebug() << "Threshold Exceeded!";
        qDebug() << "  Metric:" << metricName;
        qDebug() << "  Current Value:" << value;
        qDebug() << "  Threshold:" << threshold;
    }

private:
    void initializePerformanceModule()
    {
        qDebug() << "\n1. Initializing Performance Module...";

        // 获取性能模块实例
        m_performanceModule = PerformanceModule::instance();

        // 连接信号
        connect(m_performanceModule, &PerformanceModule::initialized,
                this, [](bool success) {
                    qDebug() << "Module initialized:" << (success ? "Success" : "Failed");
                });

        connect(m_performanceModule, &PerformanceModule::errorOccurred,
                this, [](const QString& error) {
                    qDebug() << "Module error:" << error;
                });

        // 初始化模块
        if (m_performanceModule->initialize()) {
            qDebug() << "Performance Module initialized successfully";
            qDebug() << "Version:" << m_performanceModule->version();

            // 获取组件
            m_performanceManager = m_performanceModule->performanceManager();
            m_config = m_performanceModule->config();
        } else {
            qDebug() << "Failed to initialize Performance Module";
        }
    }

    void configurePerformanceMonitoring()
    {
        qDebug() << "\n2. Configuring Performance Monitoring...";

        if (!m_config) {
            qDebug() << "Configuration not available";
            return;
        }

        // 配置监控参数
        m_config->setMonitoringEnabled(true);
        m_config->setMonitoringInterval(1000); // 1秒间隔
        m_config->setAutoOptimizationEnabled(true);
        m_config->setOptimizationInterval(30000); // 30秒间隔

        // 设置阈值
        m_config->setCpuThreshold(80.0); // CPU使用率80%
        m_config->setMemoryThreshold(1024); // 内存使用1GB
        m_config->setNetworkLatencyThreshold(100.0); // 网络延迟100ms

        // 配置启用的监控器
        QStringList enabledMonitors;
        enabledMonitors << "CPUMonitor" << "MemoryMonitor" << "NetworkMonitor";
        m_config->setEnabledMonitors(enabledMonitors);

        // 配置启用的优化器
        QStringList enabledOptimizers;
        enabledOptimizers << "StartupOptimizer" << "MemoryOptimizer";
        m_config->setEnabledOptimizers(enabledOptimizers);

        qDebug() << "Performance monitoring configured";
    }

    void startPerformanceMonitoring()
    {
        qDebug() << "\n3. Starting Performance Monitoring...";

        if (!m_performanceManager) {
            qDebug() << "Performance Manager not available";
            return;
        }

        // 连接性能管理器信号
        connect(m_performanceManager, &PerformanceManager::metricsUpdated,
                this, &PerformanceExample::onMetricsUpdated);
        connect(m_performanceManager, &PerformanceManager::performanceLevelChanged,
                this, &PerformanceExample::onPerformanceLevelChanged);
        connect(m_performanceManager, &PerformanceManager::optimizationCompleted,
                this, &PerformanceExample::onOptimizationCompleted);
        connect(m_performanceManager, &PerformanceManager::thresholdExceeded,
                this, &PerformanceExample::onThresholdExceeded);

        // 启动性能模块
        if (m_performanceModule->start()) {
            qDebug() << "Performance monitoring started successfully";
        } else {
            qDebug() << "Failed to start performance monitoring";
        }
    }

    void demonstrateBasicMonitoring()
    {
        qDebug() << "\nDemonstrating Basic Monitoring...";

        if (!m_performanceManager) {
            return;
        }

        // 获取当前性能指标
        PerformanceMetrics currentMetrics = m_performanceManager->getCurrentMetrics();
        qDebug() << "Current Performance Metrics:";
        qDebug() << "  Timestamp:" << currentMetrics.timestamp.toString();
        onMetricsUpdated(currentMetrics);

        // 获取性能等级和评分
        auto level = m_performanceManager->getCurrentPerformanceLevel();
        int score = m_performanceManager->getPerformanceScore();
        qDebug() << "Performance Score:" << score;

        // 获取系统信息
        QVariantMap systemInfo = m_performanceManager->getSystemInfo();
        qDebug() << "System Information:";
        for (auto it = systemInfo.begin(); it != systemInfo.end(); ++it) {
            qDebug() << "  " << it.key() << ":" << it.value().toString();
        }
    }

    void demonstrateOptimization()
    {
        qDebug() << "\nDemonstrating Optimization...";

        if (!m_performanceManager) {
            return;
        }

        // 检查是否需要优化
        bool needsOptimization = true; // 简化示例
        qDebug() << "Needs Optimization:" << needsOptimization;

        if (needsOptimization) {
            // 执行手动优化
            qDebug() << "Performing manual optimization...";
            bool optimizationResult = m_performanceManager->performOptimization();
            qDebug() << "Manual optimization result:" << optimizationResult;
        }

        // 生成性能报告
        QVariantMap report = m_performanceManager->generatePerformanceReport();
        qDebug() << "Performance Report Generated:";
        qDebug() << "  Report size:" << report.size() << "entries";
    }

    void demonstrateDataExport()
    {
        qDebug() << "\nDemonstrating Data Export...";

        if (!m_performanceManager) {
            return;
        }

        // 导出性能数据
        QString exportPath = "performance_data.json";
        bool exportResult = m_performanceManager->exportPerformanceData(exportPath, "json");
        qDebug() << "Data export result:" << exportResult;
        qDebug() << "Export path:" << exportPath;

        // 获取历史数据
        QDateTime from = QDateTime::currentDateTime().addSecs(-300); // 最近5分钟
        QDateTime to = QDateTime::currentDateTime();
        auto historicalMetrics = m_performanceManager->getHistoricalMetrics(from, to);
        qDebug() << "Historical metrics count:" << historicalMetrics.size();
    }

    void demonstrateConfigurationManagement()
    {
        qDebug() << "\nDemonstrating Configuration Management...";

        if (!m_config) {
            return;
        }

        // 保存当前配置
        QString configPath = "performance_config.json";
        bool saveResult = m_config->saveConfig(configPath);
        qDebug() << "Configuration save result:" << saveResult;

        // 导出配置为JSON
        QString configJson = m_config->exportToJson();
        qDebug() << "Configuration JSON size:" << configJson.size() << "characters";

        // 验证配置
        bool isValid = m_config->validateConfig();
        qDebug() << "Configuration is valid:" << isValid;

        // 获取所有配置
        QVariantMap allConfig = m_config->getAllConfig();
        qDebug() << "Total configuration entries:" << allConfig.size();
    }

    void demonstrateAdvancedFeatures()
    {
        qDebug() << "\nDemonstrating Advanced Features...";

        if (!m_performanceManager) {
            return;
        }

        // 获取所有监控器
        auto monitors = m_performanceManager->getAllMonitors();
        qDebug() << "Active monitors:" << monitors.size();

        // 获取所有优化器
        auto optimizers = m_performanceManager->getAllOptimizers();
        qDebug() << "Active optimizers:" << optimizers.size();

        // 清除历史数据
        QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-1);
        m_performanceManager->clearHistoricalData(cutoffTime);
        qDebug() << "Historical data cleanup completed";

        // 模拟系统负载
        simulateSystemLoad();
    }

    void simulateSystemLoad()
    {
        qDebug() << "Simulating system load...";

        // 创建一些CPU负载
        QThread::msleep(100);

        // 分配一些内存
        QByteArray largeData(1024 * 1024, 'X'); // 1MB数据

        qDebug() << "System load simulation completed";
    }

private:
    PerformanceModule* m_performanceModule;
    PerformanceManager* m_performanceManager;
    PerformanceConfig* m_config;
    QTimer* m_demoTimer;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Performance Module Example");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi Meet Qt");

    qDebug() << "Starting Performance Module Example...";

    // 创建并启动示例
    PerformanceExample example;
    example.start();

    return app.exec();
}

#include "PerformanceExample.moc"