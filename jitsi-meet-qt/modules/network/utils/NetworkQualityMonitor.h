#ifndef NETWORKQUALITYMONITOR_H
#define NETWORKQUALITYMONITOR_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QQueue>
#include <QDateTime>

/**
 * @brief 网络质量监控器
 * 
 * NetworkQualityMonitor提供实时网络质量监控功能，包括延迟测试、
 * 带宽测试、丢包率检测等。
 */
class NetworkQualityMonitor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 网络质量等级枚举
     */
    enum QualityLevel {
        Excellent = 5,      ///< 优秀 (90-100分)
        Good = 4,           ///< 良好 (70-89分)
        Fair = 3,           ///< 一般 (50-69分)
        Poor = 2,           ///< 较差 (30-49分)
        VeryPoor = 1        ///< 很差 (0-29分)
    };
    Q_ENUM(QualityLevel)

    /**
     * @brief 监控状态枚举
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
     * @param parent 父对象
     */
    explicit NetworkQualityMonitor(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkQualityMonitor();

    /**
     * @brief 开始监控
     * @param targetHost 目标主机
     * @param interval 监控间隔（毫秒）
     * @return 是否成功启动
     */
    bool startMonitoring(const QString& targetHost, int interval = 5000);

    /**
     * @brief 停止监控
     */
    void stopMonitoring();

    /**
     * @brief 暂停监控
     */
    void pauseMonitoring();

    /**
     * @brief 恢复监控
     */
    void resumeMonitoring();

    /**
     * @brief 获取监控状态
     * @return 当前监控状态
     */
    MonitorStatus monitorStatus() const;

    /**
     * @brief 获取当前网络质量等级
     * @return 网络质量等级
     */
    QualityLevel currentQualityLevel() const;

    /**
     * @brief 获取当前网络质量分数
     * @return 质量分数 (0-100)
     */
    int currentQualityScore() const;

    /**
     * @brief 获取当前延迟
     * @return 延迟（毫秒）
     */
    int currentLatency() const;

    /**
     * @brief 获取当前丢包率
     * @return 丢包率（百分比）
     */
    double currentPacketLoss() const;

    /**
     * @brief 获取当前带宽
     * @return 带宽（kbps）
     */
    int currentBandwidth() const;

    /**
     * @brief 获取网络质量统计信息
     * @return 统计信息映射
     */
    QVariantMap getQualityStats() const;

    /**
     * @brief 获取历史数据
     * @param minutes 获取最近多少分钟的数据
     * @return 历史数据列表
     */
    QVariantList getHistoryData(int minutes = 10) const;

    /**
     * @brief 设置目标主机
     * @param host 目标主机
     */
    void setTargetHost(const QString& host);

    /**
     * @brief 获取目标主机
     * @return 目标主机
     */
    QString targetHost() const;

    /**
     * @brief 设置监控间隔
     * @param interval 间隔（毫秒）
     */
    void setMonitorInterval(int interval);

    /**
     * @brief 获取监控间隔
     * @return 间隔（毫秒）
     */
    int monitorInterval() const;

    /**
     * @brief 设置质量阈值
     * @param excellent 优秀阈值
     * @param good 良好阈值
     * @param fair 一般阈值
     * @param poor 较差阈值
     */
    void setQualityThresholds(int excellent = 90, int good = 70, int fair = 50, int poor = 30);

    /**
     * @brief 执行单次网络测试
     * @return 测试结果
     */
    QVariantMap performSingleTest();

    /**
     * @brief 重置统计数据
     */
    void resetStats();

public slots:
    /**
     * @brief 手动触发网络测试
     */
    void triggerTest();

signals:
    /**
     * @brief 监控状态改变信号
     * @param status 新的监控状态
     */
    void monitorStatusChanged(MonitorStatus status);

    /**
     * @brief 网络质量改变信号
     * @param level 新的质量等级
     * @param score 质量分数
     */
    void qualityChanged(QualityLevel level, int score);

    /**
     * @brief 网络测试完成信号
     * @param result 测试结果
     */
    void testCompleted(const QVariantMap& result);

    /**
     * @brief 延迟改变信号
     * @param latency 新的延迟值（毫秒）
     */
    void latencyChanged(int latency);

    /**
     * @brief 丢包率改变信号
     * @param packetLoss 新的丢包率（百分比）
     */
    void packetLossChanged(double packetLoss);

    /**
     * @brief 带宽改变信号
     * @param bandwidth 新的带宽值（kbps）
     */
    void bandwidthChanged(int bandwidth);

    /**
     * @brief 网络质量警告信号
     * @param level 质量等级
     * @param message 警告消息
     */
    void qualityWarning(QualityLevel level, const QString& message);

    /**
     * @brief 监控错误信号
     * @param error 错误信息
     */
    void monitorError(const QString& error);

private slots:
    /**
     * @brief 处理监控定时器
     */
    void handleMonitorTimer();

    /**
     * @brief 处理延迟测试完成
     */
    void handleLatencyTestCompleted();

    /**
     * @brief 处理带宽测试完成
     */
    void handleBandwidthTestCompleted();

private:
    /**
     * @brief 执行延迟测试
     * @return 延迟值（毫秒）
     */
    int performLatencyTest();

    /**
     * @brief 执行丢包测试
     * @return 丢包率（百分比）
     */
    double performPacketLossTest();

    /**
     * @brief 执行带宽测试
     * @return 带宽值（kbps）
     */
    int performBandwidthTest();

    /**
     * @brief 计算网络质量分数
     * @param latency 延迟
     * @param packetLoss 丢包率
     * @param bandwidth 带宽
     * @return 质量分数
     */
    int calculateQualityScore(int latency, double packetLoss, int bandwidth);

    /**
     * @brief 获取质量等级
     * @param score 质量分数
     * @return 质量等级
     */
    QualityLevel getQualityLevel(int score);

    /**
     * @brief 更新统计信息
     * @param latency 延迟
     * @param packetLoss 丢包率
     * @param bandwidth 带宽
     * @param score 质量分数
     */
    void updateStats(int latency, double packetLoss, int bandwidth, int score);

    /**
     * @brief 添加历史数据点
     * @param data 数据点
     */
    void addHistoryDataPoint(const QVariantMap& data);

    /**
     * @brief 清理过期的历史数据
     */
    void cleanupHistoryData();

    /**
     * @brief 检查质量变化并发送警告
     * @param newLevel 新的质量等级
     * @param oldLevel 旧的质量等级
     */
    void checkQualityWarning(QualityLevel newLevel, QualityLevel oldLevel);

    class Private;
    Private* d;
};

#endif // NETWORKQUALITYMONITOR_H