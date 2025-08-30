#ifndef IOPTIMIZER_H
#define IOPTIMIZER_H

#include <QObject>
#include <QVariantMap>
#include <QDateTime>

/**
 * @brief 优化结果结构
 */
struct OptimizationResult {
    bool success = false;               ///< 优化是否成功
    QString optimizerName;              ///< 优化器名称
    QString description;                ///< 优化描述
    QDateTime timestamp;                ///< 优化时间戳
    
    struct Improvements {
        double cpuImprovement = 0.0;    ///< CPU使用率改善 (%)
        double memoryImprovement = 0.0; ///< 内存使用改善 (%)
        double performanceGain = 0.0;   ///< 性能提升 (%)
        double responseTimeGain = 0.0;  ///< 响应时间改善 (%)
        QVariantMap customMetrics;      ///< 自定义改善指标
    } improvements;
    
    struct Details {
        QStringList actionsPerformed;   ///< 执行的优化动作
        QVariantMap beforeMetrics;      ///< 优化前指标
        QVariantMap afterMetrics;       ///< 优化后指标
        QString errorMessage;           ///< 错误信息(如果有)
        int duration = 0;               ///< 优化耗时(毫秒)
    } details;
};

Q_DECLARE_METATYPE(OptimizationResult)

/**
 * @brief 优化器接口
 * 
 * IOptimizer定义了性能优化器的标准接口，所有优化器组件都应该实现此接口。
 * 该接口提供了统一的性能优化操作方法。
 */
class IOptimizer
{
public:
    /**
     * @brief 优化类型枚举
     */
    enum OptimizationType {
        StartupOptimization,    ///< 启动优化
        MemoryOptimization,     ///< 内存优化
        CPUOptimization,        ///< CPU优化
        NetworkOptimization,    ///< 网络优化
        RenderOptimization,     ///< 渲染优化
        StorageOptimization,    ///< 存储优化
        CustomOptimization      ///< 自定义优化
    };

    /**
     * @brief 优化策略枚举
     */
    enum OptimizationStrategy {
        Conservative,           ///< 保守策略
        Balanced,              ///< 平衡策略
        Aggressive             ///< 激进策略
    };

    /**
     * @brief 优化器状态枚举
     */
    enum OptimizerStatus {
        Idle,                  ///< 空闲状态
        Analyzing,             ///< 分析中
        Optimizing,            ///< 优化中
        Completed,             ///< 完成
        Failed,                ///< 失败
        Disabled               ///< 禁用
    };

    /**
     * @brief 虚析构函数
     */
    virtual ~IOptimizer() = default;

    /**
     * @brief 初始化优化器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 执行优化
     * @param strategy 优化策略
     * @return 优化结果
     */
    virtual OptimizationResult optimize(OptimizationStrategy strategy = Balanced) = 0;

    /**
     * @brief 分析是否需要优化
     * @return 是否需要优化
     */
    virtual bool shouldOptimize() const = 0;

    /**
     * @brief 获取优化建议
     * @return 优化建议列表
     */
    virtual QStringList getOptimizationSuggestions() const = 0;

    /**
     * @brief 预估优化效果
     * @param strategy 优化策略
     * @return 预估的改善效果
     */
    virtual QVariantMap estimateImprovements(OptimizationStrategy strategy = Balanced) const = 0;

    /**
     * @brief 获取优化器名称
     * @return 优化器名称
     */
    virtual QString getOptimizerName() const = 0;

    /**
     * @brief 获取优化器版本
     * @return 优化器版本
     */
    virtual QString getVersion() const = 0;

    /**
     * @brief 获取优化类型
     * @return 优化类型
     */
    virtual OptimizationType getOptimizationType() const = 0;

    /**
     * @brief 获取优化器状态
     * @return 优化器状态
     */
    virtual OptimizerStatus getStatus() const = 0;

    /**
     * @brief 获取优化器描述
     * @return 优化器描述
     */
    virtual QString getDescription() const = 0;

    /**
     * @brief 设置优化参数
     * @param parameters 参数映射
     */
    virtual void setOptimizationParameters(const QVariantMap& parameters) = 0;

    /**
     * @brief 获取优化参数
     * @return 参数映射
     */
    virtual QVariantMap getOptimizationParameters() const = 0;

    /**
     * @brief 启用优化器
     */
    virtual void enable() = 0;

    /**
     * @brief 禁用优化器
     */
    virtual void disable() = 0;

    /**
     * @brief 检查优化器是否启用
     * @return 是否启用
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief 获取上次优化结果
     * @return 上次优化结果
     */
    virtual OptimizationResult getLastOptimizationResult() const = 0;

    /**
     * @brief 获取优化历史
     * @param from 开始时间
     * @param to 结束时间
     * @return 优化历史列表
     */
    virtual QList<OptimizationResult> getOptimizationHistory(const QDateTime& from, const QDateTime& to) const = 0;

    /**
     * @brief 重置优化器
     */
    virtual void reset() = 0;

    /**
     * @brief 验证优化器配置
     * @return 配置是否有效
     */
    virtual bool validateConfiguration() const = 0;

    /**
     * @brief 获取优化器统计信息
     * @return 统计信息
     */
    virtual QVariantMap getStatistics() const = 0;

    /**
     * @brief 取消当前优化
     */
    virtual void cancelOptimization() = 0;

    /**
     * @brief 检查是否可以取消
     * @return 是否可以取消
     */
    virtual bool canCancel() const = 0;
};

Q_DECLARE_INTERFACE(IOptimizer, "org.jitsi.performance.IOptimizer/1.0")

#endif // IOPTIMIZER_H