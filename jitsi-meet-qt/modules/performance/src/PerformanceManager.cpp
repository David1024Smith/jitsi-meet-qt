#include "../include/PerformanceManager.h"
#include "../config/PerformanceConfig.h"
#include <QDebug>

PerformanceManager::PerformanceManager(QObject* parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_metricsCollector(nullptr)
    , m_monitoringTimer(new QTimer(this))
    , m_optimizationTimer(new QTimer(this))
    , m_isRunning(false)
    , m_autoOptimizationEnabled(false)
    , m_optimizationStrategy(OptimizationStrategy::Balanced)
    , m_currentLevel(PerformanceLevel::Fair)
{
    m_monitoringTimer->setSingleShot(false);
    m_optimizationTimer->setSingleShot(false);
    
    connect(m_monitoringTimer, &QTimer::timeout, this, &PerformanceManager::updateMetrics);
    connect(m_optimizationTimer, &QTimer::timeout, this, &PerformanceManager::performAutoOptimization);
}

PerformanceManager::~PerformanceManager()
{
    stop();
}

void PerformanceManager::setConfig(PerformanceConfig* config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

void PerformanceManager::setConfig(QObject* config)
{
    QMutexLocker locker(&m_mutex);
    m_config = qobject_cast<PerformanceConfig*>(config);
}

PerformanceConfig* PerformanceManager::config() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

void PerformanceManager::setMetricsCollector(MetricsCollector* collector)
{
    QMutexLocker locker(&m_mutex);
    m_metricsCollector = collector;
}

MetricsCollector* PerformanceManager::metricsCollector() const
{
    QMutexLocker locker(&m_mutex);
    return m_metricsCollector;
}

bool PerformanceManager::initialize()
{
    qDebug() << "PerformanceManager: Initializing...";
    return true;
}

bool PerformanceManager::start()
{
    QMutexLocker locker(&m_mutex);
    if (m_isRunning) {
        return true;
    }
    
    m_isRunning = true;
    m_monitoringTimer->start(1000); // 1 second interval
    
    qDebug() << "PerformanceManager: Started";
    return true;
}

void PerformanceManager::stop()
{
    QMutexLocker locker(&m_mutex);
    if (!m_isRunning) {
        return;
    }
    
    m_isRunning = false;
    m_monitoringTimer->stop();
    m_optimizationTimer->stop();
    
    qDebug() << "PerformanceManager: Stopped";
}

bool PerformanceManager::isRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning;
}

// setConfig 函数实现已移除，使用基类实现

QVariantMap PerformanceManager::getSystemInfo() const
{
    QVariantMap info;
    info["version"] = "1.0.0";
    info["status"] = m_isRunning ? "Running" : "Stopped";
    info["monitoring"] = m_isRunning;
    return info;
}

bool PerformanceManager::startMonitoring()
{
    m_monitoringTimer->start(1000);
    qDebug() << "PerformanceManager: Monitoring started";
    return true;
}

void PerformanceManager::stopMonitoring()
{
    QMutexLocker locker(&m_mutex);
    m_monitoringTimer->stop();
    qDebug() << "PerformanceManager: Monitoring stopped";
}

// getCpuUsage 和 getMemoryUsage 函数已在基类中实现

PerformanceMetrics PerformanceManager::getCurrentMetrics() const
{
    PerformanceMetrics metrics;
    metrics.timestamp = QDateTime::currentDateTime();
    
    // 填充系统指标
    metrics.system.cpuUsage = 25.0;  // 模拟值
    metrics.system.memoryUsage = 512.0;  // 模拟值
    
    // 设置其他字段
    metrics.network.latency = 50.0;
    metrics.video.frameRate = 60.0;
    
    return metrics;
}

// 这些函数的实现已移至头文件或基类

void PerformanceManager::setAutoOptimizationEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_autoOptimizationEnabled = enabled;
    
    if (enabled) {
        m_optimizationTimer->start(30000); // 30 seconds
    } else {
        m_optimizationTimer->stop();
    }
}

bool PerformanceManager::isAutoOptimizationEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoOptimizationEnabled;
}

bool PerformanceManager::isMonitoringActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning;
}

PerformanceManager::PerformanceLevel PerformanceManager::getCurrentPerformanceLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentLevel;
}

bool PerformanceManager::performOptimization()
{
    qDebug() << "PerformanceManager: Performing optimization...";
    
    // 简化实现，直接返回成功
    return true;
}

void PerformanceManager::updateMetrics()
{
    if (!m_isRunning) {
        return;
    }
    
    PerformanceMetrics metrics = getCurrentMetrics();
    
    // 检查性能等级变化
    PerformanceLevel newLevel = calculatePerformanceLevel(metrics);
    if (newLevel != m_currentLevel) {
        m_currentLevel = newLevel;
        emit performanceLevelChanged(newLevel);
    }
    
    emit metricsUpdated(metrics);
}

void PerformanceManager::performAutoOptimization()
{
    if (!m_autoOptimizationEnabled || !m_isRunning) {
        return;
    }
    
    qDebug() << "PerformanceManager: Performing auto optimization...";
    performOptimization();
}

PerformanceManager::PerformanceLevel PerformanceManager::calculatePerformanceLevel(const PerformanceMetrics& metrics) const
{
    // 简化的性能等级计算逻辑
    double cpuUsage = metrics.system.cpuUsage;
    double memoryUsage = metrics.system.memoryUsage;
    
    if (cpuUsage < 30.0 && memoryUsage < 1024.0) {
        return PerformanceLevel::Excellent;
    } else if (cpuUsage < 50.0 && memoryUsage < 2048.0) {
        return PerformanceLevel::Good;
    } else if (cpuUsage < 70.0 && memoryUsage < 4096.0) {
        return PerformanceLevel::Fair;
    } else if (cpuUsage < 85.0 && memoryUsage < 6144.0) {
        return PerformanceLevel::Poor;
    } else {
        return PerformanceLevel::Critical;
    }
}

void PerformanceManager::reset()
{
    QMutexLocker locker(&m_mutex);
    
    // 停止监控
    if (m_isRunning) {
        stop();
    }
    
    // 重置状态
    m_currentLevel = PerformanceLevel::Fair;
    m_autoOptimizationEnabled = false;
    
    // 清理资源
    if (m_metricsCollector) {
        // 重置指标收集器
    }
}

QString PerformanceManager::getMonitorName() const
{
    return "PerformanceManager";
}

QString PerformanceManager::getVersion() const
{
    return "1.0.0";
}

QVariantMap PerformanceManager::getStatus() const
{
    QMutexLocker locker(&m_mutex);
    QVariantMap status;
    status["isRunning"] = m_isRunning;
    status["autoOptimizationEnabled"] = m_autoOptimizationEnabled;
    status["currentLevel"] = static_cast<int>(m_currentLevel);
    status["hasConfig"] = (m_config != nullptr);
    status["hasMetricsCollector"] = (m_metricsCollector != nullptr);
    return status;
}

QList<PerformanceMetrics> PerformanceManager::getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    
    // 暂时返回空列表，后续可以实现历史数据存储和检索
    return QList<PerformanceMetrics>();
}

void PerformanceManager::checkThresholds()
{
    // 检查性能阈值的实现
    // 暂时为空实现，后续可以添加阈值检查逻辑
}

void PerformanceManager::handleMonitorError(const QString& error)
{
    qWarning() << "Monitor error:" << error;
    emit errorOccurred(error);
}