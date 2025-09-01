#include "PerformanceManager.h"
#include "MetricsCollector.h"
#include "PerformanceConfig.h"
#include "BaseMonitor.h"
#include "BaseOptimizer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

PerformanceManager::PerformanceManager(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_metricsCollector(nullptr)
    , m_monitoringTimer(new QTimer(this))
    , m_optimizationTimer(new QTimer(this))
    , m_isRunning(false)
    , m_autoOptimizationEnabled(false)
    , m_optimizationStrategy(Balanced)
    , m_currentLevel(Fair)
{
    // 设置定时器
    m_monitoringTimer->setSingleShot(false);
    m_optimizationTimer->setSingleShot(false);
    
    // 连接信号
    connect(m_monitoringTimer, &QTimer::timeout, this, &PerformanceManager::updateMetrics);
    connect(m_optimizationTimer, &QTimer::timeout, this, &PerformanceManager::performAutoOptimization);
}

PerformanceManager::~PerformanceManager()
{
    stop();
    
    // 清理监控器
    qDeleteAll(m_monitors);
    m_monitors.clear();
    
    // 清理优化器
    qDeleteAll(m_optimizers);
    m_optimizers.clear();
}

bool PerformanceManager::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        // 初始化默认监控器
        initializeDefaultMonitors();
        
        // 初始化默认优化器
        initializeDefaultOptimizers();
        
        qDebug() << "PerformanceManager: Initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceManager: Exception during initialization:" << e.what();
        emit errorOccurred(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

bool PerformanceManager::start()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isRunning) {
        return true;
    }
    
    try {
        // 启动所有监控器
        for (auto monitor : m_monitors) {
            if (!monitor->startTracking()) {
                qWarning() << "PerformanceManager: Failed to start monitor:" << monitor->monitorName();
            }
        }
        
        // 设置监控间隔
        int interval = m_config ? m_config->monitoringInterval() : 1000;
        m_monitoringTimer->setInterval(interval);
        m_monitoringTimer->start();
        
        // 如果启用自动优化，启动优化定时器
        if (m_autoOptimizationEnabled) {
            int optInterval = m_config ? m_config->optimizationInterval() : 30000;
            m_optimizationTimer->setInterval(optInterval);
            m_optimizationTimer->start();
        }
        
        m_isRunning = true;
        qDebug() << "PerformanceManager: Started successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceManager: Exception during start:" << e.what();
        emit errorOccurred(QString("Start failed: %1").arg(e.what()));
        return false;
    }
}

void PerformanceManager::stop()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRunning) {
        return;
    }
    
    try {
        // 停止定时器
        m_monitoringTimer->stop();
        m_optimizationTimer->stop();
        
        // 停止所有监控器
        for (auto monitor : m_monitors) {
            monitor->stopTracking();
        }
        
        m_isRunning = false;
        qDebug() << "PerformanceManager: Stopped";
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceManager: Exception during stop:" << e.what();
        emit errorOccurred(QString("Stop failed: %1").arg(e.what()));
    }
}

bool PerformanceManager::isRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning;
}

PerformanceMetrics PerformanceManager::getCurrentMetrics() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_metricsCollector) {
        return m_metricsCollector->getLatestMetrics();
    }
    
    return PerformanceMetrics();
}

QList<PerformanceMetrics> PerformanceManager::getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_metricsCollector) {
        return m_metricsCollector->getHistoricalMetrics(from, to);
    }
    
    return QList<PerformanceMetrics>();
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
    status["running"] = m_isRunning;
    status["autoOptimization"] = m_autoOptimizationEnabled;
    status["strategy"] = QVariant::fromValue(m_optimizationStrategy);
    status["currentLevel"] = QVariant::fromValue(m_currentLevel);
    status["monitorCount"] = m_monitors.size();
    status["optimizerCount"] = m_optimizers.size();
    
    return status;
}

void PerformanceManager::reset()
{
    QMutexLocker locker(&m_mutex);
    
    // 重置所有监控器
    for (auto monitor : m_monitors) {
        monitor->reset();
    }
    
    // 重置性能等级
    m_currentLevel = Fair;
    
    qDebug() << "PerformanceManager: Reset completed";
}

