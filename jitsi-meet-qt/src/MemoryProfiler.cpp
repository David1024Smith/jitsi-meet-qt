#include "MemoryProfiler.h"
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QCoreApplication>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

MemoryProfiler::MemoryProfiler(QObject *parent)
    : QObject(parent)
    , m_snapshotTimer(new QTimer(this))
    , m_analysisTimer(new QTimer(this))
    , m_snapshotInterval(5000) // 5秒
    , m_maxSnapshots(1000)
    , m_profilingEnabled(true)
    , m_isActive(false)
    , m_baselineMemory(0)
    , m_lastAnalysisTime(0)
{
    // 设置快照定时器
    m_snapshotTimer->setInterval(m_snapshotInterval);
    connect(m_snapshotTimer, &QTimer::timeout, this, &MemoryProfiler::onSnapshotTimer);
    
    // 设置分析定时器
    m_analysisTimer->setInterval(60000); // 1分钟分析一次
    connect(m_analysisTimer, &QTimer::timeout, this, &MemoryProfiler::onAnalysisTimer);
    
    qDebug() << "MemoryProfiler: Initialized";
}

MemoryProfiler::~MemoryProfiler()
{
    stopProfiling();
}

void MemoryProfiler::startProfiling()
{
    if (m_isActive || !m_profilingEnabled) {
        return;
    }
    
    m_isActive = true;
    m_profilingTimer.start();
    
    // 记录基线内存使用
    takeSnapshot();
    if (!m_snapshots.isEmpty()) {
        m_baselineMemory = m_snapshots.first().totalMemory;
    }
    
    m_snapshotTimer->start();
    m_analysisTimer->start();
    
    qDebug() << "MemoryProfiler: Profiling started, baseline memory:" 
             << m_baselineMemory / (1024*1024) << "MB";
}

void MemoryProfiler::stopProfiling()
{
    if (!m_isActive) {
        return;
    }
    
    m_isActive = false;
    m_snapshotTimer->stop();
    m_analysisTimer->stop();
    
    qDebug() << "MemoryProfiler: Profiling stopped after" 
             << m_profilingTimer.elapsed() / 1000 << "seconds";
}

void MemoryProfiler::pauseProfiling()
{
    if (m_isActive) {
        m_snapshotTimer->stop();
        m_analysisTimer->stop();
        qDebug() << "MemoryProfiler: Profiling paused";
    }
}

void MemoryProfiler::resumeProfiling()
{
    if (m_isActive) {
        m_snapshotTimer->start();
        m_analysisTimer->start();
        qDebug() << "MemoryProfiler: Profiling resumed";
    }
}

void MemoryProfiler::takeSnapshot()
{
    MemorySnapshot snapshot;
    snapshot.timestamp = QDateTime::currentMSecsSinceEpoch();
    
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        snapshot.totalMemory = pmc.WorkingSetSize;
        snapshot.heapMemory = pmc.PrivateUsage;
    }
#endif
    
    // 估算各组件内存使用
    snapshot.webEngineMemory = estimateWebEngineMemory();
    snapshot.qtObjectsMemory = estimateQtObjectsMemory();
    snapshot.stackMemory = snapshot.totalMemory - snapshot.heapMemory;
    
    // 计算碎片化率
    calculateFragmentation();
    snapshot.fragmentationRatio = 0.1; // 简化实现
    
    // 活跃分配数量（简化实现）
    snapshot.activeAllocations = QCoreApplication::instance()->children().size();
    
    QMutexLocker locker(&m_snapshotsMutex);
    m_snapshots.append(snapshot);
    
    // 限制快照数量
    while (m_snapshots.size() > m_maxSnapshots) {
        m_snapshots.removeFirst();
    }
    
    emit snapshotTaken(snapshot);
    
    qDebug() << "MemoryProfiler: Snapshot taken - Total:" 
             << snapshot.totalMemory / (1024*1024) << "MB"
             << "WebEngine:" << snapshot.webEngineMemory / (1024*1024) << "MB";
}

MemoryProfiler::MemorySnapshot MemoryProfiler::getCurrentSnapshot() const
{
    QMutexLocker locker(&m_snapshotsMutex);
    return m_snapshots.isEmpty() ? MemorySnapshot() : m_snapshots.last();
}

QList<MemoryProfiler::MemorySnapshot> MemoryProfiler::getSnapshotHistory() const
{
    QMutexLocker locker(&m_snapshotsMutex);
    return m_snapshots;
}

