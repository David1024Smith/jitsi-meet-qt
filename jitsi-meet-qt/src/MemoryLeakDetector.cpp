#include "../include/MemoryLeakDetector.h"
#include <QDebug>
#include <QCoreApplication>
#include <algorithm>
#include <chrono>

MemoryLeakDetector* MemoryLeakDetector::s_instance = nullptr;

MemoryLeakDetector::MemoryLeakDetector(QObject *parent)
    : QObject(parent)
    , m_leakCheckTimer(new QTimer(this))
    , m_trackingEnabled(true)
    , m_leakDetectionActive(false)
{
    s_instance = this;
    
    // 设置泄漏检测定时器
    m_leakCheckTimer->setInterval(30000); // 30秒检查一次
    connect(m_leakCheckTimer, &QTimer::timeout, this, &MemoryLeakDetector::performLeakCheck);
    
    qDebug() << "MemoryLeakDetector: Initialized";
}

MemoryLeakDetector::~MemoryLeakDetector()
{
    stopLeakDetection();
    
    // 最终泄漏检查
    auto leaks = detectLeaks();
    if (!leaks.empty()) {
        qWarning() << "MemoryLeakDetector: Found" << leaks.size() << "potential memory leaks at shutdown";
        for (const auto& leak : leaks) {
            qWarning() << "  Leak:" << leak.size << "bytes at" << leak.file << ":" << leak.line;
        }
    }
    
    s_instance = nullptr;
}

MemoryLeakDetector* MemoryLeakDetector::instance()
{
    return s_instance;
}

void MemoryLeakDetector::trackAllocation(void* ptr, size_t size, const QString& file, int line)
{
    if (!m_trackingEnabled || !ptr) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    AllocationInfo info;
    info.size = size;
    info.file = file;
    info.line = line;
    info.timestamp = std::chrono::steady_clock::now();
    
    m_allocations[ptr] = info;
    
    // 更新统计信息
    m_stats.totalAllocations++;
    m_stats.currentAllocations++;
    m_stats.totalBytesAllocated += size;
    m_stats.currentBytesAllocated += size;
    
    if (m_stats.currentAllocations > m_stats.peakAllocations) {
        m_stats.peakAllocations = m_stats.currentAllocations;
    }
    
    if (m_stats.currentBytesAllocated > m_stats.peakBytesAllocated) {
        m_stats.peakBytesAllocated = m_stats.currentBytesAllocated;
    }
}

void MemoryLeakDetector::trackDeallocation(void* ptr)
{
    if (!m_trackingEnabled || !ptr) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        // 更新统计信息
        m_stats.totalDeallocations++;
        m_stats.currentAllocations--;
        m_stats.currentBytesAllocated -= it->second.size;
        
        // 移除分配记录
        m_allocations.erase(it);
    }
}

void MemoryLeakDetector::startLeakDetection()
{
    if (!m_leakDetectionActive) {
        m_leakDetectionActive = true;
        m_leakCheckTimer->start();
        qDebug() << "MemoryLeakDetector: Leak detection started";
    }
}

void MemoryLeakDetector::stopLeakDetection()
{
    if (m_leakDetectionActive) {
        m_leakDetectionActive = false;
        m_leakCheckTimer->stop();
        qDebug() << "MemoryLeakDetector: Leak detection stopped";
    }
}

std::vector<MemoryLeakDetector::AllocationInfo> MemoryLeakDetector::detectLeaks() const
{
    QMutexLocker locker(&m_mutex);
    
    std::vector<AllocationInfo> leaks;
    auto now = std::chrono::steady_clock::now();
    
    // 检查超过5分钟未释放的内存分配
    const auto threshold = std::chrono::minutes(5);
    
    for (const auto& [ptr, info] : m_allocations) {
        if (now - info.timestamp > threshold) {
            leaks.push_back(info);
        }
    }
    
    return leaks;
}

MemoryLeakDetector::MemoryStats MemoryLeakDetector::getMemoryStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_stats;
}

void MemoryLeakDetector::resetStats()
{
    QMutexLocker locker(&m_mutex);
    m_stats = MemoryStats{};
    qDebug() << "MemoryLeakDetector: Statistics reset";
}

void MemoryLeakDetector::forceGarbageCollection()
{
    qDebug() << "MemoryLeakDetector: Forcing garbage collection";
    
    // 强制Qt进行垃圾回收
    if (QCoreApplication::instance()) {
        QCoreApplication::instance()->processEvents();
    }
    
    // 清理未使用的资源
    cleanupUnusedResources();
}

void MemoryLeakDetector::cleanupUnusedResources()
{
    qDebug() << "MemoryLeakDetector: Cleaning up unused resources";
    
    // 这里可以添加特定的资源清理逻辑
    // 例如：清理缓存、释放未使用的对象等
    
    QMutexLocker locker(&m_mutex);
    
    // 清理过期的分配记录（超过1小时）
    auto now = std::chrono::steady_clock::now();
    const auto expireThreshold = std::chrono::hours(1);
    
    auto it = m_allocations.begin();
    while (it != m_allocations.end()) {
        if (now - it->second.timestamp > expireThreshold) {
            qWarning() << "MemoryLeakDetector: Removing stale allocation record for"
                       << it->second.size << "bytes";
            it = m_allocations.erase(it);
        } else {
            ++it;
        }
    }
}

void MemoryLeakDetector::setTrackingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_trackingEnabled = enabled;
    qDebug() << "MemoryLeakDetector: Tracking" << (enabled ? "enabled" : "disabled");
}

void MemoryLeakDetector::setLeakDetectionInterval(int seconds)
{
    m_leakCheckTimer->setInterval(seconds * 1000);
    qDebug() << "MemoryLeakDetector: Leak detection interval set to" << seconds << "seconds";
}

void MemoryLeakDetector::performLeakCheck()
{
    auto leaks = detectLeaks();
    
    if (!leaks.empty()) {
        size_t totalLeakedBytes = 0;
        for (const auto& leak : leaks) {
            totalLeakedBytes += leak.size;
        }
        
        qWarning() << "MemoryLeakDetector: Found" << leaks.size() 
                   << "potential leaks totaling" << totalLeakedBytes << "bytes";
        
        emit memoryLeakDetected(static_cast<int>(leaks.size()), totalLeakedBytes);
    }
    
    // 发送统计更新
    emit memoryStatsUpdated(getMemoryStats());
    
    // 定期清理
    static int cleanupCounter = 0;
    if (++cleanupCounter >= 10) { // 每10次检查清理一次
        cleanupUnusedResources();
        cleanupCounter = 0;
    }
}