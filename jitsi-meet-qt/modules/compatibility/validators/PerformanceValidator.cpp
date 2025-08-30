#include "PerformanceValidator.h"
#include <QDebug>
#include <QMutexLocker>
#include <QThread>
#include <QCoreApplication>
#include <QProcess>
#include <QDateTime>

PerformanceValidator::PerformanceValidator(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
    // 设置默认性能阈值
    // 构造函数中已经通过结构体初始化设置了默认值
}

PerformanceValidator::~PerformanceValidator()
{
}

bool PerformanceValidator::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing PerformanceValidator...";
    
    // 注册元类型以支持信号槽
    qRegisterMetaType<PerformanceMetrics>("PerformanceMetrics");
    
    m_initialized = true;
    qDebug() << "PerformanceValidator initialized successfully";
    
    return true;
}

bool PerformanceValidator::validateModulePerformance(const QString& moduleName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        qWarning() << "PerformanceValidator not initialized";
        return false;
    }

    qDebug() << "Validating performance for module:" << moduleName;
    
    PerformanceMetrics metrics = measurePerformance(moduleName);
    
    // 记录性能历史
    recordPerformanceHistory(moduleName, metrics);
    
    // 发射信号
    emit performanceMeasured(moduleName, metrics);
    
    // 验证性能指标
    bool isValid = validateMetrics(metrics);
    
    if (!isValid) {
        qWarning() << "Performance validation failed for module:" << moduleName;
        
        // 检查具体哪些指标超出阈值
        if (metrics.cpuUsage > m_thresholds.maxCpuUsage) {
            emit performanceThresholdExceeded(moduleName, "cpu_usage", metrics.cpuUsage);
        }
        if (metrics.memoryUsage > m_thresholds.maxMemoryUsage) {
            emit performanceThresholdExceeded(moduleName, "memory_usage", metrics.memoryUsage);
        }
        if (metrics.executionTime > m_thresholds.maxExecutionTime) {
            emit performanceThresholdExceeded(moduleName, "execution_time", metrics.executionTime);
        }
        if (metrics.throughput < m_thresholds.minThroughput) {
            emit performanceThresholdExceeded(moduleName, "throughput", metrics.throughput);
        }
        if (metrics.latency > m_thresholds.maxLatency) {
            emit performanceThresholdExceeded(moduleName, "latency", metrics.latency);
        }
    }
    
    qDebug() << "Performance validation for module" << moduleName << (isValid ? "PASSED" : "FAILED");
    return isValid;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measurePerformance(const QString& moduleName)
{
    PerformanceMetrics metrics;
    
    if (moduleName == "audio") {
        metrics = measureAudioPerformance();
    } else if (moduleName == "network") {
        metrics = measureNetworkPerformance();
    } else if (moduleName == "ui") {
        metrics = measureUIPerformance();
    } else if (moduleName == "chat") {
        metrics = measureChatPerformance();
    } else if (moduleName == "screenshare") {
        metrics = measureScreenSharePerformance();
    } else if (moduleName == "meeting") {
        metrics = measureMeetingPerformance();
    } else if (moduleName == "performance") {
        metrics = measurePerformanceModulePerformance();
    } else if (moduleName == "settings") {
        metrics = measureSettingsPerformance();
    } else if (moduleName == "utils") {
        metrics = measureUtilsPerformance();
    } else {
        qWarning() << "Unknown module for performance measurement:" << moduleName;
        // 返回默认值
        metrics.cpuUsage = 0.0;
        metrics.memoryUsage = 0;
        metrics.executionTime = 0.0;
        metrics.throughput = 0.0;
        metrics.latency = 0.0;
    }
    
    return metrics;
}

void PerformanceValidator::setPerformanceThresholds(const PerformanceThresholds& thresholds)
{
    QMutexLocker locker(&m_mutex);
    m_thresholds = thresholds;
}

