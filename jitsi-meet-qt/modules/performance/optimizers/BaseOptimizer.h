#ifndef BASEOPTIMIZER_H
#define BASEOPTIMIZER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QVariantMap>
#include <QDateTime>
#include "IOptimizer.h"

/**
 * @brief 优化器基类
 * 
 * BaseOptimizer为所有性能优化器提供通用的基础功能：
 * - 优化生命周期管理
 * - 优化策略执行
 * - 优化结果跟踪
 * - 线程安全保护
 */
class BaseOptimizer : public QObject, public IOptimizer
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param optimizerName 优化器名称
     * @param parent 父对象
     */
    explicit BaseOptimizer(const QString& optimizerName, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~BaseOptimizer();

    // IOptimizer接口实现
    bool initialize() override;
    OptimizationResult optimize(OptimizationStrategy strategy = OptimizationStrategy::Balanced) override;
    bool shouldOptimize() const override;
    QStringList getOptimizationSuggestions() const override;
    QVariantMap estimateImprovements(OptimizationStrategy strategy = OptimizationStrategy::Balanced) const override;
    QString getOptimizerName() const override;
    QString getVersion() const override;
    OptimizationType getOptimizationType() const override;
    OptimizerStatus getStatus() const override;
    QString getDescription() const override;
    void setOptimizationParameters(const QVariantMap& parameters) override;
    QVariantMap getOptimizationParameters() const override;
    void enable() override;
    void disable() override;
    bool isEnabled() const override;
    OptimizationResult getLastOptimizationResult() const override;
    QList<OptimizationResult> getOptimizationHistory(const QDateTime& from, const QDateTime& to) const override;
    void reset() override;
    bool validateConfiguration() const override;
    QVariantMap getStatistics() const override;
    void cancelOptimization() override;
    bool canCancel() const override;

    /**
     * @brief 设置优化器状态
     * @param status 新状态
     */
    void setStatus(OptimizerStatus status);

    /**
     * @brief 设置优化间隔
     * @param interval 间隔时间(毫秒)
     */
    void setOptimizationInterval(int interval);

    /**
     * @brief 获取优化间隔
     * @return 间隔时间(毫秒)
     */
    int optimizationInterval() const;

    /**
     * @brief 设置自动优化是否启用
     * @param enabled 是否启用
     */
    void setAutoOptimizationEnabled(bool enabled);

    /**
     * @brief 获取自动优化是否启用
     * @return 是否启用
     */
    bool isAutoOptimizationEnabled() const;

    /**
     * @brief 获取优化次数
     * @return 优化次数
     */
    int getOptimizationCount() const;

    /**
     * @brief 获取成功优化次数
     * @return 成功优化次数
     */
    int getSuccessfulOptimizationCount() const;

    /**
     * @brief 获取失败优化次数
     * @return 失败优化次数
     */
    int getFailedOptimizationCount() const;

    /**
     * @brief 获取平均优化时间
     * @return 平均优化时间(毫秒)
     */
    double getAverageOptimizationTime() const;

    /**
     * @brief 获取总优化改善
     * @return 总优化改善
     */
    QVariantMap getTotalImprovements() const;

signals:
    /**
     * @brief 优化器状态改变信号
     * @param status 新状态
     */
    void statusChanged(OptimizerStatus status);

    /**
     * @brief 优化开始信号
     * @param strategy 优化策略
     */
    void optimizationStarted(OptimizationStrategy strategy);

    /**
     * @brief 优化完成信号
     * @param result 优化结果
     */
    void optimizationCompleted(const OptimizationResult& result);

    /**
     * @brief 优化进度信号
     * @param progress 进度(0-100)
     * @param description 当前操作描述
     */
    void optimizationProgress(int progress, const QString& description);

    /**
     * @brief 优化取消信号
     */
    void optimizationCancelled();

    /**
     * @brief 优化器错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    /**
     * @brief 纯虚函数：初始化优化器特定功能
     * @return 初始化是否成功
     */
    virtual bool initializeOptimizer() = 0;

    /**
     * @brief 纯虚函数：执行具体优化操作
     * @param strategy 优化策略
     * @return 优化结果
     */
    virtual OptimizationResult performOptimization(OptimizationStrategy strategy) = 0;

    /**
     * @brief 纯虚函数：分析是否需要优化
     * @return 是否需要优化
     */
    virtual bool analyzeOptimizationNeed() const = 0;

    /**
     * @brief 纯虚函数：生成优化建议
     * @return 优化建议列表
     */
    virtual QStringList generateSuggestions() const = 0;

    /**
     * @brief 纯虚函数：预估优化效果
     * @param strategy 优化策略
     * @return 预估改善效果
     */
    virtual QVariantMap estimateOptimizationImprovements(OptimizationStrategy strategy) const = 0;

    /**
     * @brief 纯虚函数：获取优化器版本
     * @return 版本字符串
     */
    virtual QString getOptimizerVersion() const = 0;

    /**
     * @brief 纯虚函数：获取优化器描述
     * @return 描述字符串
     */
    virtual QString getOptimizerDescription() const = 0;

    /**
     * @brief 纯虚函数：获取优化类型
     * @return 优化类型
     */
    virtual OptimizationType getOptimizerType() const = 0;

    /**
     * @brief 添加错误信息
     * @param error 错误信息
     */
    void addError(const QString& error);

    /**
     * @brief 更新优化进度
     * @param progress 进度(0-100)
     * @param description 当前操作描述
     */
    void updateProgress(int progress, const QString& description);

    /**
     * @brief 记录优化结果
     * @param result 优化结果
     */
    void recordOptimizationResult(const OptimizationResult& result);

    /**
     * @brief 获取优化前指标
     * @return 优化前指标
     */
    QVariantMap getBeforeMetrics() const;

    /**
     * @brief 获取优化后指标
     * @return 优化后指标
     */
    QVariantMap getAfterMetrics() const;

    /**
     * @brief 计算改善效果
     * @param beforeMetrics 优化前指标
     * @param afterMetrics 优化后指标
     * @return 改善效果
     */
    QVariantMap calculateImprovements(const QVariantMap& beforeMetrics, const QVariantMap& afterMetrics) const;

    /**
     * @brief 验证优化参数
     * @param parameters 优化参数
     * @return 参数是否有效
     */
    virtual bool validateOptimizationParameters(const QVariantMap& parameters) const;

    /**
     * @brief 获取默认优化参数
     * @return 默认参数
     */
    virtual QVariantMap getDefaultParameters() const;

private slots:
    /**
     * @brief 执行自动优化
     */
    void performAutoOptimization();

private:
    /**
     * @brief 清理历史数据
     */
    void cleanupHistory();

    /**
     * @brief 更新统计信息
     * @param result 优化结果
     */
    void updateStatistics(const OptimizationResult& result);

    QString m_optimizerName;                        ///< 优化器名称
    OptimizerStatus m_status;                       ///< 优化器状态
    bool m_enabled;                                 ///< 是否启用
    bool m_autoOptimizationEnabled;                 ///< 是否启用自动优化
    int m_optimizationInterval;                     ///< 优化间隔
    bool m_cancellationRequested;                   ///< 是否请求取消
    
    QVariantMap m_optimizationParameters;           ///< 优化参数
    OptimizationResult m_lastResult;                ///< 上次优化结果
    QList<OptimizationResult> m_optimizationHistory; ///< 优化历史
    QStringList m_errors;                           ///< 错误信息列表
    
    QTimer* m_autoOptimizationTimer;                ///< 自动优化定时器
    QTimer* m_historyCleanupTimer;                  ///< 历史清理定时器
    
    // 统计信息
    int m_optimizationCount;                        ///< 优化次数
    int m_successfulOptimizations;                  ///< 成功优化次数
    int m_failedOptimizations;                      ///< 失败优化次数
    qint64 m_totalOptimizationTime;                 ///< 总优化时间
    QVariantMap m_totalImprovements;                ///< 总改善效果
    QDateTime m_firstOptimizationTime;              ///< 首次优化时间
    QDateTime m_lastOptimizationTime;               ///< 最后优化时间
    
    mutable QMutex m_mutex;                         ///< 线程安全互斥锁
    
    // 常量
    static const int DEFAULT_OPTIMIZATION_INTERVAL = 300000; ///< 默认优化间隔(5分钟)
    static const int MAX_HISTORY_SIZE = 1000;       ///< 最大历史记录数
    static const int HISTORY_CLEANUP_INTERVAL = 3600000; ///< 历史清理间隔(1小时)
};

#endif // BASEOPTIMIZER_H