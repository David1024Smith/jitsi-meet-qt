#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include "../interfaces/IProtocolHandler.h"
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QQueue>

/**
 * @brief HTTP协议处理器
 * 
 * HTTPProtocol实现了HTTP/HTTPS协议的处理逻辑，包括请求发送、
 * 响应处理、连接池管理等功能。
 */
class HTTPProtocol : public IProtocolHandler
{
    Q_OBJECT

public:
    /**
     * @brief HTTP请求方法枚举
     */
    enum RequestMethod {
        GET,                ///< GET请求
        POST,               ///< POST请求
        PUT,                ///< PUT请求
        DELETE,             ///< DELETE请求
        HEAD,               ///< HEAD请求
        OPTIONS,            ///< OPTIONS请求
        PATCH               ///< PATCH请求
    };
    Q_ENUM(RequestMethod)

    /**
     * @brief HTTP状态码枚举
     */
    enum StatusCode {
        OK = 200,                   ///< 成功
        Created = 201,              ///< 已创建
        Accepted = 202,             ///< 已接受
        NoContent = 204,            ///< 无内容
        BadRequest = 400,           ///< 错误请求
        Unauthorized = 401,         ///< 未授权
        Forbidden = 403,            ///< 禁止访问
        NotFound = 404,             ///< 未找到
        MethodNotAllowed = 405,     ///< 方法不允许
        InternalServerError = 500,  ///< 服务器内部错误
        BadGateway = 502,           ///< 网关错误
        ServiceUnavailable = 503    ///< 服务不可用
    };
    Q_ENUM(StatusCode)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit HTTPProtocol(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~HTTPProtocol();

    // IProtocolHandler接口实现
    bool initialize(const QVariantMap& config = QVariantMap()) override;
    bool start() override;
    void stop() override;
    ProtocolStatus protocolStatus() const override;
    QString protocolName() const override;
    QString protocolVersion() const override;

    QByteArray encodeMessage(MessageType type, const QVariantMap& data) override;
    bool decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data) override;
    bool handleReceivedData(const QByteArray& data) override;
    bool sendMessage(MessageType type, const QVariantMap& data) override;
    bool sendHeartbeat() override;

    bool supportsFeature(const QString& feature) const override;
    QStringList supportedFeatures() const override;
    void setParameter(const QString& key, const QVariant& value) override;
    QVariant parameter(const QString& key) const override;
    QVariantMap protocolStats() const override;

    /**
     * @brief 发送HTTP请求
     * @param method 请求方法
     * @param url 请求URL
     * @param data 请求数据
     * @param headers 请求头
     * @return 请求ID
     */
    QString sendRequest(RequestMethod method, const QString& url, 
                       const QByteArray& data = QByteArray(),
                       const QVariantMap& headers = QVariantMap());

    /**
     * @brief 发送GET请求
     * @param url 请求URL
     * @param headers 请求头
     * @return 请求ID
     */
    QString get(const QString& url, const QVariantMap& headers = QVariantMap());

    /**
     * @brief 发送POST请求
     * @param url 请求URL
     * @param data 请求数据
     * @param headers 请求头
     * @return 请求ID
     */
    QString post(const QString& url, const QByteArray& data, 
                const QVariantMap& headers = QVariantMap());

    /**
     * @brief 发送PUT请求
     * @param url 请求URL
     * @param data 请求数据
     * @param headers 请求头
     * @return 请求ID
     */
    QString put(const QString& url, const QByteArray& data,
               const QVariantMap& headers = QVariantMap());

    /**
     * @brief 发送DELETE请求
     * @param url 请求URL
     * @param headers 请求头
     * @return 请求ID
     */
    QString deleteResource(const QString& url, const QVariantMap& headers = QVariantMap());

    /**
     * @brief 取消请求
     * @param requestId 请求ID
     * @return 取消是否成功
     */
    bool cancelRequest(const QString& requestId);

    /**
     * @brief 设置基础URL
     * @param baseUrl 基础URL
     */
    void setBaseUrl(const QString& baseUrl);

    /**
     * @brief 获取基础URL
     * @return 基础URL
     */
    QString baseUrl() const;

    /**
     * @brief 设置默认请求头
     * @param headers 默认请求头
     */
    void setDefaultHeaders(const QVariantMap& headers);

    /**
     * @brief 获取默认请求头
     * @return 默认请求头
     */
    QVariantMap defaultHeaders() const;

