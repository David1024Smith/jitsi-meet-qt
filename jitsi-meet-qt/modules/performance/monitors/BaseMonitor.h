#ifndef BASEMONITOR_H
#define BASEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QVariantMap>
#include <QDateTime>
#include "IResourceTracker.h"

/**
 * @brief 监控器基类
 * 
 * BaseMonitor为所有性能监控器提供通用的基础功能：
 * - 监控生命周期管理
 * - 数据收集和存储
 * - 阈值检查和告警
 * - 线程安全保护
 */
class BaseMonitor : public QObject, public IResourceTracker
{
    Q_OBJECT

public:
    /**
     * @brief 监控器状态枚举
     */
    enum MonitorStatus {
        Stopped,            ///< 已停止
        Starting,           ///< 启动中
        Running,            ///< 运行中
        Paused,             ///< 已暂停
        Error               ///< 错误状态
    };
    Q_ENUM(MonitorStatus)

    /**
     * @brief 构造函数
     * @param monitorName 监控器名称
     * @param parent 父对象
     */
    explicit BaseMonitor(const QString& monitorName, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~BaseMonitor();

    // IResourceTracker接口实现
    bool initialize() override;
    bool startTracking() override;
    void stopTracking() override;
    bool isTracking() const override;
    ResourceUsage getCurrentUsage(ResourceType type = All) const override;
    QList<ResourceUsage> getHistoricalUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const override;
    ResourceUsage getPeakUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const override;
    ResourceUsage getAverageUsage(ResourceType type, const QDateTime& from, const QDateTime& to) const override;
    void setTrackingInterval(int interval) override;
    int trackingInterval() const override;
    void setResourceThreshold(ResourceType type, double threshold) override;
    double resourceThreshold(ResourceType type) const override;
    bool isThresholdExceeded(ResourceType type) const override;
    QVariantMap getSystemInfo() const override;
    QVariantMap getTrackerStatus() const override;
    void reset() override;
    void clearHistoricalData(const QDateTime& olderThan = QDateTime()) override;

    /**
     * @brief 获取监控器名称
     * @return 监控器名称
     */
    QString monitorName() const;

    /**
     * @brief 获取监控器版本
     * @return 监控器版本
     */
    virtual QString version() const;

    /**
     * @brief 获取监控器描述
     * @return 监控器描述
     */
    virtual QString description() const;

    /**
     * @brief 获取监控器状态
     * @return 监控器状态
     */
    MonitorStatus status() const;

    /**
     * @brief 暂停监控
     */
    void pause();

    /**
     * @brief 恢复监控
     */
    void resume();

    /**
     * @brief 检查是否已暂停
     * @return 是否已暂停
     */
    bool isPaused() const;

    /**
     * @brief 设置数据保留时间
     * @param hours 保留小时数
     */
    void setDataRetentionHours(int hours);

    /**
     * @brief 获取数据保留时间
     * @return 保留小时数
     */
    int dataRetentionHours() const;

    /**
     * @brief 获取监控统计信息
     * @return 统计信息
     */
    QVariantMap getMonitorStatistics() const;

    /**
     * @brief 获取错误信息
     * @return 错误信息列表
     */
    QStringList getErrors() const;

    /**
     * @brief 清除错误信息
     */
    void clearErrors();

signals:
    /**
     * @brief 监控器状态改变信号
     * @param status 新状态
     */
    void statusChanged(MonitorStatus status);

    /**
     * @brief 数据更新信号
     * @param usage 资源使用数据
     */
    void dataUpdated(const ResourceUsage& usage);

    /**
     * @brief 阈值超出信号
     * @param type 资源类型
     * @param value 当前值
     * @param threshold 阈值
     */
    void thresholdExceeded(ResourceType type, double value, double threshold);

    /**
     * @brief 监控器错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 数据清理完成信号
     * @param removedCount 清理的数据条目数
     */
    void dataCleanupCompleted(int removedCount);

protected:
    /**
     * @brief 纯虚函数：初始化监控器特定功能
     * @return 初始化是否成功
     */
    virtual bool initializeMonitor() = 0;

    /**
     * @brief 纯虚函数：收集资源使用数据
     * @return 资源使用数据
     */
    virtual ResourceUsage collectResourceUsage() = 0;

    /**
     * @brief 纯虚函数：获取支持的资源类型
     * @return 支持的资源类型列表
     */
    virtual QList<ResourceType> supportedResourceTypes() const = 0;

    /**
     * @brief 设置监控器状态
     * @param status 新状态
     */
    void setStatus(MonitorStatus status);

    /**
     * @brief 添加错误信息
     * @param error 错误信息
     */
    void addError(const QString& error);

    /**
     * @brief 存储资源使用数据
     * @param usage 资源使用数据
     */
    void storeUsageData(const ResourceUsage& usage);

    /**
     * @brief 检查阈值
     * @param usage 资源使用数据
     */
    void checkThresholds(const ResourceUsage& usage);

    /**
     * @brief 获取资源值
     * @param usage 资源使用数据
     * @param type 资源类型
     * @return 资源值
     */
    double getResourceValue(const ResourceUsage& usage, ResourceType type) const;

private slots:
    /**
     * @brief 执行数据收集
     */
    void performDataCollection();

    /**
     * @brief 执行数据清理
     */
    void performDataCleanup();

private:
    /**
     * @brief 计算峰值使用情况
     * @param usageList 使用情况列表
     * @param type 资源类型
     * @return 峰值使用情况
     */
    ResourceUsage calculatePeakUsage(const QList<ResourceUsage>& usageList, ResourceType type) const;

    /**
     * @brief 计算平均使用情况
     * @param usageList 使用情况列表
     * @param type 资源类型
     * @return 平均使用情况
     */
    ResourceUsage calculateAverageUsage(const QList<ResourceUsage>& usageList, ResourceType type) const;

    /**
     * @brief 过滤历史数据
     * @param type 资源类型
     * @param from 开始时间
     * @param to 结束时间
     * @return 过滤后的数据列表
     */
    QList<ResourceUsage> filterHistoricalData(ResourceType type, const QDateTime& from, const QDateTime& to) const;

    QString m_monitorName;                          ///< 监控器名称
    MonitorStatus m_status;                         ///< 监控器状态
    QTimer* m_collectionTimer;                      ///< 数据收集定时器
    QTimer* m_cleanupTimer;                         ///< 数据清理定时器
    
    int m_trackingInterval;                         ///< 跟踪间隔
    int m_dataRetentionHours;                       ///< 数据保留时间
    
    QMap<ResourceType, double> m_thresholds;        ///< 资源阈值映射
    QList<ResourceUsage> m_historicalData;          ///< 历史数据
    QStringList m_errors;                           ///< 错误信息列表
    
    mutable QMutex m_mutex;                         ///< 线程安全互斥锁
    
    // 统计信息
    qint64 m_totalCollections;                      ///< 总收集次数
    qint64 m_successfulCollections;                 ///< 成功收集次数
    qint64 m_failedCollections;                     ///< 失败收集次数
    QDateTime m_startTime;                          ///< 启动时间
    QDateTime m_lastCollectionTime;                 ///< 最后收集时间
};

#endif // BASEMONITOR_H