#ifndef METRICSCOLLECTOR_H
#define METRICSCOLLECTOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <QDateTime>
#include <QVariantMap>
#include <QSharedPointer>
#include "PerformanceManager.h"

class BaseMonitor;
class PerformanceConfig;

/**
 * @brief 指标收集器类
 * 
 * MetricsCollector负责从各种监控器收集性能指标：
 * - 协调多个监控器的数据收集
 * - 聚合和处理原始性能数据
 * - 维护性能数据的历史记录
 * - 提供数据查询和分析接口
 */
class MetricsCollector : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 数据存储策略枚举
     */
    enum StorageStrategy {
        MemoryOnly,         ///< 仅内存存储
        FileStorage,        ///< 文件存储
        DatabaseStorage,    ///< 数据库存储
        HybridStorage       ///< 混合存储
    };
    Q_ENUM(StorageStrategy)

    /**
     * @brief 聚合类型枚举
     */
    enum AggregationType {
        Average,            ///< 平均值
        Maximum,            ///< 最大值
        Minimum,            ///< 最小值
        Sum,                ///< 总和
        Count               ///< 计数
    };
    Q_ENUM(AggregationType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MetricsCollector(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MetricsCollector();

    /**
     * @brief 初始化指标收集器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 启动指标收集
     * @return 启动是否成功
     */
    bool start();

    /**
     * @brief 停止指标收集
     */
    void stop();

    /**
     * @brief 检查是否正在收集
     * @return 是否正在收集
     */
    bool isCollecting() const;

    /**
     * @brief 设置配置
     * @param config 配置对象
     */
    void setConfig(PerformanceConfig* config);

    /**
     * @brief 获取配置
     * @return 配置对象
     */
    PerformanceConfig* config() const;

    /**
     * @brief 注册监控器
     * @param monitor 监控器对象
     * @return 注册是否成功
     */
    bool registerMonitor(BaseMonitor* monitor);

    /**
     * @brief 注销监控器
     * @param monitorName 监控器名称
     * @return 注销是否成功
     */
    bool unregisterMonitor(const QString& monitorName);

    /**
     * @brief 获取注册的监控器
     * @param monitorName 监控器名称
     * @return 监控器对象
     */
    BaseMonitor* getMonitor(const QString& monitorName) const;

    /**
     * @brief 获取所有注册的监控器
     * @return 监控器列表
     */
    QList<BaseMonitor*> getAllMonitors() const;

    /**
     * @brief 设置收集间隔
     * @param interval 间隔时间(毫秒)
     */
    void setCollectionInterval(int interval);

    /**
     * @brief 获取收集间隔
     * @return 间隔时间(毫秒)
     */
    int collectionInterval() const;

    /**
     * @brief 设置历史数据保留时间
     * @param hours 保留小时数
     */
    void setDataRetentionHours(int hours);

    /**
     * @brief 获取历史数据保留时间
     * @return 保留小时数
     */
    int dataRetentionHours() const;

    /**
     * @brief 设置存储策略
     * @param strategy 存储策略
     */
    void setStorageStrategy(StorageStrategy strategy);

    /**
     * @brief 获取存储策略
     * @return 存储策略
     */
    StorageStrategy storageStrategy() const;

    /**
     * @brief 收集当前指标
     * @return 当前性能指标
     */
    PerformanceMetrics collectCurrentMetrics();

    /**
     * @brief 获取最新指标
     * @return 最新性能指标
     */
    PerformanceMetrics getLatestMetrics() const;

    /**
     * @brief 获取历史指标
     * @param from 开始时间
     * @param to 结束时间
     * @return 历史指标列表
     */
    QList<PerformanceMetrics> getHistoricalMetrics(const QDateTime& from, const QDateTime& to) const;

    /**
     * @brief 获取聚合指标
     * @param from 开始时间
     * @param to 结束时间
     * @param type 聚合类型
     * @return 聚合后的指标
     */
    PerformanceMetrics getAggregatedMetrics(const QDateTime& from, const QDateTime& to, AggregationType type) const;

    /**
     * @brief 获取指标统计信息
     * @param metricName 指标名称
     * @param from 开始时间
     * @param to 结束时间
     * @return 统计信息
     */
    QVariantMap getMetricStatistics(const QString& metricName, const QDateTime& from, const QDateTime& to) const;

    /**
     * @brief 添加自定义指标
     * @param name 指标名称
     * @param value 指标值
     * @param timestamp 时间戳
     */
    void addCustomMetric(const QString& name, const QVariant& value, const QDateTime& timestamp = QDateTime::currentDateTime());

    /**
     * @brief 获取自定义指标
     * @param name 指标名称
     * @param from 开始时间
     * @param to 结束时间
     * @return 自定义指标数据
     */
    QList<QPair<QDateTime, QVariant>> getCustomMetrics(const QString& name, const QDateTime& from, const QDateTime& to) const;

    /**
     * @brief 清除历史数据
     * @param olderThan 清除指定时间之前的数据
     */
    void clearHistoricalData(const QDateTime& olderThan = QDateTime());

    /**
     * @brief 导出数据
     * @param filePath 文件路径
     * @param format 导出格式
     * @param from 开始时间
     * @param to 结束时间
     * @return 导出是否成功
     */
    bool exportData(const QString& filePath, const QString& format, const QDateTime& from, const QDateTime& to) const;

    /**
     * @brief 导入数据
     * @param filePath 文件路径
     * @param format 导入格式
     * @return 导入是否成功
     */
    bool importData(const QString& filePath, const QString& format);

    /**
     * @brief 获取收集器统计信息
     * @return 统计信息
     */
    QVariantMap getCollectorStatistics() const;

    /**
     * @brief 获取数据大小
     * @return 数据大小(字节)
     */
    qint64 getDataSize() const;

    /**
     * @brief 获取数据条目数量
     * @return 条目数量
     */
    int getDataCount() const;

    /**
     * @brief 压缩历史数据
     * @param compressionRatio 压缩比例
     * @return 压缩是否成功
     */
    bool compressHistoricalData(double compressionRatio = 0.5);

signals:
    /**
     * @brief 指标收集完成信号
     * @param metrics 收集到的指标
     */
    void metricsCollected(const PerformanceMetrics& metrics);

    /**
     * @brief 数据存储完成信号
     * @param success 存储是否成功
     */
    void dataStored(bool success);

    /**
     * @brief 存储空间不足信号
     * @param availableSpace 可用空间(字节)
     */
    void storageSpaceLow(qint64 availableSpace);

    /**
     * @brief 数据清理完成信号
     * @param removedCount 清理的数据条目数
     */
    void dataCleanupCompleted(int removedCount);

    /**
     * @brief 收集器错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 执行指标收集
     */
    void performCollection();

    /**
     * @brief 执行数据清理
     */
    void performDataCleanup();

    /**
     * @brief 处理监控器数据
     * @param monitorName 监控器名称
     * @param data 监控数据
     */
    void handleMonitorData(const QString& monitorName, const QVariantMap& data);

private:
    /**
     * @brief 初始化存储
     * @return 初始化是否成功
     */
    bool initializeStorage();

    /**
     * @brief 存储指标数据
     * @param metrics 指标数据
     * @return 存储是否成功
     */
    bool storeMetrics(const PerformanceMetrics& metrics);

    /**
     * @brief 加载历史数据
     * @return 加载是否成功
     */
    bool loadHistoricalData();

    /**
     * @brief 聚合指标数据
     * @param metricsList 指标列表
     * @param type 聚合类型
     * @return 聚合后的指标
     */
    PerformanceMetrics aggregateMetrics(const QList<PerformanceMetrics>& metricsList, AggregationType type) const;

    /**
     * @brief 检查存储空间
     * @return 可用空间(字节)
     */
    qint64 checkStorageSpace() const;

    /**
     * @brief 序列化指标数据
     * @param metrics 指标数据
     * @return 序列化后的数据
     */
    QByteArray serializeMetrics(const PerformanceMetrics& metrics) const;

    /**
     * @brief 反序列化指标数据
     * @param data 序列化数据
     * @return 指标数据
     */
    PerformanceMetrics deserializeMetrics(const QByteArray& data) const;

    PerformanceConfig* m_config;                    ///< 配置对象
    QTimer* m_collectionTimer;                     ///< 收集定时器
    QTimer* m_cleanupTimer;                        ///< 清理定时器
    
    QMap<QString, BaseMonitor*> m_monitors;        ///< 注册的监控器
    QQueue<PerformanceMetrics> m_metricsHistory;   ///< 指标历史记录
    QMap<QString, QList<QPair<QDateTime, QVariant>>> m_customMetrics; ///< 自定义指标
    
    bool m_isCollecting;                           ///< 是否正在收集
    StorageStrategy m_storageStrategy;             ///< 存储策略
    int m_collectionInterval;                      ///< 收集间隔
    int m_dataRetentionHours;                      ///< 数据保留时间
    
    QString m_storageFilePath;                     ///< 存储文件路径
    qint64 m_maxStorageSize;                       ///< 最大存储大小
    
    mutable QMutex m_mutex;                        ///< 线程安全互斥锁
};

#endif // METRICSCOLLECTOR_H