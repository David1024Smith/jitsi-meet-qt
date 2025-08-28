#ifndef MEMORYLEAKDETECTOR_H
#define MEMORYLEAKDETECTOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * @brief 内存泄漏检测器
 * 
 * 提供内存分配跟踪、泄漏检测和资源清理功能
 */
class MemoryLeakDetector : public QObject
{
    Q_OBJECT

public:
    struct AllocationInfo {
        size_t size;
        QString file;
        int line;
        std::chrono::steady_clock::time_point timestamp;
    };

    struct MemoryStats {
        size_t totalAllocations{0};
        size_t totalDeallocations{0};
        size_t currentAllocations{0};
        size_t peakAllocations{0};
        size_t totalBytesAllocated{0};
        size_t currentBytesAllocated{0};
        size_t peakBytesAllocated{0};
    };

    explicit MemoryLeakDetector(QObject *parent = nullptr);
    ~MemoryLeakDetector();

    static MemoryLeakDetector* instance();

    // 内存跟踪
    void trackAllocation(void* ptr, size_t size, const QString& file = QString(), int line = 0);
    void trackDeallocation(void* ptr);

    // 泄漏检测
    void startLeakDetection();
    void stopLeakDetection();
    std::vector<AllocationInfo> detectLeaks() const;

    // 统计信息
    MemoryStats getMemoryStats() const;
    void resetStats();

    // 资源清理
    void forceGarbageCollection();
    void cleanupUnusedResources();

    // 配置
    void setTrackingEnabled(bool enabled);
    void setLeakDetectionInterval(int seconds);

signals:
    void memoryLeakDetected(int leakCount, size_t totalLeakedBytes);
    void memoryStatsUpdated(const MemoryStats& stats);

private slots:
    void performLeakCheck();

private:
    void updateStats();
    
    static MemoryLeakDetector* s_instance;
    
    mutable QMutex m_mutex;
    QTimer* m_leakCheckTimer;
    
    std::unordered_map<void*, AllocationInfo> m_allocations;
    MemoryStats m_stats;
    
    bool m_trackingEnabled;
    bool m_leakDetectionActive;
};

// 便利宏用于内存跟踪
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