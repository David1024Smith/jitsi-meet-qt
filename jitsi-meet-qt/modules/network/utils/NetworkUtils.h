#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QString>
#include <QVariantMap>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QStringList>

/**
 * @brief 网络工具类
 * 
 * NetworkUtils提供各种网络相关的实用函数，包括网络检测、
 * 地址解析、连接测试等功能。
 */
class NetworkUtils
{
public:
    /**
     * @brief 网络连接类型枚举
     */
    enum ConnectionType {
        Unknown,            ///< 未知连接
        Ethernet,           ///< 以太网连接
        WiFi,               ///< WiFi连接
        Mobile,             ///< 移动网络连接
        VPN                 ///< VPN连接
    };

    /**
     * @brief 检查网络连接状态
     * @return 是否有网络连接
     */
    static bool isNetworkAvailable();

    /**
     * @brief 获取当前网络连接类型
     * @return 连接类型
     */
    static ConnectionType getConnectionType();

    /**
     * @brief 获取本地IP地址
     * @param preferIPv4 是否优先返回IPv4地址
     * @return 本地IP地址
     */
    static QString getLocalIPAddress(bool preferIPv4 = true);

    /**
     * @brief 获取所有网络接口信息
     * @return 网络接口列表
     */
    static QList<QNetworkInterface> getNetworkInterfaces();

    /**
     * @brief 获取活动的网络接口
     * @return 活动网络接口列表
     */
    static QList<QNetworkInterface> getActiveInterfaces();

    /**
     * @brief 解析主机名到IP地址
     * @param hostname 主机名
     * @return IP地址列表
     */
    static QStringList resolveHostname(const QString& hostname);

    /**
     * @brief 检查端口是否可达
     * @param host 主机地址
     * @param port 端口号
     * @param timeout 超时时间（毫秒）
     * @return 端口是否可达
     */
    static bool isPortReachable(const QString& host, int port, int timeout = 5000);

    /**
     * @brief 测试网络延迟
     * @param host 目标主机
     * @param timeout 超时时间（毫秒）
     * @return 延迟时间（毫秒），-1表示失败
     */
    static int pingHost(const QString& host, int timeout = 5000);

    /**
     * @brief 获取网络统计信息
     * @return 统计信息映射
     */
    static QVariantMap getNetworkStats();

    /**
     * @brief 格式化带宽值
     * @param bytes 字节数
     * @param precision 精度
     * @return 格式化的带宽字符串
     */
    static QString formatBandwidth(qint64 bytes, int precision = 2);

    /**
     * @brief 格式化延迟值
     * @param milliseconds 毫秒数
     * @return 格式化的延迟字符串
     */
    static QString formatLatency(int milliseconds);

    /**
     * @brief 验证IP地址格式
     * @param address IP地址字符串
     * @return 是否为有效IP地址
     */
    static bool isValidIPAddress(const QString& address);

    /**
     * @brief 验证端口号
     * @param port 端口号
     * @return 是否为有效端口号
     */
    static bool isValidPort(int port);

    /**
     * @brief 获取默认网关
     * @return 默认网关IP地址
     */
    static QString getDefaultGateway();

    /**
     * @brief 获取DNS服务器列表
     * @return DNS服务器IP地址列表
     */
    static QStringList getDNSServers();

    /**
     * @brief 检查是否为私有IP地址
     * @param address IP地址
     * @return 是否为私有IP
     */
    static bool isPrivateIPAddress(const QString& address);

    /**
     * @brief 获取网络质量评分
     * @param latency 延迟（毫秒）
     * @param packetLoss 丢包率（百分比）
     * @param bandwidth 带宽（kbps）
     * @return 质量评分（0-100）
     */
    static int calculateNetworkQuality(int latency, double packetLoss, int bandwidth);

    /**
     * @brief 生成随机端口号
     * @param minPort 最小端口号
     * @param maxPort 最大端口号
     * @return 随机端口号
     */
    static int generateRandomPort(int minPort = 1024, int maxPort = 65535);

    /**
     * @brief 检查URL格式
     * @param url URL字符串
     * @return 是否为有效URL
     */
    static bool isValidUrl(const QString& url);

    /**
     * @brief 提取URL中的主机名
     * @param url URL字符串
     * @return 主机名
     */
    static QString extractHostFromUrl(const QString& url);

    /**
     * @brief 提取URL中的端口号
     * @param url URL字符串
     * @return 端口号，-1表示未指定
     */
    static int extractPortFromUrl(const QString& url);

private:
    NetworkUtils() = delete;  // 工具类，禁止实例化
};

#endif // NETWORKUTILS_H