PerformanceValidator::PerformanceThresholds PerformanceValidator::getPerformanceThresholds() const
{
    QMutexLocker locker(&m_mutex);
    return m_thresholds;
}

QVariantMap PerformanceValidator::getPerformanceReport(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap report;
    report["module"] = moduleName;
    report["timestamp"] = QDateTime::currentDateTime();
    
    if (m_performanceHistory.contains(moduleName) && !m_performanceHistory[moduleName].isEmpty()) {
        const PerformanceMetrics& latest = m_performanceHistory[moduleName].last();
        
        report["cpu_usage"] = latest.cpuUsage;
        report["memory_usage"] = static_cast<qint64>(latest.memoryUsage);
        report["execution_time"] = latest.executionTime;
        report["throughput"] = latest.throughput;
        report["latency"] = latest.latency;
        
        // 添加阈值信息
        QVariantMap thresholds;
        thresholds["max_cpu_usage"] = m_thresholds.maxCpuUsage;
        thresholds["max_memory_usage"] = static_cast<qint64>(m_thresholds.maxMemoryUsage);
        thresholds["max_execution_time"] = m_thresholds.maxExecutionTime;
        thresholds["min_throughput"] = m_thresholds.minThroughput;
        thresholds["max_latency"] = m_thresholds.maxLatency;
        report["thresholds"] = thresholds;
        
        // 添加验证结果
        report["validation_passed"] = validateMetrics(latest);
        
        // 添加历史统计
        const auto& history = m_performanceHistory[moduleName];
        if (history.size() > 1) {
            double avgCpu = 0.0, avgMemory = 0.0, avgTime = 0.0, avgThroughput = 0.0, avgLatency = 0.0;
            
            for (const auto& metrics : history) {
                avgCpu += metrics.cpuUsage;
                avgMemory += metrics.memoryUsage;
                avgTime += metrics.executionTime;
                avgThroughput += metrics.throughput;
                avgLatency += metrics.latency;
            }
            
            int count = history.size();
            QVariantMap averages;
            averages["cpu_usage"] = avgCpu / count;
            averages["memory_usage"] = static_cast<qint64>(avgMemory / count);
            averages["execution_time"] = avgTime / count;
            averages["throughput"] = avgThroughput / count;
            averages["latency"] = avgLatency / count;
            report["averages"] = averages;
        }
    }
    
    return report;
}

QStringList PerformanceValidator::getPerformanceHistory(const QString& moduleName) const
{
    QMutexLocker locker(&m_mutex);
    
    QStringList history;
    
    if (m_performanceHistory.contains(moduleName)) {
        const auto& metrics = m_performanceHistory[moduleName];
        
        for (const auto& metric : metrics) {
            QString entry = QString("CPU: %1%, Memory: %2MB, Time: %3ms, Throughput: %4ops/s, Latency: %5ms")
                           .arg(metric.cpuUsage, 0, 'f', 2)
                           .arg(metric.memoryUsage / (1024 * 1024))
                           .arg(metric.executionTime, 0, 'f', 2)
                           .arg(metric.throughput, 0, 'f', 2)
                           .arg(metric.latency, 0, 'f', 2);
            history.append(entry);
        }
    }
    
    return history;
}

