#ifndef IPROTOCOLHANDLER_H
#define IPROTOCOLHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QByteArray>

/**
 * @brief 协议处理器接口
 * 
 * IProtocolHandler定义了网络协议处理的标准接口，提供协议解析、
 * 消息编码解码和协议状态管理的抽象方法。
 */
class IProtocolHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 协议状态枚举
     */
    enum ProtocolStatus {
        Inactive,           ///< 未激活
        Initializing,       ///< 初始化中
        Active,             ///< 激活状态
        Error,              ///< 错误状态
        Shutdown            ///< 已关闭
    };
    Q_ENUM(ProtocolStatus)

    /**
     * @brief 消息类型枚举
     */
    enum MessageType {
        Control,            ///< 控制消息
        Data,               ///< 数据消息
        Heartbeat,          ///< 心跳消息
        Error,              ///< 错误消息
        Custom              ///< 自定义消息
    };
    Q_ENUM(MessageType)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit IProtocolHandler(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~IProtocolHandler() = default;

    /**
     * @brief 初始化协议处理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief 启动协议处理
     * @return 启动是否成功
     */
    virtual bool start() = 0;

    /**
     * @brief 停止协议处理
     */
    virtual void stop() = 0;

    /**
     * @brief 获取协议状态
     * @return 当前协议状态
     */
    virtual ProtocolStatus protocolStatus() const = 0;

    /**
     * @brief 获取协议名称
     * @return 协议名称
     */
    virtual QString protocolName() const = 0;

    /**
     * @brief 获取协议版本
     * @return 协议版本
     */
    virtual QString protocolVersion() const = 0;

    /**
     * @brief 编码消息
     * @param type 消息类型
     * @param data 消息数据
     * @return 编码后的数据
     */
    virtual QByteArray encodeMessage(MessageType type, const QVariantMap& data) = 0;

    /**
     * @brief 解码消息
     * @param rawData 原始数据
     * @param type 输出消息类型
     * @param data 输出消息数据
     * @return 解码是否成功
     */
    virtual bool decodeMessage(const QByteArray& rawData, MessageType& type, QVariantMap& data) = 0;

    /**
     * @brief 处理接收到的数据
     * @param data 接收到的数据
     * @return 处理是否成功
     */
    virtual bool handleReceivedData(const QByteArray& data) = 0;

    /**
     * @brief 发送消息
     * @param type 消息类型
     * @param data 消息数据
     * @return 发送是否成功
     */
    virtual bool sendMessage(MessageType type, const QVariantMap& data) = 0;

    /**
     * @brief 发送心跳消息
     * @return 发送是否成功
     */
    virtual bool sendHeartbeat() = 0;

    /**
     * @brief 检查协议是否支持特定功能
     * @param feature 功能名称
     * @return 是否支持
     */
    virtual bool supportsFeature(const QString& feature) const = 0;

    /**
     * @brief 获取支持的功能列表
     * @return 功能列表
     */
    virtual QStringList supportedFeatures() const = 0;

    /**
     * @brief 设置协议参数
     * @param key 参数键
     * @param value 参数值
     */
    virtual void setParameter(const QString& key, const QVariant& value) = 0;

    /**
     * @brief 获取协议参数
     * @param key 参数键
     * @return 参数值
     */
    virtual QVariant parameter(const QString& key) const = 0;

    /**
     * @brief 获取协议统计信息
     * @return 统计信息映射
     */
    virtual QVariantMap protocolStats() const = 0;

public slots:
    /**
     * @brief 重置协议状态
     */
    virtual void reset() = 0;

    /**
     * @brief 刷新协议状态
     */
    virtual void refresh() = 0;

signals:
    /**
     * @brief 协议状态改变信号
     * @param status 新的协议状态
     */
    void protocolStatusChanged(ProtocolStatus status);

    /**
     * @brief 消息接收信号
     * @param type 消息类型
     * @param data 消息数据
     */
    void messageReceived(MessageType type, const QVariantMap& data);

    /**
     * @brief 消息发送信号
     * @param type 消息类型
     * @param data 消息数据
     */
    void messageSent(MessageType type, const QVariantMap& data);

    /**
     * @brief 心跳接收信号
     */
    void heartbeatReceived();

    /**
     * @brief 心跳发送信号
     */
    void heartbeatSent();

    /**
     * @brief 协议错误信号
     * @param error 错误信息
     */
    void protocolError(const QString& error);

    /**
     * @brief 协议启动信号
     */
    void protocolStarted();

    /**
     * @brief 协议停止信号
     */
    void protocolStopped();

    /**
     * @brief 协议统计更新信号
     * @param stats 统计信息
     */
    void statsUpdated(const QVariantMap& stats);
};

#endif // IPROTOCOLHANDLER_H