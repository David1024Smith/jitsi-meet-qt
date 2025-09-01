#ifndef PERFORMANCECONFIG_H
#define PERFORMANCECONFIG_H

#include <QObject>
#include <QVariantMap>
#include <QSettings>
#include <QMutex>
#include <QJsonObject>

/**
 * @brief 性能配置类
 * 
 * PerformanceConfig管理性能模块的所有配置参数，包括：
 * - 监控配置参数
 * - 优化策略配置
 * - 阈值和告警配置
 * - 存储和报告配置
 */
class PerformanceConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 配置类别枚举
     */
    enum ConfigCategory {
        MonitoringConfig,       ///< 监控配置
        OptimizationConfig,     ///< 优化配置
        ThresholdConfig,        ///< 阈值配置
        StorageConfig,          ///< 存储配置
        ReportingConfig,        ///< 报告配置
        UIConfig               ///< 界面配置
    };
    Q_ENUM(ConfigCategory)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PerformanceConfig(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PerformanceConfig();

    /**
     * @brief 加载配置
     * @param filePath 配置文件路径
     * @return 加载是否成功
     */
    bool loadConfig(const QString& filePath = QString());

    /**
     * @brief 保存配置
     * @param filePath 配置文件路径
     * @return 保存是否成功
     */
    bool saveConfig(const QString& filePath = QString());

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 验证配置
     * @return 配置是否有效
     */
    bool validateConfig();

    // 监控配置
    /**
     * @brief 设置监控是否启用
     * @param enabled 是否启用
     */
    void setMonitoringEnabled(bool enabled);

    /**
     * @brief 获取监控是否启用
     * @return 是否启用
     */
    bool isMonitoringEnabled() const;

    /**
     * @brief 设置监控间隔
     * @param interval 间隔时间(毫秒)
     */
    void setMonitoringInterval(int interval);

    /**
     * @brief 获取监控间隔
     * @return 间隔时间(毫秒)
     */
    int monitoringInterval() const;

    /**
     * @brief 设置启用的监控器
     * @param monitors 监控器名称列表
     */
    void setEnabledMonitors(const QStringList& monitors);

    /**
     * @brief 获取启用的监控器
     * @return 监控器名称列表
     */
    QStringList enabledMonitors() const;

    // 优化配置
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
     * @brief 设置启用的优化器
     * @param optimizers 优化器名称列表
     */
    void setEnabledOptimizers(const QStringList& optimizers);

    /**
     * @brief 获取启用的优化器
     * @return 优化器名称列表
     */
    QStringList enabledOptimizers() const;

    // 阈值配置
    /**
     * @brief 设置CPU使用率阈值
     * @param threshold 阈值(%)
     */
    void setCpuThreshold(double threshold);

    /**
     * @brief 获取CPU使用率阈值
     * @return 阈值(%)
     */
    double cpuThreshold() const;

    /**
     * @brief 设置内存使用阈值
     * @param threshold 阈值(MB)
     */
    void setMemoryThreshold(qint64 threshold);

    /**
     * @brief 获取内存使用阈值
     * @return 阈值(MB)
     */
    qint64 memoryThreshold() const;

    /**
     * @brief 设置网络延迟阈值
     * @param threshold 阈值(ms)
     */
    void setNetworkLatencyThreshold(double threshold);

    /**
     * @brief 获取网络延迟阈值
     * @return 阈值(ms)
     */
    double networkLatencyThreshold() const;

    /**
     * @brief 设置帧率阈值
     * @param threshold 阈值(fps)
     */
    void setFrameRateThreshold(double threshold);

    /**
     * @brief 获取帧率阈值
     * @return 阈值(fps)
     */
    double frameRateThreshold() const;

    // 存储配置
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
     * @brief 设置最大存储大小
     * @param size 最大大小(MB)
     */
    void setMaxStorageSize(qint64 size);

    /**
     * @brief 获取最大存储大小
     * @return 最大大小(MB)
     */
    qint64 maxStorageSize() const;

    /**
     * @brief 设置存储路径
     * @param path 存储路径
     */
    void setStoragePath(const QString& path);

    /**
     * @brief 获取存储路径
     * @return 存储路径
     */
    QString storagePath() const;

    // 报告配置
    /**
     * @brief 设置报告生成是否启用
     * @param enabled 是否启用
     */
    void setReportingEnabled(bool enabled);

    /**
     * @brief 获取报告生成是否启用
     * @return 是否启用
     */
    bool isReportingEnabled() const;

    /**
     * @brief 设置报告生成间隔
     * @param interval 间隔时间(小时)
     */
    void setReportingInterval(int interval);

    /**
     * @brief 获取报告生成间隔
     * @return 间隔时间(小时)
     */
    int reportingInterval() const;

    /**
     * @brief 设置报告格式
     * @param format 报告格式
     */
    void setReportFormat(const QString& format);

    /**
     * @brief 获取报告格式
     * @return 报告格式
     */
    QString reportFormat() const;

    // 界面配置
    /**
     * @brief 设置实时显示是否启用
     * @param enabled 是否启用
     */
    void setRealTimeDisplayEnabled(bool enabled);

    /**
     * @brief 获取实时显示是否启用
     * @return 是否启用
     */
    bool isRealTimeDisplayEnabled() const;

    /**
     * @brief 设置图表更新间隔
     * @param interval 间隔时间(毫秒)
     */
    void setChartUpdateInterval(int interval);

    /**
     * @brief 获取图表更新间隔
     * @return 间隔时间(毫秒)
     */
    int chartUpdateInterval() const;

    /**
     * @brief 设置显示的指标
     * @param metrics 指标名称列表
     */
    void setDisplayedMetrics(const QStringList& metrics);

    /**
     * @brief 获取显示的指标
     * @return 指标名称列表
     */
    QStringList displayedMetrics() const;

    // 通用配置方法
    /**
     * @brief 设置配置值
     * @param category 配置类别
     * @param key 配置键
     * @param value 配置值
     */
    void setValue(ConfigCategory category, const QString& key, const QVariant& value);

    /**
     * @brief 获取配置值
     * @param category 配置类别
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant value(ConfigCategory category, const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 获取类别的所有配置
     * @param category 配置类别
     * @return 配置映射
     */
    QVariantMap getCategoryConfig(ConfigCategory category) const;

    /**
     * @brief 设置类别的所有配置
     * @param category 配置类别
     * @param config 配置映射
     */
    void setCategoryConfig(ConfigCategory category, const QVariantMap& config);

    /**
     * @brief 获取所有配置
     * @return 完整配置映射
     */
    QVariantMap getAllConfig() const;

    /**
     * @brief 设置所有配置
     * @param config 完整配置映射
     */
    void setAllConfig(const QVariantMap& config);

    /**
     * @brief 获取配置文件路径
     * @return 配置文件路径
     */
    QString configFilePath() const;

    /**
     * @brief 设置配置文件路径
     * @param filePath 配置文件路径
     */
    void setConfigFilePath(const QString& filePath);

    /**
     * @brief 导出配置到JSON
     * @return JSON字符串
     */
    QString exportToJson() const;

    /**
     * @brief 从JSON导入配置
     * @param json JSON字符串
     * @return 导入是否成功
     */
    bool importFromJson(const QString& json);

signals:
    /**
     * @brief 配置改变信号
     * @param category 改变的配置类别
     * @param key 改变的配置键
     * @param value 新的配置值
     */
    void configChanged(ConfigCategory category, const QString& key, const QVariant& value);

    /**
     * @brief 配置加载完成信号
     * @param success 加载是否成功
     */
    void configLoaded(bool success);

    /**
     * @brief 配置保存完成信号
     * @param success 保存是否成功
     */
    void configSaved(bool success);

    /**
     * @brief 配置验证完成信号
     * @param valid 配置是否有效
     * @param errors 验证错误列表
     */
    void configValidated(bool valid, const QStringList& errors);

private:
    /**
     * @brief 初始化默认配置
     */
    void initializeDefaults();

    /**
     * @brief 验证配置值
     * @param category 配置类别
     * @param key 配置键
     * @param value 配置值
     * @return 是否有效
     */
    bool validateValue(ConfigCategory category, const QString& key, const QVariant& value) const;

    /**
     * @brief 获取类别名称
     * @param category 配置类别
     * @return 类别名称
     */
    QString getCategoryName(ConfigCategory category) const;

    /**
     * @brief 从JSON对象加载配置
     * @param obj JSON对象
     * @param prefix 键前缀
     */
    void loadJsonObject(const QJsonObject& obj, const QString& prefix);

    /**
     * @brief 保存配置到JSON对象
     * @param obj JSON对象
     * @param config 配置映射
     */
    void saveToJsonObject(QJsonObject& obj, const QVariantMap& config) const;
    void setNestedValue(QJsonObject& obj, const QStringList& keyParts, const QJsonValue& value) const;

    QVariantMap m_config;                   ///< 配置数据
    QString m_configFilePath;               ///< 配置文件路径
    mutable QMutex m_mutex;                 ///< 线程安全互斥锁
    
    // 默认配置值
    static const QVariantMap s_defaultConfig;
};

#endif // PERFORMANCECONFIG_H