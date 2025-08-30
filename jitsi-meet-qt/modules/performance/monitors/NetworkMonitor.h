#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include "BaseMonitor.h"
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>

/**
 * @brief 网络监控器类
 * 
 * NetworkMonitor专门负责监控网络相关的性能指标：
 * - 网络带宽使用监控
 * - 网络延迟监控
 * - 网络连接质量监控
 * - 数据包丢失率监控
 * - 网络接口状态监控
 */
class NetworkMonitor : public BaseMonitor
{
    Q_OBJECT

public:
    /**
     * @brief 网络监控模式枚举
     */
    enum MonitoringMode {
        BasicMode,          ///< 基础模式 - 基本网络统计
        BandwidthMode,      ///< 带宽模式 - 详细带宽监控
        LatencyMode,        ///< 延迟模式 - 网络延迟监控
        QualityMode,        ///< 质量模式 - 连接质量监控
        ComprehensiveMode   ///< 综合模式 - 全面网络监控
    };
    Q_ENUM(MonitoringMode)

    /**
     * @brief 网络接口类型枚举
     */
    enum InterfaceType {
        Ethernet,           ///< 以太网
        WiFi,               ///< 无线网络
        Cellular,           ///< 蜂窝网络
        Loopback,           ///< 回环接口
        VPN,                ///< VPN接口
        Unknown             ///< 未知类型
    };
    Q_ENUM(InterfaceType)

