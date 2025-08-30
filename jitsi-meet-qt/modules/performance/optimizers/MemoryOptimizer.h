#ifndef MEMORYOPTIMIZER_H
#define MEMORYOPTIMIZER_H

#include "BaseOptimizer.h"
#include <QTimer>

/**
 * @brief 内存优化器类
 * 
 * MemoryOptimizer专门负责优化内存使用：
 * - 垃圾回收优化
 * - 内存池管理
 * - 缓存优化
 * - 内存泄漏修复
 */
class MemoryOptimizer : public BaseOptimizer
{
    Q_OBJECT

public:
    /**
     * @brief 内存优化策略枚举
     */
    enum MemoryStrategy {
        LowMemory,          ///< 低内存模式 - 最小化内存使用
        BalancedMemory,     ///< 平衡模式 - 平衡内存使用和性能
        HighPerformance     ///< 高性能模式 - 优先性能，适度使用内存
    };
    Q_ENUM(MemoryStrategy)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MemoryOptimizer(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MemoryOptimizer();

    /**
     * @brief 设置内存策略
     * @param strategy 内存策略
     */
    void setMemoryStrategy(MemoryStrategy strategy);

    /**
     * @brief 获取内存策略
     * @return 内存策略
     */
    MemoryStrategy memoryStrategy() const;

    /**
     * @brief 执行垃圾回收
     * @return 回收的内存大小(字节)
     */
    qint64 performGarbageCollection();

    /**
     * @brief 优化内存池
     * @return 优化是否成功
     */
    bool optimizeMemoryPools();

    /**
     * @brief 清理缓存
     * @param maxAge 最大缓存年龄(秒)
     * @return 清理的缓存大小(字节)
     */
    qint64 cleanupCaches(int maxAge = 3600);

    /**
     * @brief 压缩内存
     * @return 压缩释放的内存大小(字节)
     */
    qint64 compressMemory();

    /**
     * @brief 设置内存限制
     * @param limitMB 内存限制(MB)
     */
    void setMemoryLimit(int limitMB);

    /**
     * @brief 获取内存限制
     * @return 内存限制(MB)
     */
    int memoryLimit() const;

    /**
     * @brief 获取当前内存使用
     * @return 内存使用统计
     */
    QVariantMap getCurrentMemoryUsage() const;

    /**
     * @brief 检测内存泄漏
     * @return 泄漏检测结果
     */
    QVariantMap detectMemoryLeaks() const;

protected:
    /**
     * @brief 初始化内存优化器
     * @return 初始化是否成功
     */
    bool initializeOptimizer() override;

    /**
     * @brief 执行内存优化
     * @param strategy 优化策略
     * @return 优化结果
     */
    OptimizationResult performOptimization(OptimizationStrategy strategy) override;

    /**
     * @brief 分析是否需要内存优化
     * @return 是否需要优化
     */
    bool analyzeOptimizationNeed() const override;

    /**
     * @brief 生成内存优化建议
     * @return 优化建议列表
     */
    QStringList generateSuggestions() const override;

    /**
     * @brief 预估内存优化效果
     * @param strategy 优化策略
     * @return 预估改善效果
     */
    QVariantMap estimateOptimizationImprovements(OptimizationStrategy strategy) const override;

    /**
     * @brief 获取优化器版本
     * @return 版本字符串
     */
    QString getOptimizerVersion() const override;

    /**
     * @brief 获取优化器描述
     * @return 描述字符串
     */
    QString getOptimizerDescription() const override;

    /**
     * @brief 获取优化类型
     * @return 优化类型
     */
    OptimizationType getOptimizerType() const override;

private slots:
    /**
     * @brief 执行定期内存检查
     */
    void performPeriodicMemoryCheck();

private:
    /**
     * @brief 执行低内存优化
     * @return 优化结果
     */
    OptimizationResult performLowMemoryOptimization();

    /**
     * @brief 执行平衡内存优化
     * @return 优化结果
     */
    OptimizationResult performBalancedMemoryOptimization();

    /**
     * @brief 执行高性能内存优化
     * @return 优化结果
     */
    OptimizationResult performHighPerformanceOptimization();

    /**
     * @brief 分析内存使用模式
     * @return 分析结果
     */
    QVariantMap analyzeMemoryUsagePattern();

    /**
     * @brief 优化对象生命周期
     * @return 优化是否成功
     */
    bool optimizeObjectLifecycle();

    /**
     * @brief 调整内存分配策略
     * @return 调整是否成功
     */
    bool adjustMemoryAllocationStrategy();

    MemoryStrategy m_memoryStrategy;                ///< 内存策略
    int m_memoryLimitMB;                           ///< 内存限制(MB)
    
    QTimer* m_memoryCheckTimer;                    ///< 内存检查定时器
    
    // 内存统计
    qint64 m_totalMemoryFreed;                     ///< 总释放内存
    qint64 m_totalGarbageCollected;                ///< 总垃圾回收内存
    qint64 m_totalCacheCleared;                    ///< 总清理缓存
    
    mutable QMutex m_memoryMutex;                  ///< 内存互斥锁
};

#endif // MEMORYOPTIMIZER_H