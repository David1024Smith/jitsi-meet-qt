#ifndef MEMORYPROFILER_H
#define MEMORYPROFILER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QElapsedTimer>
#include <QJsonObject>

/**
 * @brief 高级内存分析器 - 提供详细的内存使用分析和优化建议
 */
class MemoryProfiler : public QObject
{
    Q_OBJECT

public:
    struct MemorySnapshot {
        qint64 timestamp;
        qint64 totalMemory;
        qint64 heapMemory;
        qint64 stackMemory;
        qint64 networkMemory;
        qint64 qtObjectsMemory;
        int activeAllocations;
        double fragmentationRatio;
    };

    struct MemoryTrend {
        qint64 averageUsage;
        qint64 peakUsage;
        qint64 minimumUsage;
        double growthRate;
        int allocationRate;
        int deallocationRate;
    };

    struct OptimizationSuggestion {
        QString category;
        QString description;
        QString action;
        int priority; // 1-5, 5 being highest
        qint64 potentialSavings;
    };

    explicit MemoryProfiler(QObject *parent = nullptr);
    ~MemoryProfiler();

    // 分析控制
    void startProfiling();
    void stopProfiling();
    void pauseProfiling();
    void resumeProfiling();

    // 快照管理
    void takeSnapshot();
    MemorySnapshot getCurrentSnapshot() const;
    QList<MemorySnapshot> getSnapshotHistory() const;
    void clearSnapshotHistory();

    // 趋势分析
    MemoryTrend analyzeTrend(int periodMinutes = 10) const;
    QList<OptimizationSuggestion> generateOptimizationSuggestions() const;

    // 报告生成
    QJsonObject generateDetailedReport() const;
    QString generateTextReport() const;
    void exportReport(const QString& filePath) const;

    // 配置
    void setSnapshotInterval(int milliseconds);
    void setMaxSnapshots(int maxSnapshots);
    void setProfilingEnabled(bool enabled);

    // 统计信息
    int getSnapshotCount() const;
    qint64 getProfilingDuration() const;
    bool isProfilingActive() const;

signals:
    void snapshotTaken(const MemorySnapshot& snapshot);
    void memoryTrendChanged(const MemoryTrend& trend);
    void optimizationSuggestionAvailable(const OptimizationSuggestion& suggestion);
    void memoryLeakSuspected(qint64 suspectedSize);

private slots:
    void onSnapshotTimer();
    void onAnalysisTimer();

private:
    void analyzeMemoryUsage();
    void detectMemoryLeaks();
    void calculateFragmentation();
    qint64 estimateQtObjectsMemory();
    qint64 estimateNetworkMemory();
    double calculateGrowthRate() const;
    OptimizationSuggestion createSuggestion(const QString& category, 
                                           const QString& description,
                                           const QString& action,
                                           int priority,
                                           qint64 savings) const;

    QTimer* m_snapshotTimer;
    QTimer* m_analysisTimer;
    QElapsedTimer m_profilingTimer;
    
    mutable QMutex m_snapshotsMutex;
    QList<MemorySnapshot> m_snapshots;
    
    // 配置
    int m_snapshotInterval;
    int m_maxSnapshots;
    bool m_profilingEnabled;
    bool m_isActive;
    
    // 分析数据
    qint64 m_baselineMemory;
    qint64 m_lastAnalysisTime;
    QHash<QString, qint64> m_componentMemoryUsage;
};

#endif // MEMORYPROFILER_H