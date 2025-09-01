#ifndef IOPTIMIZER_H
#define IOPTIMIZER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <QStringList>
#include "OptimizationType.h"

/**
 * @brief 优化器状态枚举
 */
enum class OptimizerStatus
{
    Idle,              // 空闲
    Initializing,      // 初始化中
    Analyzing,         // 分析中
    Optimizing,        // 优化中
    Running,           // 运行中
    Completed,         // 已完成
    Failed,            // 失败
    Cancelled,         // 已取消
    Disabled           // 已禁用
};

/**
 * @brief 优化结果结构
 */
struct OptimizationResult
{
    OptimizationResult() = default;
    
    OptimizationResultStatus status = OptimizationResultStatus::Failed;
    
    QString message;                    // 结果消息
    QDateTime timestamp;                // 时间戳
    qint64 executionTime = 0;          // 执行时间(毫秒)
    double improvementPercentage = 0.0; // 改善百分比
    QVariantMap beforeMetrics;          // 优化前指标
    QVariantMap afterMetrics;           // 优化后指标
    QVariantMap improvements;           // 改善详情
    QStringList warnings;               // 警告信息
    QStringList errors;                 // 错误信息
    
    // 便利方法
    bool isSuccess() const { return status == OptimizationResultStatus::Success; }
    bool isFailed() const { return status == OptimizationResultStatus::Failed; }
    bool isPartial() const { return status == OptimizationResultStatus::Partial; }
};

/**
 * @brief 优化器接口
 * 
 * 定义所有性能优化器必须实现的接口
 */
class IOptimizer
{
public:
    virtual ~IOptimizer() = default;

    // 基础接口
    virtual bool initialize() = 0;
    virtual OptimizationResult optimize(OptimizationStrategy strategy = OptimizationStrategy::Balanced) = 0;
    virtual bool shouldOptimize() const = 0;
    virtual QStringList getOptimizationSuggestions() const = 0;
    virtual QVariantMap estimateImprovements(OptimizationStrategy strategy = OptimizationStrategy::Balanced) const = 0;
    
    // 信息接口
    virtual QString getOptimizerName() const = 0;
    virtual QString getVersion() const = 0;
    virtual OptimizationType getOptimizationType() const = 0;
    virtual OptimizerStatus getStatus() const = 0;
    virtual QString getDescription() const = 0;
    
    // 配置接口
    virtual void setOptimizationParameters(const QVariantMap& parameters) = 0;
    virtual QVariantMap getOptimizationParameters() const = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool isEnabled() const = 0;
    
    // 历史和统计接口
    virtual OptimizationResult getLastOptimizationResult() const = 0;
    virtual QList<OptimizationResult> getOptimizationHistory(const QDateTime& from, const QDateTime& to) const = 0;
    virtual void reset() = 0;
    virtual bool validateConfiguration() const = 0;
    virtual QVariantMap getStatistics() const = 0;
    
    // 控制接口
    virtual void cancelOptimization() = 0;
    virtual bool canCancel() const = 0;
};

#endif // IOPTIMIZER_H