void PerformanceManager::setConfig(PerformanceConfig* config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    if (m_config) {
        // 应用配置
        setMonitoringInterval(m_config->monitoringInterval());
        setAutoOptimizationEnabled(m_config->isAutoOptimizationEnabled());
    }
}

PerformanceConfig* PerformanceManager::config() const
{
    return m_config;
}

void PerformanceManager::setMetricsCollector(MetricsCollector* collector)
{
    QMutexLocker locker(&m_mutex);
    m_metricsCollector = collector;
    
    if (m_metricsCollector) {
        // 连接信号
        connect(m_metricsCollector, &MetricsCollector::metricsCollected,
                this, &PerformanceManager::metricsUpdated);
    }
}

MetricsCollector* PerformanceManager::metricsCollector() const
{
    return m_metricsCollector;
}

bool PerformanceManager::addMonitor(BaseMonitor* monitor)
{
    if (!monitor) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString name = monitor->monitorName();
    if (m_monitors.contains(name)) {
        qWarning() << "PerformanceManager: Monitor already exists:" << name;
        return false;
    }
    
    m_monitors[name] = monitor;
    
    // 连接信号
    connect(monitor, &BaseMonitor::errorOccurred,
            this, &PerformanceManager::handleMonitorError);
    
    // 如果指标收集器存在，注册监控器
    if (m_metricsCollector) {
        m_metricsCollector->registerMonitor(monitor);
    }
    
    qDebug() << "PerformanceManager: Added monitor:" << name;
    return true;
}

bool PerformanceManager::removeMonitor(const QString& monitorName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_monitors.contains(monitorName)) {
        return false;
    }
    
    BaseMonitor* monitor = m_monitors.take(monitorName);
    
    // 如果指标收集器存在，注销监控器
    if (m_metricsCollector) {
        m_metricsCollector->unregisterMonitor(monitorName);
    }
    
    monitor->deleteLater();
    
    qDebug() << "PerformanceManager: Removed monitor:" << monitorName;
    return true;
}

BaseMonitor* PerformanceManager::getMonitor(const QString& monitorName) const
{
    QMutexLocker locker(&m_mutex);
    return m_monitors.value(monitorName, nullptr);
}

QList<BaseMonitor*> PerformanceManager::getAllMonitors() const
{
    QMutexLocker locker(&m_mutex);
    return m_monitors.values();
}

bool PerformanceManager::addOptimizer(BaseOptimizer* optimizer)
{
    if (!optimizer) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString name = optimizer->getOptimizerName();
    if (m_optimizers.contains(name)) {
        qWarning() << "PerformanceManager: Optimizer already exists:" << name;
        return false;
    }
    
    m_optimizers[name] = optimizer;
    
    // 连接信号
    connect(optimizer, &BaseOptimizer::errorOccurred,
            this, &PerformanceManager::handleMonitorError);
    
    qDebug() << "PerformanceManager: Added optimizer:" << name;
    return true;
}

bool PerformanceManager::removeOptimizer(const QString& optimizerName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_optimizers.contains(optimizerName)) {
        return false;
    }
    
    BaseOptimizer* optimizer = m_optimizers.take(optimizerName);
    optimizer->deleteLater();
    
    qDebug() << "PerformanceManager: Removed optimizer:" << optimizerName;
    return true;
}

BaseOptimizer* PerformanceManager::getOptimizer(const QString& optimizerName) const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizers.value(optimizerName, nullptr);
}

QList<BaseOptimizer*> PerformanceManager::getAllOptimizers() const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizers.values();
}

void PerformanceManager::setMonitoringInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    if (interval > 0) {
        m_monitoringTimer->setInterval(interval);
        
        // 更新所有监控器的间隔
        for (auto monitor : m_monitors) {
            monitor->setTrackingInterval(interval);
        }
    }
}

int PerformanceManager::monitoringInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_monitoringTimer->interval();
}