void MemoryProfiler::clearSnapshotHistory()
{
    QMutexLocker locker(&m_snapshotsMutex);
    m_snapshots.clear();
    qDebug() << "MemoryProfiler: Snapshot history cleared";
}

MemoryProfiler::MemoryTrend MemoryProfiler::analyzeTrend(int periodMinutes) const
{
    MemoryTrend trend;
    
    QMutexLocker locker(&m_snapshotsMutex);
    
    if (m_snapshots.isEmpty()) {
        return trend;
    }
    
    qint64 cutoffTime = QDateTime::currentMSecsSinceEpoch() - (periodMinutes * 60 * 1000);
    
    QList<MemorySnapshot> recentSnapshots;
    for (const MemorySnapshot& snapshot : m_snapshots) {
        if (snapshot.timestamp >= cutoffTime) {
            recentSnapshots.append(snapshot);
        }
    }
    
    if (recentSnapshots.isEmpty()) {
        return trend;
    }
    
    // 计算统计信息
    qint64 totalMemory = 0;
    trend.peakUsage = 0;
    trend.minimumUsage = LLONG_MAX;
    
    for (const MemorySnapshot& snapshot : recentSnapshots) {
        totalMemory += snapshot.totalMemory;
        trend.peakUsage = qMax(trend.peakUsage, snapshot.totalMemory);
        trend.minimumUsage = qMin(trend.minimumUsage, snapshot.totalMemory);
    }
    
    trend.averageUsage = totalMemory / recentSnapshots.size();
    trend.growthRate = calculateGrowthRate();
    
    // 简化的分配/释放率计算
    trend.allocationRate = recentSnapshots.size() * 10; // 估算
    trend.deallocationRate = recentSnapshots.size() * 8; // 估算
    
    return trend;
}

QList<MemoryProfiler::OptimizationSuggestion> MemoryProfiler::generateOptimizationSuggestions() const
{
    QList<OptimizationSuggestion> suggestions;
    
    MemoryTrend trend = analyzeTrend(10);
    MemorySnapshot current = getCurrentSnapshot();
    
    // 检查内存增长率
    if (trend.growthRate > 0.1) { // 10%增长率
        suggestions.append(createSuggestion(
            "Memory Growth",
            "Memory usage is growing rapidly",
            "Consider implementing more aggressive garbage collection",
            4,
            current.totalMemory * 0.2
        ));
    }
    
    // 检查WebEngine内存使用
    if (current.webEngineMemory > current.totalMemory * 0.7) {
        suggestions.append(createSuggestion(
            "WebEngine Memory",
            "WebEngine is using excessive memory",
            "Clear WebEngine cache and optimize web content",
            5,
            current.webEngineMemory * 0.3
        ));
    }
    
    // 检查碎片化
    if (current.fragmentationRatio > 0.3) {
        suggestions.append(createSuggestion(
            "Memory Fragmentation",
            "High memory fragmentation detected",
            "Restart application or implement memory compaction",
            3,
            current.totalMemory * 0.1
        ));
    }
    
    // 检查Qt对象数量
    if (current.activeAllocations > 10000) {
        suggestions.append(createSuggestion(
            "Object Count",
            "Large number of active Qt objects",
            "Review object lifecycle and implement object pooling",
            3,
            current.qtObjectsMemory * 0.2
        ));
    }
    
    return suggestions;
}

QJsonObject MemoryProfiler::generateDetailedReport() const
{
    QJsonObject report;
    
    // 基本信息
    report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["profilingDuration"] = getProfilingDuration();
    report["snapshotCount"] = getSnapshotCount();
    
    // 当前快照
    MemorySnapshot current = getCurrentSnapshot();
    QJsonObject currentObj;
    currentObj["totalMemory"] = current.totalMemory;
    currentObj["heapMemory"] = current.heapMemory;
    currentObj["webEngineMemory"] = current.webEngineMemory;
    currentObj["qtObjectsMemory"] = current.qtObjectsMemory;
    currentObj["activeAllocations"] = current.activeAllocations;
    currentObj["fragmentationRatio"] = current.fragmentationRatio;
    report["currentSnapshot"] = currentObj;
    
    // 趋势分析
    MemoryTrend trend = analyzeTrend(10);
    QJsonObject trendObj;
    trendObj["averageUsage"] = trend.averageUsage;
    trendObj["peakUsage"] = trend.peakUsage;
    trendObj["minimumUsage"] = trend.minimumUsage;
    trendObj["growthRate"] = trend.growthRate;
    trendObj["allocationRate"] = trend.allocationRate;
    trendObj["deallocationRate"] = trend.deallocationRate;
    report["trend"] = trendObj;
    
    // 优化建议
    QJsonArray suggestionsArray;
    QList<OptimizationSuggestion> suggestions = generateOptimizationSuggestions();
    for (const OptimizationSuggestion& suggestion : suggestions) {
        QJsonObject suggestionObj;
        suggestionObj["category"] = suggestion.category;
        suggestionObj["description"] = suggestion.description;
        suggestionObj["action"] = suggestion.action;
        suggestionObj["priority"] = suggestion.priority;
        suggestionObj["potentialSavings"] = suggestion.potentialSavings;
        suggestionsArray.append(suggestionObj);
    }
    report["suggestions"] = suggestionsArray;
    
    return report;
}

