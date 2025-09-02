#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QRegularExpression>
#include <QTimer>
#include <QMutex>
#include <memory>

class MainApplication;

/**
 * @brief 协议处理器类 - 负责处理jitsi-meet://协议链接
 * 
 * 该类负责：
 * - 注册和处理jitsi-meet://协议
 * - 解析协议URL并提取会议信息
 * - 验证URL的有效性
 * - 与主应用程序通信以启动会议
 * - 处理多种URL格式
 */
class ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 会议信息结构体
     */
    struct MeetingInfo {
        QString roomName;           ///< 房间名称
        QString serverUrl;          ///< 服务器URL
        QString fullUrl;            ///< 完整的会议URL
        QString displayName;        ///< 显示名称（可选）
        QString password;           ///< 会议密码（可选）
        QStringList parameters;     ///< 额外参数
        bool isValid;               ///< 是否有效
        
        MeetingInfo() : isValid(false) {}
    };

    /**
     * @brief 构造函数
     * @param app 主应用程序实例
     * @param parent 父对象
     */
    explicit ProtocolHandler(MainApplication* app, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ProtocolHandler();
    
    /**
     * @brief 注册协议处理器到系统
     * @return 是否注册成功
     */
    bool registerProtocol();
    
    /**
     * @brief 取消注册协议处理器
     * @return 是否取消注册成功
     */
    bool unregisterProtocol();
    
    /**
     * @brief 检查协议是否已注册
     * @return 是否已注册
     */
    bool isProtocolRegistered() const;
    
    /**
     * @brief 处理协议URL
     * @param url 协议URL字符串
     * @return 是否处理成功
     */
    bool handleProtocolUrl(const QString& url);
    
    /**
     * @brief 解析协议URL
     * @param url 协议URL字符串
     * @return 解析后的会议信息
     */
    MeetingInfo parseProtocolUrl(const QString& url) const;
    
    /**
     * @brief 验证URL是否为有效的jitsi-meet协议URL
     * @param url URL字符串
     * @return 是否有效
     */
    bool isValidProtocolUrl(const QString& url) const;
    
    /**
     * @brief 获取支持的协议名称
     * @return 协议名称
     */
    static QString getProtocolName();
    
    /**
     * @brief 构建协议URL
     * @param roomName 房间名称
     * @param serverUrl 服务器URL（可选，使用默认服务器）
     * @param displayName 显示名称（可选）
     * @param password 密码（可选）
     * @return 构建的协议URL
     */
    static QString buildProtocolUrl(const QString& roomName, 
                                   const QString& serverUrl = QString(),
                                   const QString& displayName = QString(),
                                   const QString& password = QString());
    
    /**
     * @brief 从HTTP/HTTPS URL转换为协议URL
     * @param httpUrl HTTP或HTTPS的Jitsi Meet URL
     * @return 转换后的协议URL
     */
    static QString convertFromHttpUrl(const QString& httpUrl);
    
    /**
     * @brief 转换协议URL为HTTP URL
     * @param protocolUrl 协议URL
     * @return HTTP URL
     */
    static QString convertToHttpUrl(const QString& protocolUrl);
    
    /**
     * @brief 设置延迟处理时间（用于应用程序启动时的URL处理）
     * @param delayMs 延迟时间（毫秒）
     */
    void setProcessingDelay(int delayMs);
    
    /**
     * @brief 获取最后处理的URL
     * @return 最后处理的URL
     */
    QString getLastProcessedUrl() const;

signals:
    /**
     * @brief 协议URL处理信号
     * @param meetingInfo 会议信息
     */
    void protocolUrlReceived(const ProtocolHandler::MeetingInfo& meetingInfo);
    
    /**
     * @brief 协议注册状态改变信号
     * @param registered 是否已注册
     */
    void protocolRegistrationChanged(bool registered);
    
    /**
     * @brief 无效URL接收信号
     * @param url 无效的URL
     * @param reason 无效原因
     */
    void invalidUrlReceived(const QString& url, const QString& reason);

public slots:
    /**
     * @brief 处理延迟的协议URL
     */
    void processDelayedUrl();

private slots:
    /**
     * @brief 处理应用程序激活事件
     */
    void onApplicationActivated();

private:
    /**
     * @brief 初始化协议处理器
     */
    void initialize();
    
    /**
     * @brief 处理URL的内部方法
     * @param url 要处理的URL
     * @return 是否处理成功
     */
    bool processUrl(const QString& url);
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 注册Windows协议
     * @return 是否成功
     */
    bool registerWindowsProtocol();
    
    /**
     * @brief 取消注册Windows协议
     * @return 是否成功
     */
    bool unregisterWindowsProtocol();
    
    /**
     * @brief 检查Windows协议注册状态
     * @return 是否已注册
     */
    bool isWindowsProtocolRegistered() const;
    
    /**
     * @brief 注册Linux协议
     * @return 是否成功
     */
    bool registerLinuxProtocol();
    
    /**
     * @brief 取消注册Linux协议
     * @return 是否成功
     */
    bool unregisterLinuxProtocol();
    
    /**
     * @brief 注册macOS协议
     * @return 是否成功
     */
    bool registerMacProtocol();
    
    /**
     * @brief 取消注册macOS协议
     * @return 是否成功
     */
    bool unregisterMacProtocol();
    
    /**
     * @brief 验证房间名称的有效性
     * @param roomName 房间名称
     * @return 是否有效
     */
    bool isValidRoomName(const QString& roomName) const;
    
    /**
     * @brief 验证服务器URL的有效性
     * @param serverUrl 服务器URL
     * @return 是否有效
     */
    bool isValidServerUrl(const QString& serverUrl) const;
    
    /**
     * @brief 标准化服务器URL
     * @param serverUrl 原始服务器URL
     * @return 标准化后的URL
     */
    QString normalizeServerUrl(const QString& serverUrl) const;
    
    /**
     * @brief 解析URL参数
     * @param url QUrl对象
     * @return 参数映射
     */
    QStringList parseUrlParameters(const QUrl& url) const;
    
    /**
     * @brief 记录协议处理日志
     * @param message 日志消息
     * @param url 相关URL
     */
    void logProtocolHandling(const QString& message, const QString& url = QString()) const;

private:
    MainApplication* m_app;                    ///< 主应用程序实例
    QTimer* m_delayTimer;                      ///< 延迟处理定时器
    QString m_pendingUrl;                      ///< 待处理的URL
    QString m_lastProcessedUrl;               ///< 最后处理的URL
    int m_processingDelay;                     ///< 处理延迟时间
    bool m_isRegistered;                       ///< 协议是否已注册
    mutable QMutex m_mutex;                    ///< 线程安全互斥锁
    
    // 协议相关常量
    static const QString PROTOCOL_NAME;       ///< 协议名称
    static const QString PROTOCOL_SCHEME;     ///< 协议方案
    static const QString DEFAULT_SERVER;      ///< 默认服务器
    
    // 正则表达式
    static const QRegularExpression ROOM_NAME_REGEX;     ///< 房间名称验证正则
    static const QRegularExpression SERVER_URL_REGEX;    ///< 服务器URL验证正则
    static const QRegularExpression PROTOCOL_URL_REGEX;  ///< 协议URL验证正则
};

// 声明元类型以便在信号槽中使用
Q_DECLARE_METATYPE(ProtocolHandler::MeetingInfo)

#endif // PROTOCOLHANDLER_H