#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

/**
 * @brief 协议处理器类
 * 
 * 处理各种会议协议的注册、解析和调用
 */
class ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 协议类型枚举
     */
    enum ProtocolType {
        JitsiProtocol,   ///< Jitsi协议 (jitsi://)
        MeetProtocol,    ///< Meet协议 (meet://)
        ConferenceProtocol, ///< Conference协议 (conference://)
        CustomProtocol   ///< 自定义协议
    };
    Q_ENUM(ProtocolType)

    /**
     * @brief 注册状态枚举
     */
    enum RegistrationStatus {
        NotRegistered,   ///< 未注册
        Registered,      ///< 已注册
        RegistrationFailed, ///< 注册失败
        PermissionDenied ///< 权限拒绝
    };
    Q_ENUM(RegistrationStatus)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ProtocolHandler(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ProtocolHandler();

    /**
     * @brief 注册协议处理器
     * @param protocol 协议名称
     * @param applicationPath 应用程序路径
     * @return 注册是否成功
     */
    bool registerProtocol(const QString& protocol, const QString& applicationPath = QString());

    /**
     * @brief 取消注册协议处理器
     * @param protocol 协议名称
     * @return 取消注册是否成功
     */
    bool unregisterProtocol(const QString& protocol);

    /**
     * @brief 检查协议是否已注册
     * @param protocol 协议名称
     * @return 是否已注册
     */
    bool isProtocolRegistered(const QString& protocol);

    /**
     * @brief 获取协议注册状态
     * @param protocol 协议名称
     * @return 注册状态
     */
    RegistrationStatus getRegistrationStatus(const QString& protocol);

    /**
     * @brief 处理协议调用
     * @param protocolUrl 协议URL
     * @return 处理结果映射
     */
    QVariantMap handleProtocolCall(const QString& protocolUrl);

    /**
     * @brief 解析协议URL
     * @param protocolUrl 协议URL
     * @return 解析结果映射
     */
    QVariantMap parseProtocolUrl(const QString& protocolUrl);

    /**
     * @brief 验证协议URL
     * @param protocolUrl 协议URL
     * @return 是否有效
     */
    bool validateProtocolUrl(const QString& protocolUrl);

    /**
     * @brief 获取支持的协议列表
     * @return 协议列表
     */
    QStringList getSupportedProtocols() const;

    /**
     * @brief 添加自定义协议
     * @param protocol 协议名称
     * @param description 协议描述
     * @return 添加是否成功
     */
    bool addCustomProtocol(const QString& protocol, const QString& description);

    /**
     * @brief 移除自定义协议
     * @param protocol 协议名称
     * @return 移除是否成功
     */
    bool removeCustomProtocol(const QString& protocol);

    /**
     * @brief 设置默认协议
     * @param protocol 协议名称
     */
    void setDefaultProtocol(const QString& protocol);

    /**
     * @brief 获取默认协议
     * @return 协议名称
     */
    QString defaultProtocol() const;

    /**
     * @brief 构建协议URL
     * @param protocol 协议名称
     * @param server 服务器地址
     * @param roomName 房间名称
     * @param parameters 参数映射
     * @return 协议URL
     */
    QString buildProtocolUrl(const QString& protocol,
                           const QString& server,
                           const QString& roomName,
                           const QVariantMap& parameters = QVariantMap());

    /**
     * @brief 转换协议URL为标准URL
     * @param protocolUrl 协议URL
     * @return 标准URL
     */
    QString convertToStandardUrl(const QString& protocolUrl);

    /**
     * @brief 转换标准URL为协议URL
     * @param standardUrl 标准URL
     * @param protocol 目标协议
     * @return 协议URL
     */
    QString convertToProtocolUrl(const QString& standardUrl, const QString& protocol);

    /**
     * @brief 获取协议信息
     * @param protocol 协议名称
     * @return 协议信息映射
     */
    QVariantMap getProtocolInfo(const QString& protocol);

    /**
     * @brief 设置协议处理器为默认
     * @param protocol 协议名称
     * @return 设置是否成功
     */
    bool setAsDefaultHandler(const QString& protocol);

    /**
     * @brief 检查是否为默认处理器
     * @param protocol 协议名称
     * @return 是否为默认处理器
     */
    bool isDefaultHandler(const QString& protocol);

    /**
     * @brief 启用协议处理
     * @param enabled 是否启用
     */
    void setProtocolHandlingEnabled(bool enabled);

    /**
     * @brief 获取协议处理是否启用
     * @return 是否启用
     */
    bool isProtocolHandlingEnabled() const;

public slots:
    /**
     * @brief 刷新协议注册状态
     */
    void refreshRegistrationStatus();

    /**
     * @brief 重新注册所有协议
     */
    void reregisterAllProtocols();

signals:
    /**
     * @brief 协议注册状态改变信号
     * @param protocol 协议名称
     * @param status 新状态
     */
    void registrationStatusChanged(const QString& protocol, RegistrationStatus status);

    /**
     * @brief 协议调用信号
     * @param protocolUrl 协议URL
     * @param parameters 解析的参数
     */
    void protocolCalled(const QString& protocolUrl, const QVariantMap& parameters);

    /**
     * @brief 协议处理完成信号
     * @param protocolUrl 协议URL
     * @param success 是否成功
     */
    void protocolHandled(const QString& protocolUrl, bool success);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief 初始化支持的协议
     */
    void initializeSupportedProtocols();

    /**
     * @brief 在Windows注册表中注册协议
     * @param protocol 协议名称
     * @param applicationPath 应用程序路径
     * @return 注册是否成功
     */
    bool registerProtocolWindows(const QString& protocol, const QString& applicationPath);

    /**
     * @brief 在Linux中注册协议
     * @param protocol 协议名称
     * @param applicationPath 应用程序路径
     * @return 注册是否成功
     */
    bool registerProtocolLinux(const QString& protocol, const QString& applicationPath);

    /**
     * @brief 在macOS中注册协议
     * @param protocol 协议名称
     * @param applicationPath 应用程序路径
     * @return 注册是否成功
     */
    bool registerProtocolMacOS(const QString& protocol, const QString& applicationPath);

    /**
     * @brief 检查协议注册状态（Windows）
     * @param protocol 协议名称
     * @return 注册状态
     */
    RegistrationStatus checkRegistrationWindows(const QString& protocol);

    /**
     * @brief 检查协议注册状态（Linux）
     * @param protocol 协议名称
     * @return 注册状态
     */
    RegistrationStatus checkRegistrationLinux(const QString& protocol);

    /**
     * @brief 检查协议注册状态（macOS）
     * @param protocol 协议名称
     * @return 注册状态
     */
    RegistrationStatus checkRegistrationMacOS(const QString& protocol);

    /**
     * @brief 解析Jitsi协议URL
     * @param url 协议URL
     * @return 解析结果
     */
    QVariantMap parseJitsiProtocol(const QString& url);

    /**
     * @brief 解析Meet协议URL
     * @param url 协议URL
     * @return 解析结果
     */
    QVariantMap parseMeetProtocol(const QString& url);

    /**
     * @brief 解析Conference协议URL
     * @param url 协议URL
     * @return 解析结果
     */
    QVariantMap parseConferenceProtocol(const QString& url);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // PROTOCOLHANDLER_H