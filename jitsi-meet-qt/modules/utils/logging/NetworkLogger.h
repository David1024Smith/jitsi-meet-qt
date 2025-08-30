#ifndef NETWORKLOGGER_H
#define NETWORKLOGGER_H

#include "../interfaces/ILogger.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief 网络日志记录器
 * 
 * NetworkLogger实现了基于网络的日志记录功能，支持HTTP/HTTPS协议、
 * 批量发送、重试机制和缓冲区管理。
 */
class NetworkLogger : public ILogger
{
    Q_OBJECT

public:
    /**
     * @brief 传输协议枚举
     */
    enum Protocol {
        HTTP,           ///< HTTP协议
        HTTPS           ///< HTTPS协议
    };
    Q_ENUM(Protocol)

    /**
     * @brief 数据格式枚举
     */
    enum DataFormat {
        JSON,           ///< JSON格式
        XML,            ///< XML格式
        PlainText       ///< 纯文本格式
    };
    Q_ENUM(DataFormat)

    /**
     * @brief 网络日志配置结构
     */
    struct NetworkConfig {
        QString serverUrl;          ///< 服务器URL
        Protocol protocol;          ///< 传输协议
        DataFormat format;          ///< 数据格式
        int port;                   ///< 端口号
        QString endpoint;           ///< API端点
        QString apiKey;             ///< API密钥
        QString username;           ///< 用户名
        QString password;           ///< 密码
        int timeout;                ///< 超时时间（毫秒）
        int maxRetries;             ///< 最大重试次数
        int batchSize;              ///< 批量发送大小
        int bufferSize;             ///< 缓冲区大小
        bool compressionEnabled;    ///< 是否启用压缩
        bool encryptionEnabled;     ///< 是否启用加密
        
        NetworkConfig() 
            : protocol(HTTPS), format(JSON), port(443), timeout(30000)
            , maxRetries(3), batchSize(10), bufferSize(1000)
            , compressionEnabled(true), encryptionEnabled(true) {}
    };

    /**
     * @brief 构造函数
     * @param config 网络配置
     * @param parent 父对象
     */
    explicit NetworkLogger(const NetworkConfig& config, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~NetworkLogger() override;

    // ILogger接口实现
    bool initialize() override;
    void cleanup() override;
    void log(const LogEntry& entry) override;
    void setLogLevel(LogLevel level) override;
    LogLevel logLevel() const override;
    void setFormat(const QString& format) override;
    QString format() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    QString name() const override;
    void flush() override;

    /**
     * @brief 设置网络配置
     * @param config 网络配置
     */
    void setNetworkConfig(const NetworkConfig& config);

    /**
     * @brief 获取网络配置
     * @return 网络配置
     */
    NetworkConfig networkConfig() const;

    /**
     * @brief 设置批量发送间隔（毫秒）
     * @param interval 发送间隔
     */
    void setBatchInterval(int interval);

    /**
     * @brief 获取批量发送间隔
     * @return 发送间隔
     */
    int batchInterval() const;

    /**
     * @brief 设置连接超时时间（毫秒）
     * @param timeout 超时时间
     */
    void setConnectionTimeout(int timeout);

    /**
     * @brief 获取连接超时时间
     * @return 超时时间
     */
    int connectionTimeout() const;

    /**
     * @brief 检查网络连接状态
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取缓冲区中的日志数量
     * @return 日志数量
     */
    int bufferedLogCount() const;

    /**
     * @brief 获取发送统计信息
     * @return 统计信息JSON对象
     */
    QJsonObject getStatistics() const;

    /**
     * @brief 清空缓冲区
     */
    void clearBuffer();

    /**
     * @brief 测试网络连接
     * @return 连接测试是否成功
     */
    bool testConnection();

private slots:
    /**
     * @brief 批量发送定时器槽函数
     */
    void onBatchTimer();

    /**
     * @brief 网络请求完成槽函数
     * @param reply 网络回复
     */
    void onRequestFinished(QNetworkReply* reply);

    /**
     * @brief 网络错误处理槽函数
     * @param error 网络错误
     */
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    /**
     * @brief 添加日志到缓冲区
     * @param entry 日志条目
     */
    void addToBuffer(const LogEntry& entry);

    /**
     * @brief 发送批量日志
     */
    void sendBatch();

    /**
     * @brief 发送单个日志条目
     * @param entry 日志条目
     */
    void sendLogEntry(const LogEntry& entry);

    /**
     * @brief 创建网络请求
     * @param data 请求数据
     * @return 网络请求对象
     */
    QNetworkRequest createRequest(const QByteArray& data);

    /**
     * @brief 格式化日志条目为指定格式
     * @param entry 日志条目
     * @return 格式化后的数据
     */
    QByteArray formatLogEntry(const LogEntry& entry) const;

    /**
     * @brief 格式化批量日志条目
     * @param entries 日志条目列表
     * @return 格式化后的数据
     */
    QByteArray formatBatchEntries(const QList<LogEntry>& entries) const;

    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @return 压缩后的数据
     */
    QByteArray compressData(const QByteArray& data) const;

    /**
     * @brief 加密数据
     * @param data 原始数据
     * @return 加密后的数据
     */
    QByteArray encryptData(const QByteArray& data) const;

    /**
     * @brief 重试发送失败的请求
     * @param data 请求数据
     * @param retryCount 当前重试次数
     */
    void retryRequest(const QByteArray& data, int retryCount);

    /**
     * @brief 构建完整的服务器URL
     * @return 完整URL
     */
    QString buildServerUrl() const;

    /**
     * @brief 更新统计信息
     * @param success 是否成功
     * @param logCount 日志数量
     */
    void updateStatistics(bool success, int logCount);

private:
    LogLevel m_logLevel;                        ///< 日志级别
    QString m_format;                           ///< 日志格式
    bool m_enabled;                             ///< 是否启用
    
    NetworkConfig m_config;                     ///< 网络配置
    QNetworkAccessManager* m_networkManager;    ///< 网络访问管理器
    
    QQueue<LogEntry> m_logBuffer;               ///< 日志缓冲区
    QTimer* m_batchTimer;                       ///< 批量发送定时器
    int m_batchInterval;                        ///< 批量发送间隔
    
    bool m_connected;                           ///< 连接状态
    
    // 统计信息
    struct Statistics {
        qint64 totalSent;           ///< 总发送数量
        qint64 totalFailed;         ///< 总失败数量
        qint64 totalBytes;          ///< 总字节数
        QDateTime lastSent;         ///< 最后发送时间
        double averageLatency;      ///< 平均延迟
        
        Statistics() : totalSent(0), totalFailed(0), totalBytes(0), averageLatency(0.0) {}
    } m_statistics;
    
    mutable QMutex m_bufferMutex;               ///< 缓冲区互斥锁
    mutable QMutex m_statisticsMutex;           ///< 统计信息互斥锁
    
    QHash<QNetworkReply*, QByteArray> m_pendingRequests;  ///< 待处理请求
    QHash<QNetworkReply*, int> m_retryCount;              ///< 重试计数
};

#endif // NETWORKLOGGER_H