QString MemoryProfiler::generateTextReport() const
{
    QString report;
    
    report += "=== Memory Profiler Report ===\n";
    report += QString("Generated: %1\n").arg(QDateTime::currentDateTime().toString());
    report += QString("Profiling Duration: %1 seconds\n").arg(getProfilingDuration() / 1000);
    report += QString("Snapshots Collected: %1\n\n").arg(getSnapshotCount());
    
    // 当前状态
    MemorySnapshot current = getCurrentSnapshot();
    report += "Current Memory Usage:\n";
    report += QString("  Total Memory: %1 MB\n").arg(current.totalMemory / (1024*1024));
    report += QString("  Heap Memory: %1 MB\n").arg(current.heapMemory / (1024*1024));
    report += QString("  WebEngine Memory: %1 MB\n").arg(current.webEngineMemory / (1024*1024));
    report += QString("  Qt Objects Memory: %1 MB\n").arg(current.qtObjectsMemory / (1024*1024));
    report += QString("  Active Allocations: %1\n").arg(current.activeAllocations);
    report += QString("  Fragmentation Ratio: %1%\n\n").arg(current.fragmentationRatio * 100, 0, 'f', 1);
    
    // 趋势分析
    MemoryTrend trend = analyzeTrend(10);
    report += "Memory Trend (Last 10 minutes):\n";
    report += QString("  Average Usage: %1 MB\n").arg(trend.averageUsage / (1024*1024));
    report += QString("  Peak Usage: %1 MB\n").arg(trend.peakUsage / (1024*1024));
    report += QString("  Minimum Usage: %1 MB\n").arg(trend.minimumUsage / (1024*1024));
    report += QString("  Growth Rate: %1%\n").arg(trend.growthRate * 100, 0, 'f', 2);
    report += QString("  Allocation Rate: %1/min\n").arg(trend.allocationRate);
    report += QString("  Deallocation Rate: %1/min\n\n").arg(trend.deallocationRate);
    
    // 优化建议
    QList<OptimizationSuggestion> suggestions = generateOptimizationSuggestions();
    if (!suggestions.isEmpty()) {
        report += "Optimization Suggestions:\n";
        for (const OptimizationSuggestion& suggestion : suggestions) {
            report += QString("  [Priority %1] %2: %3\n")
                     .arg(suggestion.priority)
                     .arg(suggestion.category)
                     .arg(suggestion.description);
            report += QString("    Action: %1\n").arg(suggestion.action);
            report += QString("    Potential Savings: %1 MB\n\n")
                     .arg(suggestion.potentialSavings / (1024*1024));
        }
    }
    
    report += "===============================\n";
    
    return report;
}

void MemoryProfiler::exportReport(const QString& filePath) const
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (filePath.endsWith(".json")) {
            QJsonDocument doc(generateDetailedReport());
            file.write(doc.toJson());
        } else {
            file.write(generateTextReport().toUtf8());
        }
        qDebug() << "MemoryProfiler: Report exported to" << filePath;
    } else {
        qWarning() << "MemoryProfiler: Failed to export report to" << filePath;
    }
}

void MemoryProfiler::setSnapshotInterval(int milliseconds)
{
    m_snapshotInterval = milliseconds;
    m_snapshotTimer->setInterval(milliseconds);
    qDebug() << "MemoryProfiler: Snapshot interval set to" << milliseconds << "ms";
}

void MemoryProfiler::setMaxSnapshots(int maxSnapshots)
{
    m_maxSnapshots = maxSnapshots;
    
    QMutexLocker locker(&m_snapshotsMutex);
    while (m_snapshots.size() > m_maxSnapshots) {
        m_snapshots.removeFirst();
    }
    
    qDebug() << "MemoryProfiler: Max snapshots set to" << maxSnapshots;
}