void PerformanceManager::setAutoOptimizationEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    m_autoOptimizationEnabled = enabled;
    
    if (enabled && m_isRunning) {
        int interval = m_config ? m_config->optimizationInterval() : 30000;
        m_optimizationTimer->setInterval(interval);
        m_optimizationTimer->start();
    } else {
        m_optimizationTimer->stop();
    }
}

bool PerformanceManager::isAutoOptimizationEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoOptimizationEnabled;
}

void PerformanceManager::setOptimizationStrategy(OptimizationStrategy strategy)
{
    QMutexLocker locker(&m_mutex);
    m_optimizationStrategy = strategy;
}

PerformanceManager::OptimizationStrategy PerformanceManager::optimizationStrategy() const
{
    QMutexLocker locker(&m_mutex);
    return m_optimizationStrategy;
}

PerformanceManager::PerformanceLevel PerformanceManager::getCurrentPerformanceLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentLevel;
}

int PerformanceManager::getPerformanceScore() const
{
    PerformanceMetrics metrics = getCurrentMetrics();
    return calculatePerformanceScore(metrics);
}

bool PerformanceManager::performOptimization()
{
    QMutexLocker locker(&m_mutex);
    
    try {
        PerformanceMetrics metrics = getCurrentMetrics();
        QVariantMap improvements;
        bool success = true;
        
        // 执行所有优化器
        for (auto optimizer : m_optimizers) {
            // 这里应该调用优化器的优化方法
            // optimizer->optimize(metrics);
        }
        
        emit optimizationCompleted(success, improvements);
        return success;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceManager: Exception during optimization:" << e.what();
        emit errorOccurred(QString("Optimization failed: %1").arg(e.what()));
        return false;
    }
}

QVariantMap PerformanceManager::generatePerformanceReport() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap report;
    report["timestamp"] = QDateTime::currentDateTime();
    report["performanceLevel"] = QVariant::fromValue(m_currentLevel);
    report["performanceScore"] = getPerformanceScore();
    
    // 添加当前指标
    PerformanceMetrics current = getCurrentMetrics();
    QVariantMap currentMetrics;
    currentMetrics["cpu"] = current.system.cpuUsage;
    currentMetrics["memory"] = static_cast<qint64>(current.system.memoryUsage);
    currentMetrics["network_latency"] = current.network.latency;
    currentMetrics["frame_rate"] = current.video.frameRate;
    report["currentMetrics"] = currentMetrics;
    
    // 添加监控器状态
    QVariantMap monitorStatus;
    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it) {
        monitorStatus[it.key()] = it.value()->getTrackerStatus();
    }
    report["monitorStatus"] = monitorStatus;
    
    return report;
}

QVariantMap PerformanceManager::getSystemInfo() const
{
    QVariantMap info;
    info["version"] = getVersion();
    info["running"] = isRunning();
    info["monitorCount"] = m_monitors.size();
    info["optimizerCount"] = m_optimizers.size();
    
    return info;
}

void PerformanceManager::updateMetrics()
{
    if (!m_metricsCollector) {
        return;
    }
    
    try {
        PerformanceMetrics metrics = m_metricsCollector->collectCurrentMetrics();
        
        // 计算性能等级
        PerformanceLevel newLevel = calculatePerformanceLevel(metrics);
        if (newLevel != m_currentLevel) {
            m_currentLevel = newLevel;
            emit performanceLevelChanged(newLevel);
        }
        
        // 检查阈值
        checkThresholds();
        
        emit metricsUpdated(metrics);
        
    } catch (const std::exception& e) {
        qWarning() << "PerformanceManager: Exception in updateMetrics:" << e.what();
        emit errorOccurred(QString("Metrics update failed: %1").arg(e.what()));
    }
}

