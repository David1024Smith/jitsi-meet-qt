#include "ModuleHealthMonitor.h"
#include "GlobalModuleConfig.h"
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QProcess>
#include <QFileInfo>

ModuleHealthMonitor::ModuleHealthMonitor(QObject* parent)
    : IHealthMonitor(parent)
    , m_monitoringTimer(new QTimer(this))
    , m_threadPool(new QThreadPool(this))
    , m_monitoringInterval(30000) // 30秒
    , m_maxHistorySize(100)
    , m_totalChecks(0)
    , m_failedChecks(0)
    , m_totalCheckDuration(0.0)
{
    // 设置监控定时器
    m_monitoringTimer->setSingleShot(false);
    connect(m_monitoringTimer, &QTimer::timeout, this, &ModuleHealthMonitor::performScheduledCheck);
    
    // 设置线程池
    m_threadPool->setMaxThreadCount(4); // 最多4个并发健康检查
    
    qDebug() << "ModuleHealthMonitor initialized";
}

ModuleHealthMonitor::~ModuleHealthMonitor()
{
    m_monitoringTimer->stop();
    m_threadPool->waitForDone(5000); // 等待5秒完成所有任务
}

IHealthMonitor::HealthReport ModuleHealthMonitor::checkModuleHealth(const QString& moduleName)
{
    return performHealthCheck(moduleName, Basic);
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performHealthCheck(const QString& moduleName, CheckType type)
{
    QElapsedTimer timer;
    timer.start();
    
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    try {
        switch (type) {
        case Basic:
            report = performBasicCheck(moduleName);
            break;
        case Performance:
            report = performPerformanceCheck(moduleName);
            break;
        case Resource:
            report = performResourceCheck(moduleName);
            break;
        case Connectivity:
            report = performConnectivityCheck(moduleName);
            break;
        case Functional:
            report = performFunctionalCheck(moduleName);
            break;
        }
        
        report.checkDuration = timer.elapsed();
        
        // 更新统计信息
        QMutexLocker locker(&m_mutex);
        m_totalChecks++;
        m_totalCheckDuration += report.checkDuration;
        
        if (report.status == Failure || report.status == Critical) {
            m_failedChecks++;
        }
        
        // 更新健康数据
        updateHealthData(moduleName, report);
        
        // 检查阈值
        checkThresholds(moduleName, report);
        
        emit healthCheckCompleted(moduleName, report);
        
    } catch (const std::exception& e) {
        report.status = Failure;
        report.message = QString("Health check failed: %1").arg(e.what());
        report.score = 0.0;
        report.checkDuration = timer.elapsed();
        
        QMutexLocker locker(&m_mutex);
        m_failedChecks++;
        
        qWarning() << "Health check exception for module" << moduleName << ":" << e.what();
    }
    
    return report;
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performBasicCheck(const QString& moduleName)
{
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    GlobalModuleConfig* config = GlobalModuleConfig::instance();
    
    // 检查模块是否存在
    if (!config->hasModule(moduleName)) {
        report.status = Failure;
        report.message = "Module not found";
        report.score = 0.0;
        return report;
    }
    
    // 检查模块是否启用
    if (!config->isModuleEnabled(moduleName)) {
        report.status = Warning;
        report.message = "Module is disabled";
        report.score = 50.0;
        return report;
    }
    
    // 检查模块响应性
    if (!isModuleResponsive(moduleName)) {
        report.status = Critical;
        report.message = "Module is not responsive";
        report.score = 25.0;
        return report;
    }
    
    // 基础检查通过
    report.status = Healthy;
    report.message = "Module is healthy";
    report.score = 100.0;
    
    // 添加详细信息
    report.details["enabled"] = config->isModuleEnabled(moduleName);
    report.details["version"] = config->getModuleInfo(moduleName).version;
    report.details["responsive"] = true;
    
    return report;
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performPerformanceCheck(const QString& moduleName)
{
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    // 测量性能指标
    double performanceScore = measureModulePerformance(moduleName);
    
    if (performanceScore >= 80.0) {
        report.status = Healthy;
        report.message = "Performance is excellent";
    } else if (performanceScore >= 60.0) {
        report.status = Warning;
        report.message = "Performance is acceptable";
    } else if (performanceScore >= 40.0) {
        report.status = Critical;
        report.message = "Performance is poor";
    } else {
        report.status = Failure;
        report.message = "Performance is unacceptable";
    }
    
    report.score = performanceScore;
    report.details["performanceScore"] = performanceScore;
    report.details["cpuUsage"] = QVariant(); // 实际实现中应该获取真实的CPU使用率
    report.details["memoryUsage"] = QVariant(); // 实际实现中应该获取真实的内存使用
    
    return report;
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performResourceCheck(const QString& moduleName)
{
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    // 检查资源使用情况
    QVariantMap metrics = collectModuleMetrics(moduleName);
    
    double resourceScore = 100.0;
    QString issues;
    
    // 检查内存使用
    if (metrics.contains("memoryUsage")) {
        double memUsage = metrics["memoryUsage"].toDouble();
        if (memUsage > 80.0) {
            resourceScore -= 30.0;
            issues += "High memory usage; ";
        } else if (memUsage > 60.0) {
            resourceScore -= 15.0;
            issues += "Moderate memory usage; ";
        }
    }
    
    // 检查CPU使用
    if (metrics.contains("cpuUsage")) {
        double cpuUsage = metrics["cpuUsage"].toDouble();
        if (cpuUsage > 80.0) {
            resourceScore -= 25.0;
            issues += "High CPU usage; ";
        } else if (cpuUsage > 60.0) {
            resourceScore -= 10.0;
            issues += "Moderate CPU usage; ";
        }
    }
    
    // 确定状态
    if (resourceScore >= 80.0) {
        report.status = Healthy;
        report.message = issues.isEmpty() ? "Resource usage is optimal" : issues.trimmed();
    } else if (resourceScore >= 60.0) {
        report.status = Warning;
        report.message = "Resource usage is elevated: " + issues.trimmed();
    } else if (resourceScore >= 40.0) {
        report.status = Critical;
        report.message = "Resource usage is high: " + issues.trimmed();
    } else {
        report.status = Failure;
        report.message = "Resource usage is critical: " + issues.trimmed();
    }
    
    report.score = resourceScore;
    report.details = metrics;
    
    return report;
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performConnectivityCheck(const QString& moduleName)
{
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    // 检查网络连接（针对网络相关模块）
    if (moduleName.contains("network", Qt::CaseInsensitive) || 
        moduleName.contains("chat", Qt::CaseInsensitive) ||
        moduleName.contains("meeting", Qt::CaseInsensitive)) {
        
        // 简化的连接检查
        bool connected = true; // 实际实现中应该进行真实的连接测试
        
        if (connected) {
            report.status = Healthy;
            report.message = "Connectivity is good";
            report.score = 100.0;
        } else {
            report.status = Failure;
            report.message = "No connectivity";
            report.score = 0.0;
        }
        
        report.details["connected"] = connected;
        report.details["latency"] = 50; // 模拟延迟
        
    } else {
        // 非网络模块
        report.status = Healthy;
        report.message = "Connectivity check not applicable";
        report.score = 100.0;
    }
    
    return report;
}

IHealthMonitor::HealthReport ModuleHealthMonitor::performFunctionalCheck(const QString& moduleName)
{
    HealthReport report;
    report.moduleName = moduleName;
    report.timestamp = QDateTime::currentDateTime();
    
    // 执行功能性测试
    bool functionalityWorking = true; // 实际实现中应该调用模块的自检功能
    
    if (functionalityWorking) {
        report.status = Healthy;
        report.message = "All functions are working correctly";
        report.score = 100.0;
    } else {
        report.status = Failure;
        report.message = "Some functions are not working";
        report.score = 30.0;
    }
    
    report.details["functionalTest"] = functionalityWorking;
    
    return report;
}

void ModuleHealthMonitor::startMonitoring(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_healthData.contains(moduleName)) {
        ModuleHealthData data;
        data.moduleName = moduleName;
        data.currentStatus = Unknown;
        data.currentScore = 0.0;
        data.threshold = Warning;
        data.performanceThreshold = 60.0;
        data.autoRecoveryEnabled = false;
        data.isMonitored = false;
        data.consecutiveFailures = 0;
        m_healthData[moduleName] = data;
    }
    
    m_healthData[moduleName].isMonitored = true;
    
    // 启动监控定时器（如果还没启动）
    if (!m_monitoringTimer->isActive()) {
        m_monitoringTimer->start(m_monitoringInterval);
    }
    
    emit monitoringStarted(moduleName);
    qDebug() << "Started monitoring module:" << moduleName;
}

void ModuleHealthMonitor::stopMonitoring(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_healthData.contains(moduleName)) {
        m_healthData[moduleName].isMonitored = false;
    }
    
    // 检查是否还有模块在监控中
    bool hasMonitoredModules = false;
    for (const auto& data : m_healthData) {
        if (data.isMonitored) {
            hasMonitoredModules = true;
            break;
        }
    }
    
    // 如果没有模块在监控，停止定时器
    if (!hasMonitoredModules) {
        m_monitoringTimer->stop();
    }
    
    emit monitoringStopped(moduleName);
    qDebug() << "Stopped monitoring module:" << moduleName;
}

void ModuleHealthMonitor::performScheduledCheck()
{
    QMutexLocker locker(&m_mutex);
    
    // 检查所有被监控的模块
    for (const auto& data : m_healthData) {
        if (data.isMonitored) {
            // 在线程池中异步执行健康检查
            HealthCheckTask* task = new HealthCheckTask(data.moduleName, Basic, this);
            connect(task, &HealthCheckTask::checkCompleted, 
                    this, [this](const QString& moduleName, const HealthReport& report) {
                updateHealthData(moduleName, report);
                checkThresholds(moduleName, report);
            });
            m_threadPool->start(task);
        }
    }
}

void ModuleHealthMonitor::updateHealthData(const QString& moduleName, const HealthReport& report)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_healthData.contains(moduleName)) {
        ModuleHealthData data;
        data.moduleName = moduleName;
        data.threshold = Warning;
        data.performanceThreshold = 60.0;
        data.autoRecoveryEnabled = false;
        data.isMonitored = false;
        data.consecutiveFailures = 0;
        m_healthData[moduleName] = data;
    }
    
    ModuleHealthData& data = m_healthData[moduleName];
    HealthStatus oldStatus = data.currentStatus;
    
    data.currentStatus = report.status;
    data.currentScore = report.score;
    data.lastCheckTime = report.timestamp;
    
    // 更新连续失败计数
    if (report.status == Failure || report.status == Critical) {
        data.consecutiveFailures++;
    } else {
        data.consecutiveFailures = 0;
    }
    
    // 添加到历史记录
    addToHistory(moduleName, report);
    
    // 发送状态变更信号
    if (oldStatus != report.status) {
        emit healthStatusChanged(moduleName, report.status);
    }
}

bool ModuleHealthMonitor::isModuleResponsive(const QString& moduleName)
{
    // 简化实现，实际应该测试模块的响应性
    Q_UNUSED(moduleName)
    return true;
}

double ModuleHealthMonitor::measureModulePerformance(const QString& moduleName)
{
    // 简化实现，实际应该测量真实的性能指标
    Q_UNUSED(moduleName)
    return 85.0; // 模拟性能分数
}

QVariantMap ModuleHealthMonitor::collectModuleMetrics(const QString& moduleName)
{
    // 简化实现，实际应该收集真实的指标
    Q_UNUSED(moduleName)
    
    QVariantMap metrics;
    metrics["memoryUsage"] = 45.0; // 模拟内存使用率
    metrics["cpuUsage"] = 25.0;    // 模拟CPU使用率
    metrics["threadCount"] = 5;     // 模拟线程数
    metrics["handleCount"] = 120;   // 模拟句柄数
    
    return metrics;
}

// HealthCheckTask实现
HealthCheckTask::HealthCheckTask(const QString& moduleName, IHealthMonitor::CheckType type, ModuleHealthMonitor* monitor)
    : m_moduleName(moduleName)
    , m_checkType(type)
    , m_monitor(monitor)
{
    setAutoDelete(true);
}

void HealthCheckTask::run()
{
    if (m_monitor) {
        IHealthMonitor::HealthReport report = m_monitor->performHealthCheck(m_moduleName, m_checkType);
        emit checkCompleted(m_moduleName, report);
    }
}

#include "ModuleHealthMonitor.moc"