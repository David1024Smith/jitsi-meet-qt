#ifndef PERFORMANCEUTILS_H
#define PERFORMANCEUTILS_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include "PerformanceManager.h"

/**
 * @brief 性能工具类
 * 
 * PerformanceUtils提供性能模块的通用工具函数：
 * - 数据格式化和转换
 * - 性能计算和分析
 * - 文件操作和导入导出
 * - 系统信息获取
 */
class PerformanceUtils : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 数据格式枚举
     */
    enum DataFormat {
        JSON,               ///< JSON格式
        XML,                ///< XML格式
        CSV,                ///< CSV格式
        Binary              ///< 二进制格式
    };
    Q_ENUM(DataFormat)

    /**
     * @brief 时间单位枚举
     */
    enum TimeUnit {
        Milliseconds,       ///< 毫秒
        Seconds,            ///< 秒
        Minutes,            ///< 分钟
        Hours,              ///< 小时
        Days                ///< 天
    };
    Q_ENUM(TimeUnit)

    /**
     * @brief 大小单位枚举
     */
    enum SizeUnit {
        Bytes,              ///< 字节
        Kilobytes,          ///< 千字节
        Megabytes,          ///< 兆字节
        Gigabytes,          ///< 吉字节
        Terabytes           ///< 太字节
    };
    Q_ENUM(SizeUnit)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PerformanceUtils(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PerformanceUtils();

    // 数据格式化函数
    /**
     * @brief 格式化字节大小
     * @param bytes 字节数
     * @param unit 目标单位
     * @param precision 精度
     * @return 格式化字符串
     */
    static QString formatBytes(qint64 bytes, SizeUnit unit = Bytes, int precision = 2);

    /**
     * @brief 格式化时间
     * @param milliseconds 毫秒数
     * @param unit 目标单位
     * @param precision 精度
     * @return 格式化字符串
     */
    static QString formatTime(qint64 milliseconds, TimeUnit unit = Milliseconds, int precision = 2);

    /**
     * @brief 格式化百分比
     * @param percentage 百分比
     * @param precision 精度
     * @return 格式化字符串
     */
    static QString formatPercentage(double percentage, int precision = 1);

    /**
     * @brief 格式化频率
     * @param frequency 频率(Hz)
     * @param precision 精度
     * @return 格式化字符串
     */
    static QString formatFrequency(double frequency, int precision = 2);

    /**
     * @brief 格式化带宽
     * @param bandwidth 带宽(bps)
     * @param precision 精度
     * @return 格式化字符串
     */
    static QString formatBandwidth(qint64 bandwidth, int precision = 2);

    // 数据转换函数
    /**
     * @brief 字节转换
     * @param bytes 字节数
     * @param fromUnit 源单位
     * @param toUnit 目标单位
     * @return 转换后的值
     */
    static double convertBytes(qint64 bytes, SizeUnit fromUnit, SizeUnit toUnit);

    /**
     * @brief 时间转换
     * @param time 时间值
     * @param fromUnit 源单位
     * @param toUnit 目标单位
     * @return 转换后的值
     */
    static double convertTime(qint64 time, TimeUnit fromUnit, TimeUnit toUnit);

    /**
     * @brief 性能指标转JSON
     * @param metrics 性能指标
     * @return JSON对象
     */
    static QJsonObject metricsToJson(const PerformanceMetrics& metrics);

    /**
     * @brief JSON转性能指标
     * @param json JSON对象
     * @return 性能指标
     */
    static PerformanceMetrics metricsFromJson(const QJsonObject& json);

    // 性能计算函数
    /**
     * @brief 计算平均值
     * @param values 数值列表
     * @return 平均值
     */
    static double calculateAverage(const QList<double>& values);

    /**
     * @brief 计算中位数
     * @param values 数值列表
     * @return 中位数
     */
    static double calculateMedian(QList<double> values);

    /**
     * @brief 计算标准差
     * @param values 数值列表
     * @return 标准差
     */
    static double calculateStandardDeviation(const QList<double>& values);

    /**
     * @brief 计算百分位数
     * @param values 数值列表
     * @param percentile 百分位(0-100)
     * @return 百分位数值
     */
    static double calculatePercentile(QList<double> values, double percentile);

    /**
     * @brief 计算变化率
     * @param oldValue 旧值
     * @param newValue 新值
     * @return 变化率(%)
     */
    static double calculateChangeRate(double oldValue, double newValue);

    /**
     * @brief 计算趋势
     * @param values 数值列表
     * @return 趋势(-1: 下降, 0: 平稳, 1: 上升)
     */
    static int calculateTrend(const QList<double>& values);

    // 系统信息函数
    /**
     * @brief 获取系统信息
     * @return 系统信息
     */
    static QVariantMap getSystemInfo();

    /**
     * @brief 获取CPU信息
     * @return CPU信息
     */
    static QVariantMap getCPUInfo();

    /**
     * @brief 获取内存信息
     * @return 内存信息
     */
    static QVariantMap getMemoryInfo();

    /**
     * @brief 获取磁盘信息
     * @return 磁盘信息
     */
    static QVariantMap getDiskInfo();

    /**
     * @brief 获取网络信息
     * @return 网络信息
     */
    static QVariantMap getNetworkInfo();

    /**
     * @brief 获取进程信息
     * @param processId 进程ID
     * @return 进程信息
     */
    static QVariantMap getProcessInfo(qint64 processId = 0);

    // 文件操作函数
    /**
     * @brief 导出性能数据
     * @param metrics 性能指标列表
     * @param filePath 文件路径
     * @param format 数据格式
     * @return 导出是否成功
     */
    static bool exportPerformanceData(const QList<PerformanceMetrics>& metrics, const QString& filePath, DataFormat format = JSON);

    /**
     * @brief 导入性能数据
     * @param filePath 文件路径
     * @param format 数据格式
     * @return 性能指标列表
     */
    static QList<PerformanceMetrics> importPerformanceData(const QString& filePath, DataFormat format = JSON);

    /**
     * @brief 生成性能报告
     * @param metrics 性能指标列表
     * @param filePath 报告文件路径
     * @param format 报告格式
     * @return 生成是否成功
     */
    static bool generatePerformanceReport(const QList<PerformanceMetrics>& metrics, const QString& filePath, const QString& format = "html");

    // 配置函数
    /**
     * @brief 加载配置文件
     * @param filePath 配置文件路径
     * @return 配置数据
     */
    static QVariantMap loadConfiguration(const QString& filePath);

    /**
     * @brief 保存配置文件
     * @param config 配置数据
     * @param filePath 配置文件路径
     * @return 保存是否成功
     */
    static bool saveConfiguration(const QVariantMap& config, const QString& filePath);

    /**
     * @brief 验证配置
     * @param config 配置数据
     * @return 验证结果(是否有效, 错误列表)
     */
    static QPair<bool, QStringList> validateConfiguration(const QVariantMap& config);

    // 诊断函数
    /**
     * @brief 诊断系统性能
     * @return 诊断结果
     */
    static QVariantMap diagnoseSystemPerformance();

    /**
     * @brief 检查性能瓶颈
     * @param metrics 性能指标
     * @return 瓶颈列表
     */
    static QStringList checkPerformanceBottlenecks(const PerformanceMetrics& metrics);

    /**
     * @brief 生成优化建议
     * @param metrics 性能指标
     * @return 优化建议列表
     */
    static QStringList generateOptimizationSuggestions(const PerformanceMetrics& metrics);

    // 工具函数
    /**
     * @brief 获取时间戳字符串
     * @param timestamp 时间戳
     * @param format 格式字符串
     * @return 时间戳字符串
     */
    static QString getTimestampString(const QDateTime& timestamp = QDateTime::currentDateTime(), const QString& format = "yyyy-MM-dd hh:mm:ss");

    /**
     * @brief 生成唯一ID
     * @return 唯一ID字符串
     */
    static QString generateUniqueId();

    /**
     * @brief 计算文件哈希
     * @param filePath 文件路径
     * @param algorithm 哈希算法
     * @return 哈希值
     */
    static QString calculateFileHash(const QString& filePath, const QString& algorithm = "MD5");

    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @return 压缩后的数据
     */
    static QByteArray compressData(const QByteArray& data);

    /**
     * @brief 解压数据
     * @param compressedData 压缩数据
     * @return 解压后的数据
     */
    static QByteArray decompressData(const QByteArray& compressedData);

    /**
     * @brief 获取可用磁盘空间
     * @param path 路径
     * @return 可用空间(字节)
     */
    static qint64 getAvailableDiskSpace(const QString& path);

    /**
     * @brief 检查文件是否存在
     * @param filePath 文件路径
     * @return 是否存在
     */
    static bool fileExists(const QString& filePath);

    /**
     * @brief 创建目录
     * @param dirPath 目录路径
     * @return 创建是否成功
     */
    static bool createDirectory(const QString& dirPath);

    /**
     * @brief 删除文件
     * @param filePath 文件路径
     * @return 删除是否成功
     */
    static bool removeFile(const QString& filePath);

private:
    /**
     * @brief 获取单位后缀
     * @param unit 单位
     * @return 单位后缀
     */
    static QString getSizeUnitSuffix(SizeUnit unit);

    /**
     * @brief 获取时间单位后缀
     * @param unit 时间单位
     * @return 单位后缀
     */
    static QString getTimeUnitSuffix(TimeUnit unit);

    /**
     * @brief 获取单位倍数
     * @param unit 单位
     * @return 倍数
     */
    static qint64 getSizeUnitMultiplier(SizeUnit unit);

    /**
     * @brief 获取时间单位倍数
     * @param unit 时间单位
     * @return 倍数
     */
    static qint64 getTimeUnitMultiplier(TimeUnit unit);

    /**
     * @brief 导出为JSON格式
     * @param metrics 性能指标列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    static bool exportToJson(const QList<PerformanceMetrics>& metrics, const QString& filePath);

    /**
     * @brief 导出为CSV格式
     * @param metrics 性能指标列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    static bool exportToCsv(const QList<PerformanceMetrics>& metrics, const QString& filePath);

    /**
     * @brief 从JSON格式导入
     * @param filePath 文件路径
     * @return 性能指标列表
     */
    static QList<PerformanceMetrics> importFromJson(const QString& filePath);

    /**
     * @brief 从CSV格式导入
     * @param filePath 文件路径
     * @return 性能指标列表
     */
    static QList<PerformanceMetrics> importFromCsv(const QString& filePath);

    /**
     * @brief 生成HTML报告
     * @param metrics 性能指标列表
     * @param filePath 报告文件路径
     * @return 生成是否成功
     */
    static bool generateHtmlReport(const QList<PerformanceMetrics>& metrics, const QString& filePath);

    /**
     * @brief 生成PDF报告
     * @param metrics 性能指标列表
     * @param filePath 报告文件路径
     * @return 生成是否成功
     */
    static bool generatePdfReport(const QList<PerformanceMetrics>& metrics, const QString& filePath);
};

#endif // PERFORMANCEUTILS_H