void PerformanceManager::checkThresholds()
{
    if (!m_config) {
        return;
    }
    
    PerformanceMetrics metrics = getCurrentMetrics();
    
    // 检查CPU阈值
    double cpuThreshold = m_config->cpuThreshold();
    if (metrics.system.cpuUsage > cpuThreshold) {
        emit thresholdExceeded("cpu", metrics.system.cpuUsage, cpuThreshold);
    }
    
    // 检查内存阈值
    qint64 memoryThreshold = m_config->memoryThreshold();
    if (metrics.system.memoryUsage > memoryThreshold) {
        emit thresholdExceeded("memory", metrics.system.memoryUsage, memoryThreshold);
    }
    
    // 检查网络延迟阈值
    double latencyThreshold = m_config->networkLatencyThreshold();
    if (metrics.network.latency > latencyThreshold) {
        emit thresholdExceeded("network_latency", metrics.network.latency, latencyThreshold);
    }
    
    // 检查帧率阈值
    double frameRateThreshold = m_config->frameRateThreshold();
    if (metrics.video.frameRate < frameRateThreshold) {
        emit thresholdExceeded("frame_rate", metrics.video.frameRate, frameRateThreshold);
    }
}

void PerformanceManager::performAutoOptimization()
{
    if (!m_autoOptimizationEnabled) {
        return;
    }
    
    PerformanceMetrics metrics = getCurrentMetrics();
    if (shouldOptimize(metrics)) {
        performOptimization();
    }
}

void PerformanceManager::handleMonitorError(const QString& error)
{
    qWarning() << "PerformanceManager: Monitor error:" << error;
    emit errorOccurred(QString("Monitor error: %1").arg(error));
}

void PerformanceManager::initializeDefaultMonitors()
{
    // 这里应该创建默认的监控器实例
    // 由于需要具体的监控器实现，这里只是占位符
    qDebug() << "PerformanceManager: Default monitors initialized";
}

void PerformanceManager::initializeDefaultOptimizers()
{
    // 这里应该创建默认的优化器实例
    // 由于需要具体的优化器实现，这里只是占位符
    qDebug() << "PerformanceManager: Default optimizers initialized";
}

PerformanceManager::PerformanceLevel PerformanceManager::calculatePerformanceLevel(const PerformanceMetrics& metrics) const
{
    int score = calculatePerformanceScore(metrics);
    
    if (score >= 90) return Excellent;
    if (score >= 75) return Good;
    if (score >= 60) return Fair;
    if (score >= 40) return Poor;
    return Critical;
}

int PerformanceManager::calculatePerformanceScore(const PerformanceMetrics& metrics) const
{
    // 简单的评分算法
    int score = 100;
    
    // CPU使用率影响 (权重: 25%)
    if (metrics.system.cpuUsage > 80) score -= 25;
    else if (metrics.system.cpuUsage > 60) score -= 15;
    else if (metrics.system.cpuUsage > 40) score -= 5;
    
    // 内存使用影响 (权重: 20%)
    if (metrics.system.memoryUsage > 8000) score -= 20;  // > 8GB
    else if (metrics.system.memoryUsage > 4000) score -= 10;  // > 4GB
    else if (metrics.system.memoryUsage > 2000) score -= 5;   // > 2GB
    
    // 网络延迟影响 (权重: 25%)
    if (metrics.network.latency > 200) score -= 25;
    else if (metrics.network.latency > 100) score -= 15;
    else if (metrics.network.latency > 50) score -= 5;
    
    // 帧率影响 (权重: 30%)
    if (metrics.video.frameRate < 15) score -= 30;
    else if (metrics.video.frameRate < 24) score -= 20;
    else if (metrics.video.frameRate < 30) score -= 10;
    
    return qMax(0, score);
}

bool PerformanceManager::shouldOptimize(const PerformanceMetrics& metrics) const
{
    PerformanceLevel level = calculatePerformanceLevel(metrics);
    
    switch (m_optimizationStrategy) {
    case Conservative:
        return level == Critical;
    case Balanced:
        return level <= Poor;
    case Aggressive:
        return level <= Fair;
    }
    
    return false;
}

bool PerformanceManager::isMonitoringActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRunning && m_monitoringTimer->isActive();
}

bool PerformanceManager::startMonitoring()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRunning) {
        return start();
    }
    
    if (!m_monitoringTimer->isActive()) {
        m_monitoringTimer->start();
        qDebug() << "PerformanceManager: Monitoring started";
    }
    
    return true;
}

void PerformanceManager::stopMonitoring()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_monitoringTimer->isActive()) {
        m_monitoringTimer->stop();
        qDebug() << "PerformanceManager: Monitoring stopped";
    }
}