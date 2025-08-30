#ifndef STARTUPOPTIMIZER_H
#define STARTUPOPTIMIZER_H

#include "BaseOptimizer.h"
#include <QSettings>
#include <QDir>
#include <QFileInfo>

/**
 * @brief 启动优化器类
 * 
 * StartupOptimizer专门负责优化应用程序启动性能：
 * - 预加载关键模块
 * - 优化启动顺序
 * - 缓存启动数据
 * - 延迟加载非关键组件
 */
class StartupOptimizer : public BaseOptimizer
{
    Q_OBJECT

public:
    /**
     * @brief 启动优化策略枚举
     */
    enum StartupStrategy {
        FastStart,          ///< 快速启动 - 最小化启动时间
        BalancedStart,      ///< 平衡启动 - 平衡启动时间和功能
        FullStart          ///< 完整启动 - 预加载所有功能
    };
    Q_ENUM(StartupStrategy)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit StartupOptimizer(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~StartupOptimizer();

    /**
     * @brief 设置启动策略
     * @param strategy 启动策略
     */
    void setStartupStrategy(StartupStrategy strategy);

    /**
     * @brief 获取启动策略
     * @return 启动策略
     */
    StartupStrategy startupStrategy() const;

    /**
     * @brief 预加载模块
     * @param moduleNames 模块名称列表
     * @return 预加载是否成功
     */
    bool preloadModules(const QStringList& moduleNames);

    /**
     * @brief 设置延迟加载模块
     * @param moduleNames 模块名称列表
     */
    void setDeferredModules(const QStringList& moduleNames);

    /**
     * @brief 获取延迟加载模块列表
     * @return 模块名称列表
     */
    QStringList deferredModules() const;

    /**
     * @brief 优化启动缓存
     * @return 优化是否成功
     */
    bool optimizeStartupCache();

    /**
     * @brief 清理启动缓存
     * @return 清理是否成功
     */
    bool clearStartupCache();

    /**
     * @brief 获取启动时间统计
     * @return 启动时间统计信息
     */
    QVariantMap getStartupTimeStats() const;

    /**
     * @brief 设置启动超时时间
     * @param timeout 超时时间(毫秒)
     */
    void setStartupTimeout(int timeout);

    /**
     * @brief 获取启动超时时间
     * @return 超时时间(毫秒)
     */
    int startupTimeout() const;

protected:
    /**
     * @brief 初始化启动优化器
     * @return 初始化是否成功
     */
    bool initializeOptimizer() override;

    /**
     * @brief 执行启动优化
     * @param strategy 优化策略
     * @return 优化结果
     */
    OptimizationResult performOptimization(OptimizationStrategy strategy) override;

    /**
     * @brief 分析是否需要启动优化
     * @return 是否需要优化
     */
    bool analyzeOptimizationNeed() const override;

    /**
     * @brief 生成启动优化建议
     * @return 优化建议列表
     */
    QStringList generateSuggestions() const override;

    /**
     * @brief 预估启动优化效果
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

    /**
     * @brief 获取优化前指标
     * @return 优化前指标
     */
    QVariantMap getBeforeMetrics() const override;

    /**
     * @brief 获取优化后指标
     * @return 优化后指标
     */
    QVariantMap getAfterMetrics() const override;

private:
    /**
     * @brief 执行快速启动优化
     * @return 优化结果
     */
    OptimizationResult performFastStartOptimization();

    /**
     * @brief 执行平衡启动优化
     * @return 优化结果
     */
    OptimizationResult performBalancedStartOptimization();

    /**
     * @brief 执行完整启动优化
     * @return 优化结果
     */
    OptimizationResult performFullStartOptimization();

    /**
     * @brief 优化模块加载顺序
     * @return 优化是否成功
     */
    bool optimizeModuleLoadOrder();

    /**
     * @brief 创建启动缓存
     * @return 创建是否成功
     */
    bool createStartupCache();

    /**
     * @brief 加载启动缓存
     * @return 加载是否成功
     */
    bool loadStartupCache();

    /**
     * @brief 验证启动缓存
     * @return 缓存是否有效
     */
    bool validateStartupCache();

    /**
     * @brief 优化启动配置
     * @return 优化是否成功
     */
    bool optimizeStartupConfiguration();

    /**
     * @brief 测量启动时间
     * @return 启动时间(毫秒)
     */
    qint64 measureStartupTime();

    /**
     * @brief 分析启动瓶颈
     * @return 瓶颈分析结果
     */
    QVariantMap analyzeStartupBottlenecks();

    /**
     * @brief 获取关键模块列表
     * @return 关键模块列表
     */
    QStringList getCriticalModules() const;

    /**
     * @brief 获取可选模块列表
     * @return 可选模块列表
     */
    QStringList getOptionalModules() const;

    StartupStrategy m_startupStrategy;              ///< 启动策略
    QStringList m_deferredModules;                  ///< 延迟加载模块
    QStringList m_preloadedModules;                 ///< 预加载模块
    int m_startupTimeout;                           ///< 启动超时时间
    
    QSettings* m_startupSettings;                   ///< 启动设置
    QString m_cacheDirectory;                       ///< 缓存目录
    
    // 启动时间统计
    qint64 m_lastStartupTime;                       ///< 上次启动时间
    qint64 m_averageStartupTime;                    ///< 平均启动时间
    qint64 m_bestStartupTime;                       ///< 最佳启动时间
    QList<qint64> m_startupTimeHistory;             ///< 启动时间历史
    
    // 优化状态
    bool m_cacheOptimized;                          ///< 缓存是否已优化
    bool m_configOptimized;                         ///< 配置是否已优化
    bool m_moduleOrderOptimized;                    ///< 模块顺序是否已优化
    
    mutable QMutex m_startupMutex;                  ///< 启动互斥锁
};

#endif // STARTUPOPTIMIZER_H