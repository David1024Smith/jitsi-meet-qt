#ifndef MEMORYLEAKDETECTOR_H
#define MEMORYLEAKDETECTOR_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QTimer>
#include <QDebug>

/**
 * @brief 内存泄漏检测器 - 用于检测和报告潜在的内存泄漏
 */
class MemoryLeakDetector : public QObject
{
    Q_OBJECT

public:
    struct AllocationInfo {
        void* address;
        size_t size;
        QString file;
        int line;
        qint64 timestamp;
    };

    explicit MemoryLeakDetector(QObject *parent = nullptr);
    ~MemoryLeakDetector();

    static MemoryLeakDetector* instance();

    // 内存分配跟踪
    void trackAllocation(void* ptr, size_t size, const QString& file = QString(), int line = 0);
    void trackDeallocation(void* ptr);

    // 泄漏检测
    void startLeakDetection();
    void stopLeakDetection();
    void performLeakCheck();

    // 统计信息
    int getAllocationCount() const;
    qint64 getTotalAllocatedMemory() const;
    QList<AllocationInfo> getPotentialLeaks() const;

    // 报告
    void generateLeakReport();
    void logMemoryStatistics();

signals:
    void memoryLeakDetected(const QList<AllocationInfo>& leaks);
    void memoryStatisticsUpdated(int allocations, qint64 totalMemory);

private slots:
    void onLeakCheckTimer();

private:
    void cleanupOldAllocations();

    static MemoryLeakDetector* s_instance;
    
    QHash<void*, AllocationInfo> m_allocations;
    mutable QMutex m_allocationsMutex;
    
    QTimer* m_leakCheckTimer;
    qint64 m_leakCheckInterval;
    qint64 m_leakThresholdTime;
    
    // 统计信息
    int m_totalAllocations;
    int m_totalDeallocations;
    qint64 m_totalAllocatedMemory;
    qint64 m_peakMemoryUsage;
};

// 便利宏用于跟踪内存分配
#ifdef QT_DEBUG
#define TRACK_ALLOCATION(ptr, size) \
    if (MemoryLeakDetector::instance()) { \
        MemoryLeakDetector::instance()->trackAllocation(ptr, size, __FILE__, __LINE__); \
    }

#define TRACK_DEALLOCATION(ptr) \
    if (MemoryLeakDetector::instance()) { \
        MemoryLeakDetector::instance()->trackDeallocation(ptr); \
    }
#else
#define TRACK_ALLOCATION(ptr, size)
#define TRACK_DEALLOCATION(ptr)
#endif

#endif // MEMORYLEAKDETECTOR_H