void PerformanceValidator::clearPerformanceHistory()
{
    QMutexLocker locker(&m_mutex);
    m_performanceHistory.clear();
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureAudioPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    // 模拟音频性能测试
    qDebug() << "Measuring audio performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟音频处理操作
    QThread::msleep(100);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 48000.0; // 模拟48kHz采样率
    metrics.latency = 20.0; // 模拟20ms延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureNetworkPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring network performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟网络操作
    QThread::msleep(150);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 1000.0; // 模拟1000 packets/sec
    metrics.latency = 50.0; // 模拟50ms网络延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureUIPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring UI performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟UI渲染操作
    QThread::msleep(50);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 60.0; // 模拟60 FPS
    metrics.latency = 16.7; // 模拟16.7ms渲染延迟 (60 FPS)
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureChatPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring chat performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟聊天操作
    QThread::msleep(80);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 100.0; // 模拟100 messages/sec
    metrics.latency = 100.0; // 模拟100ms消息延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureScreenSharePerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring screenshare performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟屏幕捕获和编码操作
    QThread::msleep(200);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 30.0; // 模拟30 FPS
    metrics.latency = 33.3; // 模拟33.3ms编码延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureMeetingPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring meeting performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟会议管理操作
    QThread::msleep(120);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 50.0; // 模拟50 operations/sec
    metrics.latency = 200.0; // 模拟200ms会议操作延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measurePerformanceModulePerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring performance module performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟性能监控操作
    QThread::msleep(60);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 1000.0; // 模拟1000 metrics/sec
    metrics.latency = 10.0; // 模拟10ms监控延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureSettingsPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring settings performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟设置操作
    QThread::msleep(40);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 200.0; // 模拟200 settings/sec
    metrics.latency = 5.0; // 模拟5ms设置延迟
    
    return metrics;
}

PerformanceValidator::PerformanceMetrics PerformanceValidator::measureUtilsPerformance()
{
    PerformanceMetrics metrics;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Measuring utils performance...";
    
    double startCpu = getCurrentCpuUsage();
    qint64 startMemory = getCurrentMemoryUsage();
    
    // 模拟工具操作
    QThread::msleep(30);
    
    metrics.cpuUsage = getCurrentCpuUsage() - startCpu;
    metrics.memoryUsage = getCurrentMemoryUsage() - startMemory;
    metrics.executionTime = timer.elapsed();
    metrics.throughput = 500.0; // 模拟500 operations/sec
    metrics.latency = 2.0; // 模拟2ms工具延迟
    
    return metrics;
}

double PerformanceValidator::getCurrentCpuUsage()
{
    // 简化的CPU使用率获取
    // 实际实现应该使用系统API获取真实的CPU使用率
    static double simulatedCpu = 10.0;
    simulatedCpu += (qrand() % 20 - 10) * 0.1; // 模拟波动
    if (simulatedCpu < 0) simulatedCpu = 0;
    if (simulatedCpu > 100) simulatedCpu = 100;
    return simulatedCpu;
}

qint64 PerformanceValidator::getCurrentMemoryUsage()
{
    // 简化的内存使用获取
    // 实际实现应该使用系统API获取真实的内存使用
    static qint64 simulatedMemory = 100 * 1024 * 1024; // 100MB基础
    simulatedMemory += (qrand() % 20 - 10) * 1024 * 1024; // 模拟波动
    if (simulatedMemory < 0) simulatedMemory = 0;
    return simulatedMemory;
}

double PerformanceValidator::measureExecutionTime(std::function<void()> operation)
{
    QElapsedTimer timer;
    timer.start();
    
    if (operation) {
        operation();
    }
    
    return timer.elapsed();
}

bool PerformanceValidator::validateMetrics(const PerformanceMetrics& metrics)
{
    return metrics.cpuUsage <= m_thresholds.maxCpuUsage &&
           metrics.memoryUsage <= m_thresholds.maxMemoryUsage &&
           metrics.executionTime <= m_thresholds.maxExecutionTime &&
           metrics.throughput >= m_thresholds.minThroughput &&
           metrics.latency <= m_thresholds.maxLatency;
}

void PerformanceValidator::recordPerformanceHistory(const QString& moduleName, const PerformanceMetrics& metrics)
{
    if (!m_performanceHistory.contains(moduleName)) {
        m_performanceHistory[moduleName] = QList<PerformanceMetrics>();
    }
    
    m_performanceHistory[moduleName].append(metrics);
    
    // 限制历史记录数量
    if (m_performanceHistory[moduleName].size() > 100) {
        m_performanceHistory[moduleName].removeFirst();
    }
}