    /**
     * @brief 设置请求超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setRequestTimeout(int timeout);

    /**
     * @brief 获取请求超时时间
     * @return 超时时间（毫秒）
     */
    int requestTimeout() const;

    /**
     * @brief 设置最大并发请求数
     * @param maxConcurrent 最大并发数
     */
    void setMaxConcurrentRequests(int maxConcurrent);

    /**
     * @brief 获取最大并发请求数
     * @return 最大并发数
     */
    int maxConcurrentRequests() const;

public slots:
    void reset() override;
    void refresh() override;

    /**
     * @brief 清除所有请求
     */
    void clearAllRequests();

    /**
     * @brief 重试失败的请求
     * @param requestId 请求ID
     */
    void retryRequest(const QString& requestId);

signals:
    /**
     * @brief 请求完成信号
     * @param requestId 请求ID
     * @param statusCode HTTP状态码
     * @param data 响应数据
     * @param headers 响应头
     */
    void requestCompleted(const QString& requestId, int statusCode, 
                         const QByteArray& data, const QVariantMap& headers);

    /**
     * @brief 请求失败信号
     * @param requestId 请求ID
     * @param error 错误信息
     */
    void requestFailed(const QString& requestId, const QString& error);

    /**
     * @brief 请求进度信号
     * @param requestId 请求ID
     * @param bytesReceived 已接收字节数
     * @param bytesTotal 总字节数
     */
    void requestProgress(const QString& requestId, qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief 上传进度信号
     * @param requestId 请求ID
     * @param bytesSent 已发送字节数
     * @param bytesTotal 总字节数
     */
    void uploadProgress(const QString& requestId, qint64 bytesSent, qint64 bytesTotal);

private slots:
    /**
     * @brief 处理网络回复完成
     */
    void handleReplyFinished();

    /**
     * @brief 处理网络错误
     * @param error 网络错误
     */
    void handleNetworkError(QNetworkReply::NetworkError error);

    /**
     * @brief 处理SSL错误
     * @param errors SSL错误列表
     */
    void handleSslErrors(const QList<QSslError>& errors);

    /**
     * @brief 处理下载进度
     * @param bytesReceived 已接收字节数
     * @param bytesTotal 总字节数
     */
    void handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief 处理上传进度
     * @param bytesSent 已发送字节数
     * @param bytesTotal 总字节数
     */
    void handleUploadProgress(qint64 bytesSent, qint64 bytesTotal);

    /**
     * @brief 处理请求超时
     */
    void handleRequestTimeout();

    /**
     * @brief 处理队列中的请求
     */
    void processRequestQueue();

private:
    /**
     * @brief 创建网络请求
     * @param method 请求方法
     * @param url 请求URL
     * @param headers 请求头
     * @return 网络请求对象
     */
    QNetworkRequest createNetworkRequest(RequestMethod method, const QString& url,
                                       const QVariantMap& headers);

    /**
     * @brief 执行网络请求
     * @param request 网络请求
     * @param method 请求方法
     * @param data 请求数据
     * @return 网络回复对象
     */
    QNetworkReply* executeRequest(const QNetworkRequest& request, RequestMethod method,
                                 const QByteArray& data);

    /**
     * @brief 生成请求ID
     * @return 请求ID
     */
    QString generateRequestId();

    /**
     * @brief 获取方法字符串
     * @param method 请求方法
     * @return 方法字符串
     */
    QString getMethodString(RequestMethod method) const;

    /**
     * @brief 解析响应头
     * @param reply 网络回复
     * @return 响应头映射
     */
    QVariantMap parseResponseHeaders(QNetworkReply* reply);

    /**
     * @brief 处理重定向
     * @param reply 网络回复
     * @return 是否需要重定向
     */
    bool handleRedirect(QNetworkReply* reply);

    /**
     * @brief 应用默认请求头
     * @param request 网络请求
     * @param customHeaders 自定义请求头
     */
    void applyHeaders(QNetworkRequest& request, const QVariantMap& customHeaders);

    /**
     * @brief 检查是否可以发送新请求
     * @return 是否可以发送
     */
    bool canSendNewRequest() const;

    /**
     * @brief 将请求加入队列
     * @param requestInfo 请求信息
     */
    void enqueueRequest(const QVariantMap& requestInfo);

    class Private;
    Private* d;
};

#endif // HTTPPROTOCOL_H