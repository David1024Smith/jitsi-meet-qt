/**
 * @file performance_optimization_complete.cpp
 * @brief 完整的性能优化演示程序
 * 
 * 这个示例展示了如何使用所有性能优化组件来创建一个高性能的Jitsi Meet Qt应用程序。
 * 包括启动优化、内存管理、泄漏检测、性能监控和配置管理。
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QTimer>
#include <QGroupBox>
#include <QGridLayout>
#include <QDebug>

#include "PerformanceManager.h"
#include "StartupOptimizer.h"
#include "MemoryLeakDetector.h"
#include "MemoryProfiler.h"
#include "OptimizedRecentManager.h"
#include "PerformanceConfig.h"

class PerformanceOptimizationDemo : public QMainWindow
{
    Q_OBJECT

public:
    PerformanceOptimizationDemo(QWidget *parent = nullptr);
    ~PerformanceOptimizationDemo();

private slots:
    void onStartupPhaseCompleted(const QString& phase, qint64 duration);
    void onMemoryWarning(qint64 memoryUsage);
    void onMemoryLeakDetected(const QList<MemoryLeakDetector::AllocationInfo>& leaks);
    void onMemoryTrendChanged(const MemoryProfiler::MemoryTrend& trend);
    void onOptimizationSuggestion(const MemoryProfiler::OptimizationSuggestion& suggestion);
    void onPerformanceMetricsUpdated(const PerformanceManager::PerformanceMetrics& metrics);
    
    void startPerformanceTest();
    void stopPerformanceTest();
    void simulateMemoryLoad();
    void clearMemoryLoad();
    void exportPerformanceReport();
    void resetConfiguration();

private:
    void setupUI();
    void setupPerformanceComponents();
    void updateMetricsDisplay();
    void updateMemoryDisplay();
    void logMessage(const QString& message);

    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    
    // 指标显示
    QGroupBox* m_metricsGroup;
    QLabel* m_startupTimeLabel;
    QLabel* m_memoryUsageLabel;
    QLabel* m_networkMemoryLabel;
    QLabel* m_recentItemsLabel;
    QProgressBar* m_memoryProgressBar;
    
    // 内存分析显示
    QGroupBox* m_memoryGroup;
    QLabel* m_averageMemoryLabel;
    QLabel* m_peakMemoryLabel;
    QLabel* m_growthRateLabel;
    QLabel* m_fragmentationLabel;
    
    // 控制按钮
    QGroupBox* m_controlGroup;
    QPushButton* m_startTestButton;
    QPushButton* m_stopTestButton;
    QPushButton* m_simulateLoadButton;
    QPushButton* m_clearLoadButton;
    QPushButton* m_exportReportButton;
    QPushButton* m_resetConfigButton;
    
    // 日志显示
    QTextEdit* m_logTextEdit;
    
    // 性能组件
    PerformanceManager* m_performanceManager;
    StartupOptimizer* m_startupOptimizer;
    MemoryLeakDetector* m_memoryLeakDetector;
    MemoryProfiler* m_memoryProfiler;
    OptimizedRecentManager* m_recentManager;
    PerformanceConfig* m_performanceConfig;
    
    // 测试数据
    QList<void*> m_testAllocations;
    QTimer* m_updateTimer;
    bool m_testRunning;
};

PerformanceOptimizationDemo::PerformanceOptimizationDemo(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_performanceManager(nullptr)
    , m_startupOptimizer(nullptr)
    , m_memoryLeakDetector(nullptr)
    , m_memoryProfiler(nullptr)
    , m_recentManager(nullptr)
    , m_performanceConfig(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_testRunning(false)
{
    setWindowTitle("Jitsi Meet Qt - Performance Optimization Demo");
    setMinimumSize(800, 600);
    
    setupPerformanceComponents();
    setupUI();
    
    // 设置更新定时器
    m_updateTimer->setInterval(1000); // 每秒更新一次
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceOptimizationDemo::updateMetricsDisplay);
    m_updateTimer->start();
    
    logMessage("Performance Optimization Demo initialized");
}

PerformanceOptimizationDemo::~PerformanceOptimizationDemo()
{
    if (m_testRunning) {
        stopPerformanceTest();
    }
    
    // 清理测试分配
    clearMemoryLoad();
}

void PerformanceOptimizationDemo::setupPerformanceComponents()
{
    // 创建性能配置管理器
    m_performanceConfig = new PerformanceConfig(this);
    
    // 创建启动优化器
    m_startupOptimizer = new StartupOptimizer(this);
    m_startupOptimizer->setOptimizationLevel(StartupOptimizer::Moderate);
    connect(m_startupOptimizer, &StartupOptimizer::startupPhaseCompleted,
            this, &PerformanceOptimizationDemo::onStartupPhaseCompleted);
    
    // 创建性能管理器
    m_performanceManager = new PerformanceManager(this);
    connect(m_performanceManager, &PerformanceManager::memoryWarning,
            this, &PerformanceOptimizationDemo::onMemoryWarning);
    connect(m_performanceManager, &PerformanceManager::performanceMetricsUpdated,
            this, &PerformanceOptimizationDemo::onPerformanceMetricsUpdated);
    
    // 创建内存泄漏检测器
    m_memoryLeakDetector = new MemoryLeakDetector(this);
    connect(m_memoryLeakDetector, &MemoryLeakDetector::memoryLeakDetected,
            this, &PerformanceOptimizationDemo::onMemoryLeakDetected);
    
    // 创建内存分析器
    m_memoryProfiler = new MemoryProfiler(this);
    connect(m_memoryProfiler, &MemoryProfiler::memoryTrendChanged,
            this, &PerformanceOptimizationDemo::onMemoryTrendChanged);
    connect(m_memoryProfiler, &MemoryProfiler::optimizationSuggestionAvailable,
            this, &PerformanceOptimizationDemo::onOptimizationSuggestion);
    
    // 创建优化的最近项目管理器
    m_recentManager = new OptimizedRecentManager(this);
    
    // 启用快速启动
    m_startupOptimizer->enableFastStartup();
    
    // 开始性能监控
    m_performanceManager->startMemoryMonitoring();
    m_memoryLeakDetector->startLeakDetection();
}

void PerformanceOptimizationDemo::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建指标显示组
    m_metricsGroup = new QGroupBox("Performance Metrics");
    QGridLayout* metricsLayout = new QGridLayout(m_metricsGroup);
    
    m_startupTimeLabel = new QLabel("Startup Time: 0 ms");
    m_memoryUsageLabel = new QLabel("Memory Usage: 0 MB");
    m_networkMemoryLabel = new QLabel("Network Memory: 0 MB");
    m_recentItemsLabel = new QLabel("Recent Items: 0");
    
    m_memoryProgressBar = new QProgressBar;
    m_memoryProgressBar->setRange(0, 100);
    m_memoryProgressBar->setValue(0);
    
    metricsLayout->addWidget(m_startupTimeLabel, 0, 0);
    metricsLayout->addWidget(m_memoryUsageLabel, 0, 1);
    metricsLayout->addWidget(m_networkMemoryLabel, 1, 0);
    metricsLayout->addWidget(m_recentItemsLabel, 1, 1);
    metricsLayout->addWidget(new QLabel("Memory Usage:"), 2, 0);
    metricsLayout->addWidget(m_memoryProgressBar, 2, 1);
    
    // 创建内存分析显示组
    m_memoryGroup = new QGroupBox("Memory Analysis");
    QGridLayout* memoryLayout = new QGridLayout(m_memoryGroup);
    
    m_averageMemoryLabel = new QLabel("Average: 0 MB");
    m_peakMemoryLabel = new QLabel("Peak: 0 MB");
    m_growthRateLabel = new QLabel("Growth Rate: 0%");
    m_fragmentationLabel = new QLabel("Fragmentation: 0%");
    
    memoryLayout->addWidget(m_averageMemoryLabel, 0, 0);
    memoryLayout->addWidget(m_peakMemoryLabel, 0, 1);
    memoryLayout->addWidget(m_growthRateLabel, 1, 0);
    memoryLayout->addWidget(m_fragmentationLabel, 1, 1);
    
    // 创建控制按钮组
    m_controlGroup = new QGroupBox("Controls");
    QGridLayout* controlLayout = new QGridLayout(m_controlGroup);
    
    m_startTestButton = new QPushButton("Start Performance Test");
    m_stopTestButton = new QPushButton("Stop Performance Test");
    m_simulateLoadButton = new QPushButton("Simulate Memory Load");
    m_clearLoadButton = new QPushButton("Clear Memory Load");
    m_exportReportButton = new QPushButton("Export Report");
    m_resetConfigButton = new QPushButton("Reset Configuration");
    
    m_stopTestButton->setEnabled(false);
    m_clearLoadButton->setEnabled(false);
    
    connect(m_startTestButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::startPerformanceTest);
    connect(m_stopTestButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::stopPerformanceTest);
    connect(m_simulateLoadButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::simulateMemoryLoad);
    connect(m_clearLoadButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::clearMemoryLoad);
    connect(m_exportReportButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::exportPerformanceReport);
    connect(m_resetConfigButton, &QPushButton::clicked, this, &PerformanceOptimizationDemo::resetConfiguration);
    
    controlLayout->addWidget(m_startTestButton, 0, 0);
    controlLayout->addWidget(m_stopTestButton, 0, 1);
    controlLayout->addWidget(m_simulateLoadButton, 1, 0);
    controlLayout->addWidget(m_clearLoadButton, 1, 1);
    controlLayout->addWidget(m_exportReportButton, 2, 0);
    controlLayout->addWidget(m_resetConfigButton, 2, 1);
    
    // 创建日志显示
    m_logTextEdit = new QTextEdit;
    m_logTextEdit->setMaximumHeight(200);
    m_logTextEdit->setReadOnly(true);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_metricsGroup);
    m_mainLayout->addWidget(m_memoryGroup);
    m_mainLayout->addWidget(m_controlGroup);
    m_mainLayout->addWidget(new QLabel("Performance Log:"));
    m_mainLayout->addWidget(m_logTextEdit);
}

void PerformanceOptimizationDemo::onStartupPhaseCompleted(const QString& phase, qint64 duration)
{
    logMessage(QString("Startup phase '%1' completed in %2 ms").arg(phase).arg(duration));
}

void PerformanceOptimizationDemo::onMemoryWarning(qint64 memoryUsage)
{
    logMessage(QString("Memory warning: %1 MB").arg(memoryUsage / (1024*1024)));
}

void PerformanceOptimizationDemo::onMemoryLeakDetected(const QList<MemoryLeakDetector::AllocationInfo>& leaks)
{
    logMessage(QString("Memory leaks detected: %1 potential leaks").arg(leaks.size()));
}

void PerformanceOptimizationDemo::onMemoryTrendChanged(const MemoryProfiler::MemoryTrend& trend)
{
    updateMemoryDisplay();
    logMessage(QString("Memory trend updated - Growth rate: %1%").arg(trend.growthRate * 100, 0, 'f', 2));
}

void PerformanceOptimizationDemo::onOptimizationSuggestion(const MemoryProfiler::OptimizationSuggestion& suggestion)
{
    logMessage(QString("Optimization suggestion [%1]: %2").arg(suggestion.category).arg(suggestion.description));
}

void PerformanceOptimizationDemo::onPerformanceMetricsUpdated(const PerformanceManager::PerformanceMetrics& metrics)
{
    // 指标会在updateMetricsDisplay中更新
}

void PerformanceOptimizationDemo::startPerformanceTest()
{
    if (m_testRunning) {
        return;
    }
    
    m_testRunning = true;
    m_startTestButton->setEnabled(false);
    m_stopTestButton->setEnabled(true);
    
    // 开始内存分析
    m_memoryProfiler->startProfiling();
    
    // 添加一些测试数据到最近项目管理器
    for (int i = 0; i < 20; ++i) {
        QString url = QString("https://meet.jit.si/test-room-%1").arg(i);
        QString name = QString("Test Room %1").arg(i);
        m_recentManager->addRecentItem(url, name);
    }
    
    logMessage("Performance test started");
}

void PerformanceOptimizationDemo::stopPerformanceTest()
{
    if (!m_testRunning) {
        return;
    }
    
    m_testRunning = false;
    m_startTestButton->setEnabled(true);
    m_stopTestButton->setEnabled(false);
    
    // 停止内存分析
    m_memoryProfiler->stopProfiling();
    
    // 清理测试数据
    m_recentManager->clearRecentItems();
    
    logMessage("Performance test stopped");
}

void PerformanceOptimizationDemo::simulateMemoryLoad()
{
    // 分配一些内存来模拟负载
    for (int i = 0; i < 100; ++i) {
        void* ptr = malloc(1024 * 1024); // 1MB
        if (ptr) {
            m_testAllocations.append(ptr);
            m_memoryLeakDetector->trackAllocation(ptr, 1024 * 1024, __FILE__, __LINE__);
        }
    }
    
    m_simulateLoadButton->setEnabled(false);
    m_clearLoadButton->setEnabled(true);
    
    logMessage(QString("Simulated memory load: %1 MB allocated").arg(m_testAllocations.size()));
}

void PerformanceOptimizationDemo::clearMemoryLoad()
{
    // 释放分配的内存
    for (void* ptr : m_testAllocations) {
        m_memoryLeakDetector->trackDeallocation(ptr);
        free(ptr);
    }
    m_testAllocations.clear();
    
    m_simulateLoadButton->setEnabled(true);
    m_clearLoadButton->setEnabled(false);
    
    logMessage("Memory load cleared");
}

void PerformanceOptimizationDemo::exportPerformanceReport()
{
    QString reportPath = "performance_report.json";
    m_memoryProfiler->exportReport(reportPath);
    
    // 也生成性能管理器的指标日志
    m_performanceManager->logPerformanceMetrics();
    m_memoryLeakDetector->logMemoryStatistics();
    
    logMessage(QString("Performance report exported to %1").arg(reportPath));
}

void PerformanceOptimizationDemo::resetConfiguration()
{
    m_performanceConfig->resetToDefaults();
    logMessage("Configuration reset to defaults");
}

void PerformanceOptimizationDemo::updateMetricsDisplay()
{
    PerformanceManager::PerformanceMetrics metrics = m_performanceManager->getMetrics();
    
    m_startupTimeLabel->setText(QString("Startup Time: %1 ms").arg(metrics.startupTime));
    m_memoryUsageLabel->setText(QString("Memory Usage: %1 MB").arg(metrics.memoryUsage / (1024*1024)));
    m_networkMemoryLabel->setText(QString("Network Memory: %1 MB").arg(metrics.networkMemory / (1024*1024)));
    m_recentItemsLabel->setText(QString("Recent Items: %1").arg(m_recentManager->getItemCount()));
    
    // 更新内存进度条（假设1GB为100%）
    int memoryPercent = qMin(100, (int)(metrics.memoryUsage / (1024*1024*10))); // 10MB = 1%
    m_memoryProgressBar->setValue(memoryPercent);
    
    updateMemoryDisplay();
}

void PerformanceOptimizationDemo::updateMemoryDisplay()
{
    if (!m_memoryProfiler->isProfilingActive()) {
        return;
    }
    
    MemoryProfiler::MemoryTrend trend = m_memoryProfiler->analyzeTrend(1);
    MemoryProfiler::MemorySnapshot current = m_memoryProfiler->getCurrentSnapshot();
    
    m_averageMemoryLabel->setText(QString("Average: %1 MB").arg(trend.averageUsage / (1024*1024)));
    m_peakMemoryLabel->setText(QString("Peak: %1 MB").arg(trend.peakUsage / (1024*1024)));
    m_growthRateLabel->setText(QString("Growth Rate: %1%").arg(trend.growthRate * 100, 0, 'f', 2));
    m_fragmentationLabel->setText(QString("Fragmentation: %1%").arg(current.fragmentationRatio * 100, 0, 'f', 1));
}

void PerformanceOptimizationDemo::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);
    
    m_logTextEdit->append(logEntry);
    qDebug() << logEntry;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Jitsi Meet Qt Performance Demo");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Jitsi Meet Qt");
    
    PerformanceOptimizationDemo demo;
    demo.show();
    
    return app.exec();
}

#include "performance_optimization_complete.moc"