    /**
     * @brief 网络质量等级枚举
     */
    enum QualityLevel {
        Excellent = 5,      ///< 优秀
        Good = 4,           ///< 良好
        Fair = 3,           ///< 一般
        Poor = 2,           ///< 较差
        Critical = 1        ///< 严重
    };
    Q_ENUM(QualityLevel)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NetworkMonitor(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkMonitor();

    /**
     * @brief 获取监控器版本
     * @return 版本字符串
     */
    QString version() const override;

    /**
     * @brief 获取监控器描述
     * @return 描述字符串
     */
    QString description() const override;

    /**
     * @brief 设置监控模式
     * @param mode 监控模式
     */
    void setMonitoringMode(MonitoringMode mode);

    /**
     * @brief 获取监控模式
     * @return 监控模式
     */
    MonitoringMode monitoringMode() const;

    /**
     * @brief 获取所有网络接口
     * @return 网络接口列表
     */
    QList<QNetworkInterface> getAllInterfaces() const;

    /**
     * @brief 获取活动网络接口
     * @return 活动网络接口列表
     */
    QList<QNetworkInterface> getActiveInterfaces() const;

    /**
     * @brief 获取主要网络接口
     * @return 主要网络接口
     */
    QNetworkInterface getPrimaryInterface() const;

    /**
     * @brief 设置监控的网络接口
     * @param interfaceName 接口名称
     */
    void setMonitoredInterface(const QString& interfaceName);

    /**
     * @brief 获取监控的网络接口
     * @return 接口名称
     */
    QString monitoredInterface() const;

    /**
     * @brief 获取接口类型
     * @param interface 网络接口
     * @return 接口类型
     */
    InterfaceType getInterfaceType(const QNetworkInterface& interface) const;

    /**
     * @brief 获取下载速度
     * @param interfaceName 接口名称
     * @return 下载速度(字节/秒)
     */
    qint64 getDownloadSpeed(const QString& interfaceName = QString()) const;

    /**
     * @brief 获取上传速度
     * @param interfaceName 接口名称
     * @return 上传速度(字节/秒)
     */
    qint64 getUploadSpeed(const QString& interfaceName = QString()) const;

    /**
     * @brief 获取总下载字节数
     * @param interfaceName 接口名称
     * @return 总下载字节数
     */
    qint64 getTotalBytesReceived(const QString& interfaceName = QString()) const;

    /**
     * @brief 获取总上传字节数
     * @param interfaceName 接口名称
     * @return 总上传字节数
     */
    qint64 getTotalBytesSent(const QString& interfaceName = QString()) const;

    /**
     * @brief 获取网络延迟
     * @param host 目标主机
     * @return 网络延迟(毫秒)
     */
    double getNetworkLatency(const QString& host = "8.8.8.8") const;

    /**
     * @brief 获取数据包丢失率
     * @param host 目标主机
     * @return 丢包率(%)
     */
    double getPacketLoss(const QString& host = "8.8.8.8") const;

    /**
     * @brief 获取网络抖动
     * @param host 目标主机
     * @return 网络抖动(毫秒)
     */
    double getNetworkJitter(const QString& host = "8.8.8.8") const;

    /**
     * @brief 获取网络质量等级
     * @return 网络质量等级
     */
    QualityLevel getNetworkQuality() const;

    /**
     * @brief 获取网络质量评分
     * @return 网络质量评分(0-100)
     */
    int getNetworkScore() const;

    /**
     * @brief 获取连接数量
     * @return 活动连接数量
     */
    int getConnectionCount() const;

    /**
     * @brief 获取带宽使用历史
     * @param minutes 历史分钟数
     * @return 带宽使用历史数据
     */
    QList<QPair<qint64, qint64>> getBandwidthHistory(int minutes) const;

    /**
     * @brief 获取延迟历史
     * @param minutes 历史分钟数
     * @return 延迟历史数据
     */
    QList<double> getLatencyHistory(int minutes) const;

    /**
     * @brief 执行网络速度测试
     * @param testUrl 测试URL
     * @return 测试结果(下载速度, 上传速度)
     */
    QPair<double, double> performSpeedTest(const QString& testUrl = QString()) const;

    /**
     * @brief 执行ping测试
     * @param host 目标主机
     * @param count ping次数
     * @return ping结果(平均延迟, 丢包率)
     */
    QPair<double, double> performPingTest(const QString& host, int count = 10) const;

    /**
     * @brief 检查网络连接状态
     * @return 是否连接到网络
     */
    bool isNetworkConnected() const;

    /**
     * @brief 检查互联网连接状态
     * @return 是否连接到互联网
     */
    bool isInternetConnected() const;

    /**
     * @brief 获取公网IP地址
     * @return 公网IP地址
     */
    QString getPublicIPAddress() const;

    /**
     * @brief 获取本地IP地址
     * @return 本地IP地址列表
     */
    QStringList getLocalIPAddresses() const;

    /**
     * @brief 设置延迟测试主机列表
     * @param hosts 主机列表
     */
    void setLatencyTestHosts(const QStringList& hosts);

    /**
     * @brief 获取延迟测试主机列表
     * @return 主机列表
     */
    QStringList latencyTestHosts() const;

    /**
     * @brief 获取网络统计信息
     * @return 网络统计信息
     */
    QVariantMap getNetworkStatistics() const;

protected:
    /**
     * @brief 初始化网络监控器
     * @return 初始化是否成功
     */
    bool initializeMonitor() override;

    /**
     * @brief 收集网络资源使用数据
     * @return 网络资源使用数据
     */
    ResourceUsage collectResourceUsage() override;

    /**
     * @brief 获取支持的资源类型
     * @return 支持的资源类型列表
     */
    QList<ResourceType> supportedResourceTypes() const override;

private slots:
    /**
     * @brief 处理网络状态变化
     * @param accessible 网络是否可访问
     */
    void handleNetworkAccessibilityChanged(QNetworkAccessManager::NetworkAccessibility accessible);

    /**
     * @brief 处理延迟测试完成
     */
    void handleLatencyTestFinished();

    /**
     * @brief 处理速度测试完成
     */
    void handleSpeedTestFinished();

    /**
     * @brief 执行定期网络检查
     */
    void performPeriodicCheck();

private:
    /**
     * @brief 初始化网络接口监控
     * @return 初始化是否成功
     */
    bool initializeInterfaceMonitoring();

    /**
     * @brief 更新接口统计信息
     */
    void updateInterfaceStatistics();

    /**
     * @brief 计算网络速度
     * @param interfaceName 接口名称
     * @return 网络速度(下载, 上传)
     */
    QPair<qint64, qint64> calculateNetworkSpeed(const QString& interfaceName);

    /**
     * @brief 执行延迟测试
     * @param host 目标主机
     * @return 延迟(毫秒)
     */
    double performLatencyTest(const QString& host);

    /**
     * @brief 计算网络质量
     * @return 网络质量等级
     */
    QualityLevel calculateNetworkQuality();

    /**
     * @brief 读取接口统计信息
     * @param interfaceName 接口名称
     * @return 统计信息
     */
    QVariantMap readInterfaceStatistics(const QString& interfaceName);

#ifdef Q_OS_WIN
    /**
     * @brief Windows平台网络统计读取
     * @param interfaceName 接口名称
     * @return 网络统计信息
     */
    QVariantMap readNetworkStatisticsWindows(const QString& interfaceName);
#endif

#ifdef Q_OS_LINUX
    /**
     * @brief Linux平台网络统计读取
     * @param interfaceName 接口名称
     * @return 网络统计信息
     */
    QVariantMap readNetworkStatisticsLinux(const QString& interfaceName);

    /**
     * @brief 解析/proc/net/dev文件
     * @return 网络接口统计信息
     */
    QVariantMap parseProcNetDev();
#endif

#ifdef Q_OS_MACOS
    /**
     * @brief macOS平台网络统计读取
     * @param interfaceName 接口名称
     * @return 网络统计信息
     */
    QVariantMap readNetworkStatisticsMacOS(const QString& interfaceName);
#endif

    MonitoringMode m_monitoringMode;                ///< 监控模式
    QString m_monitoredInterface;                   ///< 监控的网络接口
    QStringList m_latencyTestHosts;                 ///< 延迟测试主机列表
    
    QNetworkAccessManager* m_networkManager;        ///< 网络访问管理器
    QTimer* m_periodicTimer;                        ///< 定期检查定时器
    QTimer* m_latencyTimer;                         ///< 延迟测试定时器
    
    // 网络统计数据
    struct InterfaceStatistics {
        qint64 bytesReceived = 0;
        qint64 bytesSent = 0;
        qint64 packetsReceived = 0;
        qint64 packetsSent = 0;
        QDateTime timestamp;
    };
    QMap<QString, InterfaceStatistics> m_interfaceStats; ///< 接口统计信息
    QMap<QString, InterfaceStatistics> m_lastInterfaceStats; ///< 上次接口统计信息
    
    // 历史数据
    QList<QPair<qint64, qint64>> m_bandwidthHistory; ///< 带宽使用历史
    QList<double> m_latencyHistory;                  ///< 延迟历史
    QList<double> m_jitterHistory;                   ///< 抖动历史
    QList<double> m_packetLossHistory;               ///< 丢包率历史
    
    // 网络质量数据
    QualityLevel m_currentQuality;                   ///< 当前网络质量
    int m_currentScore;                              ///< 当前网络评分
    
    // 缓存数据
    mutable QMutex m_dataMutex;                      ///< 数据互斥锁
    bool m_isConnected;                              ///< 网络连接状态
    bool m_isInternetConnected;                      ///< 互联网连接状态
    QString m_publicIP;                              ///< 公网IP地址
    QDateTime m_lastUpdateTime;                      ///< 上次更新时间
    
    // 测试相关
    QNetworkReply* m_currentSpeedTest;               ///< 当前速度测试
    QTcpSocket* m_latencySocket;                     ///< 延迟测试套接字
    QMap<QString, QList<double>> m_hostLatencies;    ///< 主机延迟记录
};

#endif // NETWORKMONITOR_H