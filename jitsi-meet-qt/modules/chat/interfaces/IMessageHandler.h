#ifndef IMESSAGEHANDLER_H
#define IMESSAGEHANDLER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <functional>

// Forward declarations
class ChatMessage;
class IMessageStorage;

/**
 * @brief 消息处理器接口
 * 
 * IMessageHandler定义了消息处理器的标准接口，
 * 提供消息处理功能的抽象定义。
 */
class IMessageHandler
{

public:
    /**
     * @brief 消息处理状态枚举
     */
    enum ProcessingStatus {
        Idle,               ///< 空闲
        Processing,         ///< 处理中
        Paused,             ///< 暂停
        Error               ///< 错误
    };

    /**
     * @brief 消息优先级枚举
     */
    enum MessagePriority {
        Low = 0,            ///< 低优先级
        Normal = 1,         ///< 普通优先级
        High = 2,           ///< 高优先级
        Critical = 3        ///< 关键优先级
    };

    /**
     * @brief 处理结果枚举
     */
    enum ProcessingResult {
        Success,            ///< 成功
        Failed,             ///< 失败
        Filtered,           ///< 被过滤
        Queued,             ///< 已排队
        Rejected            ///< 被拒绝
    };

    /**
     * @brief 虚析构函数
     */
    virtual ~IMessageHandler() = default;

    /**
     * @brief 初始化消息处理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief 处理传入消息
     * @param data 消息数据
     * @param priority 消息优先级
     * @return 处理结果
     */
    virtual ProcessingResult processIncomingMessage(const QVariantMap& data, MessagePriority priority = Normal) = 0;

    /**
     * @brief 处理传出消息
     * @param message 消息对象
     * @param priority 消息优先级
     * @return 处理结果
     */
    virtual ProcessingResult processOutgoingMessage(ChatMessage* message, MessagePriority priority = Normal) = 0;

    /**
     * @brief 验证消息格式
     * @param data 消息数据
     * @return 验证是否通过
     */
    virtual bool validateMessage(const QVariantMap& data) const = 0;

    /**
     * @brief 格式化消息
     * @param message 消息对象
     * @return 格式化后的数据
     */
    virtual QVariantMap formatMessage(ChatMessage* message) const = 0;

    /**
     * @brief 解析消息数据
     * @param data 原始数据
     * @return 解析后的消息对象
     */
    virtual ChatMessage* parseMessage(const QVariantMap& data) const = 0;

    /**
     * @brief 检查处理是否启用
     * @return 是否启用处理
     */
    virtual bool isProcessingEnabled() const = 0;

    /**
     * @brief 设置处理启用状态
     * @param enabled 是否启用
     */
    virtual void setProcessingEnabled(bool enabled) = 0;

    /**
     * @brief 获取处理状态
     * @return 处理状态
     */
    virtual ProcessingStatus processingStatus() const = 0;

    /**
     * @brief 获取队列大小
     * @return 队列中的消息数量
     */
    virtual int queueSize() const = 0;

    /**
     * @brief 获取已处理消息数量
     * @return 已处理的消息数量
     */
    virtual int processedCount() const = 0;

    /**
     * @brief 获取处理统计信息
     * @return 统计信息
     */
    virtual QVariantMap getStatistics() const = 0;

    /**
     * @brief 设置消息存储
     * @param storage 存储对象
     */
    virtual void setMessageStorage(IMessageStorage* storage) = 0;

    /**
     * @brief 获取消息存储
     * @return 存储对象
     */
    virtual IMessageStorage* messageStorage() const = 0;

    /**
     * @brief 设置消息过滤器
     * @param filter 过滤器函数
     */
    virtual void setMessageFilter(std::function<bool(const QVariantMap&)> filter) = 0;

    /**
     * @brief 设置消息转换器
     * @param transformer 转换器函数
     */
    virtual void setMessageTransformer(std::function<QVariantMap(const QVariantMap&)> transformer) = 0;

    /**
     * @brief 添加消息处理器
     * @param processor 处理器函数
     */
    virtual void addMessageProcessor(std::function<void(ChatMessage*)> processor) = 0;

    /**
     * @brief 清除消息队列
     */
    virtual void clearQueue() = 0;

    /**
     * @brief 获取队列中的消息
     * @return 队列中的消息列表
     */
    virtual QList<QVariantMap> getQueuedMessages() const = 0;

    /**
     * @brief 开始处理
     */
    virtual void startProcessing() = 0;

    /**
     * @brief 停止处理
     */
    virtual void stopProcessing() = 0;

    /**
     * @brief 暂停处理
     */
    virtual void pauseProcessing() = 0;

    /**
     * @brief 恢复处理
     */
    virtual void resumeProcessing() = 0;

    /**
     * @brief 处理队列中的消息
     */
    virtual void processQueue() = 0;

    /**
     * @brief 重试失败的消息
     */
    virtual void retryFailedMessages() = 0;

    /**
     * @brief 清除统计信息
     */
    virtual void clearStatistics() = 0;

// 信号接口（由实现类提供）
    // 消息处理相关
    // void messageProcessed(ChatMessage* message, ProcessingResult result);
    // void messageValidationFailed(const QVariantMap& data, const QString& reason);
    // void messageFiltered(const QVariantMap& data);
    
    // 状态相关
    // void processingEnabledChanged(bool enabled);
    // void processingStatusChanged(ProcessingStatus status);
    // void queueSizeChanged(int size);
    // void processedCountChanged(int count);
    
    // 错误和通知
    // void processingError(const QString& error);
    // void queueFull();
    // void queueEmpty();
};

Q_DECLARE_INTERFACE(IMessageHandler, "org.jitsi.chat.IMessageHandler/1.0")

#endif // IMESSAGEHANDLER_H