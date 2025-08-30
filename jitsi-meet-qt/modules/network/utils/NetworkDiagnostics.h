#ifndef NETWORKDIAGNOSTICS_H
#define NETWORKDIAGNOSTICS_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

/**
 * @brief 网络诊断工具类
 * 
 * NetworkDiagnostics提供全面的网络诊断功能，包括连接测试、
 * 路由跟踪、DNS解析测试等。
 */
class NetworkDiagnostics : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 诊断结果状态枚举
     */
    enum DiagnosticStatus {
        Success,            ///< 成功
        Warning,            ///< 警告
        Error,              ///< 错误
        Timeout,            ///< 超时
        Unknown             ///< 未知
    };
    Q_ENUM(DiagnosticStatus)

    /**
     * @brief 诊断测试类型枚举
     */
    enum TestType {
        ConnectivityTest,   ///< 连接性测试
        LatencyTest,        ///< 延迟测试
        BandwidthTest,      ///< 带宽测试
        DNSTest,            ///< DNS测试
        RouteTest,          ///< 路由测试
        PortTest,           ///< 端口测试
        FirewallTest        ///< 防火墙测试
    };
    Q_ENUM(TestType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit NetworkDiagnostics(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkDiagnostics();

    /**
     * @brief 执行完整的网络诊断
     * @param targetHost 目标主机
     * @param targetPort 目标端口
     * @return 诊断结果
     */
    QVariantMap performFullDiagnostics(const QString& targetHost, int targetPort = 80);

    /**
     * @brief 执行连接性测试
     * @param host 目标主机
     * @param port 目标端口
     * @param timeout 超时时间（毫秒）
     * @return 测试结果
     */
    QVariantMap testConnectivity(const QString& host, int port = 80, int timeout = 5000);

    /**
     * @brief 执行延迟测试
     * @param host 目标主机
     * @param count 测试次数
     * @param timeout 超时时间（毫秒）
     * @return 测试结果
     */
    QVariantMap testLatency(const QString& host, int count = 5, int timeout = 5000);

    /**
     * @brief 执行DNS解析测试
     * @param hostname 主机名
     * @param dnsServer DNS服务器（可选）
     * @return 测试结果
     */
    QVariantMap testDNSResolution(const QString& hostname, const QString& dnsServer = QString());

    /**
     * @brief 执行路由跟踪测试
     * @param host 目标主机
     * @param maxHops 最大跳数
     * @return 测试结果
     */
    QVariantMap testRoute(const QString& host, int maxHops = 30);

    /**
     * @brief 执行端口扫描测试
     * @param host 目标主机
     * @param ports 端口列表
     * @param timeout 超时时间（毫秒）
     * @return 测试结果
     */
    QVariantMap testPorts(const QString& host, const QList<int>& ports, int timeout = 3000);

    /**
     * @brief 执行防火墙测试
     * @param host 目标主机
     * @param port 目标端口
     * @return 测试结果
     */
    QVariantMap testFirewall(const QString& host, int port = 80);

    /**
     * @brief 测试网络接口状态
     * @return 测试结果
     */
    QVariantMap testNetworkInterfaces();

    /**
     * @brief 测试默认网关
     * @return 测试结果
     */
    QVariantMap testDefaultGateway();

    /**
     * @brief 测试DNS服务器
     * @return 测试结果
     */
    QVariantMap testDNSServers();

    /**
     * @brief 生成诊断报告
     * @param results 诊断结果列表
     * @return 格式化的诊断报告
     */
    QString generateDiagnosticReport(const QVariantList& results);

    /**
     * @brief 获取网络配置信息
     * @return 网络配置信息
     */
    QVariantMap getNetworkConfiguration();

    /**
     * @brief 获取系统网络信息
     * @return 系统网络信息
     */
    QVariantMap getSystemNetworkInfo();

    /**
     * @brief 检查网络连接问题
     * @param host 目标主机
     * @param port 目标端口
     * @return 问题诊断结果
     */
    QVariantMap diagnoseConnectionIssues(const QString& host, int port = 80);

    /**
     * @brief 获建议的网络优化建议
     * @param diagnosticResults 诊断结果
     * @return 优化建议列表
     */
    QStringList getOptimizationSuggestions(const QVariantMap& diagnosticResults);

public slots:
    /**
     * @brief 执行快速网络检查
     * @param host 目标主机
     */
    void performQuickCheck(const QString& host = "8.8.8.8");

signals:
    /**
     * @brief 诊断开始信号
     * @param testType 测试类型
     * @param target 测试目标
     */
    void diagnosticStarted(TestType testType, const QString& target);

    /**
     * @brief 诊断完成信号
     * @param testType 测试类型
     * @param result 测试结果
     */
    void diagnosticCompleted(TestType testType, const QVariantMap& result);

    /**
     * @brief 诊断进度信号
     * @param testType 测试类型
     * @param progress 进度百分比 (0-100)
     * @param message 进度消息
     */
    void diagnosticProgress(TestType testType, int progress, const QString& message);

    /**
     * @brief 诊断错误信号
     * @param testType 测试类型
     * @param error 错误信息
     */
    void diagnosticError(TestType testType, const QString& error);

private:
    /**
     * @brief 创建测试结果
     * @param testType 测试类型
     * @param status 测试状态
     * @param message 结果消息
     * @param data 附加数据
     * @return 测试结果映射
     */
    QVariantMap createTestResult(TestType testType, DiagnosticStatus status, 
                                const QString& message, const QVariantMap& data = QVariantMap());

    /**
     * @brief 执行系统命令并获取输出
     * @param command 命令
     * @param arguments 参数列表
     * @param timeout 超时时间（毫秒）
     * @return 命令输出
     */
    QString executeSystemCommand(const QString& command, const QStringList& arguments, int timeout = 10000);

    /**
     * @brief 解析ping命令输出
     * @param output ping命令输出
     * @return 解析结果
     */
    QVariantMap parsePingOutput(const QString& output);

    /**
     * @brief 解析traceroute命令输出
     * @param output traceroute命令输出
     * @return 解析结果
     */
    QVariantMap parseTracerouteOutput(const QString& output);

    /**
     * @brief 解析nslookup命令输出
     * @param output nslookup命令输出
     * @return 解析结果
     */
    QVariantMap parseNslookupOutput(const QString& output);

    /**
     * @brief 检测操作系统类型
     * @return 操作系统类型字符串
     */
    QString detectOperatingSystem();

    /**
     * @brief 获取平台特定的ping命令
     * @return ping命令和参数
     */
    QPair<QString, QStringList> getPingCommand(const QString& host, int count);

    /**
     * @brief 获取平台特定的traceroute命令
     * @return traceroute命令和参数
     */
    QPair<QString, QStringList> getTracerouteCommand(const QString& host, int maxHops);

    /**
     * @brief 获取平台特定的nslookup命令
     * @return nslookup命令和参数
     */
    QPair<QString, QStringList> getNslookupCommand(const QString& hostname, const QString& dnsServer);

    /**
     * @brief 分析网络问题
     * @param results 测试结果列表
     * @return 问题分析结果
     */
    QVariantMap analyzeNetworkIssues(const QVariantList& results);

    /**
     * @brief 格式化测试结果为可读文本
     * @param result 测试结果
     * @return 格式化文本
     */
    QString formatTestResult(const QVariantMap& result);

    class Private;
    Private* d;
};

#endif // NETWORKDIAGNOSTICS_H