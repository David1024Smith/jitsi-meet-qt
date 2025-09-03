#ifndef NETWORKDIAGNOSTICS_H
#define NETWORKDIAGNOSTICS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QHostInfo>
#include <QTcpSocket>
#include <QJsonObject>

/**
 * @brief 网络诊断工具类
 * 
 * 提供网络连接诊断功能，包括DNS解析、连通性测试、代理检测等
 */
class NetworkDiagnostics : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 诊断结果结构
     */
    struct DiagnosticResult {
        bool success;           // 诊断是否成功
        QString operation;      // 诊断操作名称
        QString message;        // 结果消息
        int responseTime;       // 响应时间（毫秒）
        QJsonObject details;    // 详细信息
    };

    explicit NetworkDiagnostics(QObject *parent = nullptr);
    ~NetworkDiagnostics();

    /**
     * @brief 开始完整的网络诊断
     * @param serverUrl 要诊断的服务器URL
     */
    void startDiagnosis(const QString& serverUrl);

    /**
     * @brief 测试DNS解析
     * @param hostname 主机名
     */
    void testDnsResolution(const QString& hostname);

    /**
     * @brief 测试TCP连接
     * @param hostname 主机名
     * @param port 端口号
     */
    void testTcpConnection(const QString& hostname, int port);

    /**
     * @brief 测试HTTP连接
     * @param url 测试URL
     */
    void testHttpConnection(const QString& url);

    /**
     * @brief 检测代理设置
     */
    void detectProxySettings();

    /**
     * @brief 获取网络接口信息
     */
    void getNetworkInterfaces();

signals:
    /**
     * @brief 诊断步骤完成信号
     * @param result 诊断结果
     */
    void diagnosticStepCompleted(const NetworkDiagnostics::DiagnosticResult& result);

    /**
     * @brief 诊断完成信号
     * @param success 整体诊断是否成功
     * @param summary 诊断摘要
     */
    void diagnosisCompleted(bool success, const QString& summary);

    /**
     * @brief 诊断进度信号
     * @param progress 进度百分比 (0-100)
     * @param currentStep 当前步骤描述
     */
    void diagnosisProgress(int progress, const QString& currentStep);

private slots:
    /**
     * @brief DNS解析完成槽函数
     * @param hostInfo 主机信息
     */
    void onDnsLookupFinished(const QHostInfo& hostInfo);

    /**
     * @brief TCP连接状态变化槽函数
     */
    void onTcpConnectionStateChanged();

    /**
     * @brief HTTP请求完成槽函数
     */
    void onHttpRequestFinished();

    /**
     * @brief 网络错误处理槽函数
     * @param error 网络错误
     */
    void onNetworkError(QNetworkReply::NetworkError error);

    /**
     * @brief 诊断超时槽函数
     */
    void onDiagnosticTimeout();

private:
    /**
     * @brief 执行下一个诊断步骤
     */
    void executeNextStep();

    /**
     * @brief 完成当前诊断步骤
     * @param result 诊断结果
     */
    void completeCurrentStep(const DiagnosticResult& result);

    /**
     * @brief 生成诊断摘要
     * @return 诊断摘要字符串
     */
    QString generateSummary() const;

    /**
     * @brief 解析URL获取主机名和端口
     * @param url URL字符串
     * @param hostname 输出主机名
     * @param port 输出端口号
     * @return 解析是否成功
     */
    bool parseUrl(const QString& url, QString& hostname, int& port) const;

private:
    QNetworkAccessManager* m_networkManager;    // 网络管理器
    QTimer* m_timeoutTimer;                     // 超时定时器
    QTcpSocket* m_tcpSocket;                    // TCP套接字
    
    QString m_targetUrl;                        // 目标URL
    QString m_targetHostname;                   // 目标主机名
    int m_targetPort;                           // 目标端口
    
    QList<QString> m_diagnosticSteps;           // 诊断步骤列表
    int m_currentStepIndex;                     // 当前步骤索引
    QList<DiagnosticResult> m_results;          // 诊断结果列表
    
    QDateTime m_stepStartTime;                  // 步骤开始时间
    
    static const int DIAGNOSTIC_TIMEOUT = 10000; // 诊断超时时间（毫秒）
};

#endif // NETWORKDIAGNOSTICS_H