void MemoryProfiler::setProfilingEnabled(bool enabled)
{
    m_profilingEnabled = enabled;
    if (!enabled && m_isActive) {
        stopProfiling();
    }
    qDebug() << "MemoryProfiler: Profiling" << (enabled ? "enabled" : "disabled");
}

int MemoryProfiler::getSnapshotCount() const
{
    QMutexLocker locker(&m_snapshotsMutex);
    return m_snapshots.size();
}

qint64 MemoryProfiler::getProfilingDuration() const
{
    return m_profilingTimer.isValid() ? m_profilingTimer.elapsed() : 0;
}

bool MemoryProfiler::isProfilingActive() const
{
    return m_isActive;
}

void MemoryProfiler::onSnapshotTimer()
{
    takeSnapshot();
}

void MemoryProfiler::onAnalysisTimer()
{
    analyzeMemoryUsage();
}

void MemoryProfiler::analyzeMemoryUsage()
{
    MemoryTrend trend = analyzeTrend(5);
    emit memoryTrendChanged(trend);
    
    // 检查内存泄漏
    detectMemoryLeaks();
    
    // 生成优化建议
    QList<OptimizationSuggestion> suggestions = generateOptimizationSuggestions();
    for (const OptimizationSuggestion& suggestion : suggestions) {
        if (suggestion.priority >= 4) {
            emit optimizationSuggestionAvailable(suggestion);
        }
    }
    
    qDebug() << "MemoryProfiler: Analysis completed - Average usage:" 
             << trend.averageUsage / (1024*1024) << "MB";
}

void MemoryProfiler::detectMemoryLeaks()
{
    if (m_snapshots.size() < 10) {
        return; // 需要足够的数据点
    }
    
    // 简化的内存泄漏检测
    QMutexLocker locker(&m_snapshotsMutex);
    
    qint64 recentAverage = 0;
    qint64 oldAverage = 0;
    
    // 计算最近5个快照的平均值
    for (int i = m_snapshots.size() - 5; i < m_snapshots.size(); ++i) {
        recentAverage += m_snapshots[i].totalMemory;
    }
    recentAverage /= 5;
    
    // 计算较早5个快照的平均值
    for (int i = m_snapshots.size() - 10; i < m_snapshots.size() - 5; ++i) {
        oldAverage += m_snapshots[i].totalMemory;
    }
    oldAverage /= 5;
    
    // 如果内存增长超过20%，可能存在泄漏
    if (recentAverage > oldAverage * 1.2) {
        qint64 suspectedLeak = recentAverage - oldAverage;
        emit memoryLeakSuspected(suspectedLeak);
        qWarning() << "MemoryProfiler: Suspected memory leak detected:" 
                   << suspectedLeak / (1024*1024) << "MB";
    }
}

void MemoryProfiler::calculateFragmentation()
{
    // 简化的碎片化计算
    // 实际实现需要更复杂的内存分析
}

qint64 MemoryProfiler::estimateQtObjectsMemory()
{
    // 估算Qt对象占用的内存
    int objectCount = QCoreApplication::instance()->children().size();
    return objectCount * 1024; // 假设每个对象平均1KB
}

qint64 MemoryProfiler::estimateWebEngineMemory()
{
    // 估算WebEngine占用的内存
    // 实际实现需要与WebEngine集成
    QMutexLocker locker(&m_snapshotsMutex);
    if (!m_snapshots.isEmpty()) {
        return m_snapshots.last().totalMemory * 0.6; // 估算60%
    }
    return 0;
}

double MemoryProfiler::calculateGrowthRate() const
{
    QMutexLocker locker(&m_snapshotsMutex);
    
    if (m_snapshots.size() < 2) {
        return 0.0;
    }
    
    qint64 oldMemory = m_snapshots.first().totalMemory;
    qint64 newMemory = m_snapshots.last().totalMemory;
    
    if (oldMemory == 0) {
        return 0.0;
    }
    
    return (double)(newMemory - oldMemory) / oldMemory;
}

MemoryProfiler::OptimizationSuggestion MemoryProfiler::createSuggestion(
    const QString& category, 
    const QString& description,
    const QString& action,
    int priority,
    qint64 savings) const
{
    OptimizationSuggestion suggestion;
    suggestion.category = category;
    suggestion.description = description;
    suggestion.action = action;
    suggestion.priority = priority;
    suggestion.potentialSavings = savings;
    return suggestion;
}