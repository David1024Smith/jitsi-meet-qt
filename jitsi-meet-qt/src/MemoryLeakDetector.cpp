#include "MemoryLeakDetector.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

MemoryLeakDetector* MemoryLeakDetector::s_instance = nullptr;

MemoryLeakDetector::MemoryLeakDetector(QObject *parent)
    : QObject(parent)
    , m_leakCheckTimer(new QTimer(this))
    , m_leakCheckInterval(60000) // 1分钟检查一次
    , m_leakThresholdTime(300000) // 5分钟阈值
    , m_totalAllocations(0)
    , m_totalDeallocations(0)
    , m_totalAllocatedMemory(0)
    , m_peakMemoryUsage(0)
{
    s_instance = this;
    
    // 设置泄漏检查定时器
    m_leakCheckTimer->setInterval(m_leakCheckInterval);
    connect(m_leakCheckTimer, &QTimer::timeout, this, &MemoryLeakDetector::onLeakCheckTimer);
    
    qDebug() << "MemoryLeakDetector: Initialized";
}

MemoryLeakDetector::~MemoryLeakDetector()
{
    stopLeakDetection();
    
    // 生成最终报告
    if (!m_allocations.isEmpty()) {
        qWarning() << "MemoryLeakDetector: Potential memory leaks detected at shutdown";
        generateLeakReport();
    }
    
    s_instance = nullptr;
}

MemoryLeakDetector* MemoryLeakDetector::instance()
{
    return s_instance;
}

void MemoryLeakDetector::trackAllocation(void* ptr, size_t size, const QString& file, int line)
{
    if (!ptr) {
        return;
    }
    
    QMutexLocker locker(&m_allocationsMutex);
    
    AllocationInfo info;
    info.address = ptr;
    info.size = size;
    info.file = file;
    info.line = line;
    info.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    m_allocations[ptr] = info;
    m_totalAllocations++;
    m_totalAllocatedMemory += size;
    
    if (m_totalAllocatedMemory > m_peakMemoryUsage) {
        m_peakMemoryUsage = m_totalAllocatedMemory;
    }
    
    emit memoryStatisticsUpdated(m_allocations.size(), m_totalAllocatedMemory);
}

void MemoryLeakDetector::trackDeallocation(void* ptr)
{
    if (!ptr) {
        return;
    }
    
    QMutexLocker locker(&m_allocationsMutex);
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        m_totalAllocatedMemory -= it->size;
        m_allocations.erase(it);
        m_totalDeallocations++;
        
        emit memoryStatisticsUpdated(m_allocations.size(), m_totalAllocatedMemory);
    }
}

void MemoryLeakDetector::startLeakDetection()
{
    if (!m_leakCheckTimer->isActive()) {
        m_leakCheckTimer->start();
        qDebug() << "MemoryLeakDetector: Leak detection started";
    }
}

void MemoryLeakDetector::stopLeakDetection()
{
    m_leakCheckTimer->stop();
    qDebug() << "MemoryLeakDetector: Leak detection stopped";
}

void MemoryLeakDetector::performLeakCheck()
{
    QMutexLocker locker(&m_allocationsMutex);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QList<AllocationInfo> potentialLeaks;
    
    for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
        const AllocationInfo& info = it.value();
        qint64 age = currentTime - info.timestamp;
        
        if (age > m_leakThresholdTime) {
            potentialLeaks.append(info);
        }
    }
    
    if (!potentialLeaks.isEmpty()) {
        qWarning() << "MemoryLeakDetector: Found" << potentialLeaks.size() << "potential memory leaks";
        emit memoryLeakDetected(potentialLeaks);
    }
    
    // 清理过期的分配记录
    cleanupOldAllocations();
}

int MemoryLeakDetector::getAllocationCount() const
{
    QMutexLocker locker(&m_allocationsMutex);
    return m_allocations.size();
}

qint64 MemoryLeakDetector::getTotalAllocatedMemory() const
{
    QMutexLocker locker(&m_allocationsMutex);
    return m_totalAllocatedMemory;
}

QList<MemoryLeakDetector::AllocationInfo> MemoryLeakDetector::getPotentialLeaks() const
{
    QMutexLocker locker(&m_allocationsMutex);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QList<AllocationInfo> leaks;
    
    for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
        const AllocationInfo& info = it.value();
        qint64 age = currentTime - info.timestamp;
        
        if (age > m_leakThresholdTime) {
            leaks.append(info);
        }
    }
    
    return leaks;
}

void MemoryLeakDetector::generateLeakReport()
{
    QList<AllocationInfo> leaks = getPotentialLeaks();
    
    if (leaks.isEmpty()) {
        qDebug() << "MemoryLeakDetector: No memory leaks detected";
        return;
    }
    
    qWarning() << "=== Memory Leak Report ===";
    qWarning() << "Total potential leaks:" << leaks.size();
    
    qint64 totalLeakedMemory = 0;
    QHash<QString, int> leaksByFile;
    
    for (const AllocationInfo& leak : leaks) {
        totalLeakedMemory += leak.size;
        
        QString location = leak.file.isEmpty() ? "Unknown" : 
                          QString("%1:%2").arg(leak.file).arg(leak.line);
        leaksByFile[location]++;
        
        qint64 age = QDateTime::currentMSecsSinceEpoch() - leak.timestamp;
        qWarning() << "Leak: Address=" << leak.address 
                   << "Size=" << leak.size 
                   << "Age=" << age/1000 << "s"
                   << "Location=" << location;
    }
    
    qWarning() << "Total leaked memory:" << totalLeakedMemory << "bytes";
    qWarning() << "Leaks by location:";
    
    for (auto it = leaksByFile.begin(); it != leaksByFile.end(); ++it) {
        qWarning() << "  " << it.key() << ":" << it.value() << "leaks";
    }
    
    qWarning() << "========================";
}

void MemoryLeakDetector::logMemoryStatistics()
{
    QMutexLocker locker(&m_allocationsMutex);
    
    qDebug() << "=== Memory Statistics ===";
    qDebug() << "Active allocations:" << m_allocations.size();
    qDebug() << "Total allocations:" << m_totalAllocations;
    qDebug() << "Total deallocations:" << m_totalDeallocations;
    qDebug() << "Current allocated memory:" << m_totalAllocatedMemory << "bytes";
    qDebug() << "Peak memory usage:" << m_peakMemoryUsage << "bytes";
    qDebug() << "Allocation/Deallocation ratio:" 
             << (m_totalDeallocations > 0 ? (double)m_totalAllocations / m_totalDeallocations : 0);
    qDebug() << "========================";
}

void MemoryLeakDetector::onLeakCheckTimer()
{
    performLeakCheck();
}

void MemoryLeakDetector::cleanupOldAllocations()
{
    // 清理超过1小时的分配记录以避免内存占用过多
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 cleanupThreshold = 3600000; // 1小时
    
    auto it = m_allocations.begin();
    while (it != m_allocations.end()) {
        qint64 age = currentTime - it->timestamp;
        if (age > cleanupThreshold) {
            it = m_allocations.erase(it);
        } else {
            ++it;
        }
    }
}