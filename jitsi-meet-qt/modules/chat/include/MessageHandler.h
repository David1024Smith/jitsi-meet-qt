#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <QDateTime>
#include <QQueue>
#include <memory>
#include "IMessageHandler.h"

// Forward declarations
class ChatMessage;
class IMessageStorage;

/**
 * @brief 消息处理器类
 * 
 * MessageHandler负责消息的处理、验证、格式化和路由，
 * 提供消息的底层处理功能。
 */
class MessageHandler : public QObject, public IMessageHandler
{
    Q_OBJECT
    Q_INTERFACES(IMessageHandler)
    Q_PROPERTY(bool processingEnabled READ isProcessingEnabled WRITE setProcessingEnabled NOTIFY processingEnabledChanged)
    Q_PROPERTY(int queueSize READ queueSize NOTIFY queueSizeChanged)
    Q_PROPERTY(int processedCount READ processedCount NOTIFY processedCountChanged)

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
    Q_ENUM(ProcessingStatus)

    /**
     * @brief 消息优先级枚举
     */
    enum MessagePriority {
        Low = 0,            ///< 低优先级
        Normal = 1,         ///< 普通优先级
        High = 2,           ///< 高优先级
        Critical = 3        ///< 关键优先级
    };
    Q_ENUM(MessagePriority)

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
    Q_ENUM(ProcessingResult)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MessageHandler(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MessageHandler();

    /**
     * @brief 初始化消息处理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap()) override;

    /**
     * @brief 处理传入消息
     * @param data 消息数据
     * @param priority 消息优先级
     * @return 处理结果
     */
    IMessageHandler::ProcessingResult processIncomingMessage(const QVariantMap& data, IMessageHandler::MessagePriority priority = IMessageHandler::Normal) override;

    /**
     * @brief 处理传出消息
     * @param message 消息对象
     * @param priority 消息优先级
     * @return 处理结果
     */
    IMessageHandler::ProcessingResult processOutgoingMessage(ChatMessage* message, IMessageHandler::MessagePriority priority = IMessageHandler::Normal) override;

    /**
     * @brief 验证消息格式
     * @param data 消息数据
     * @return 验证是否通过
     */
    bool validateMessage(const QVariantMap& data) const override;

    /**
     * @brief 格式化消息
     * @param message 消息对象
     * @return 格式化后的数据
     */
    QVariantMap formatMessage(ChatMessage* message) const override;

    /**
     * @brief 解析消息数据
     * @param data 原始数据
     * @return 解析后的消息对象
     */
    ChatMessage* parseMessage(const QVariantMap& data) const override;

    /**
     * @brief 检查处理是否启用
     * @return 是否启用处理
     */
    bool isProcessingEnabled() const override;

    /**
     * @brief 设置处理启用状态
     * @param enabled 是否启用
     */
    void setProcessingEnabled(bool enabled) override;

    /**
     * @brief 获取处理状态
     * @return 处理状态
     */
    IMessageHandler::ProcessingStatus processingStatus() const override;

    /**
     * @brief 获取队列大小
     * @return 队列中的消息数量
     */
    int queueSize() const override;

    /**
     * @brief 获取已处理消息数量
     * @return 已处理的消息数量
     */
    int processedCount() const override;

    /**
     * @brief 获取处理统计信息
     * @return 统计信息
     */
    QVariantMap getStatistics() const override;

    /**
     * @brief 设置消息存储
     * @param storage 存储对象
     */
    void setMessageStorage(IMessageStorage* storage) override;

    /**
     * @brief 获取消息存储
     * @return 存储对象
     */
    IMessageStorage* messageStorage() const override;

    /**
     * @brief 设置消息过滤器
     * @param filter 过滤器函数
     */
    void setMessageFilter(std::function<bool(const QVariantMap&)> filter) override;

    /**
     * @brief 设置消息转换器
     * @param transformer 转换器函数
     */
    void setMessageTransformer(std::function<QVariantMap(const QVariantMap&)> transformer) override;

    /**
     * @brief 添加消息处理器
     * @param processor 处理器函数
     */
    void addMessageProcessor(std::function<void(ChatMessage*)> processor) override;

    /**
     * @brief 清除消息队列
     */
    void clearQueue() override;

    /**
     * @brief 获取队列中的消息
     * @return 队列中的消息列表
     */
    QList<QVariantMap> getQueuedMessages() const override;

public slots:
    /**
     * @brief 开始处理
     */
    void startProcessing() override;

    /**
     * @brief 停止处理
     */
    void stopProcessing() override;

    /**
     * @brief 暂停处理
     */
    void pauseProcessing() override;

    /**
     * @brief 恢复处理
     */
    void resumeProcessing() override;

    /**
     * @brief 处理队列中的消息
     */
    void processQueue() override;

    /**
     * @brief 重试失败的消息
     */
    void retryFailedMessages() override;

    /**
     * @brief 清除统计信息
     */
    void clearStatistics() override;

signals:
    /**
     * @brief 消息处理完成信号
     * @param message 处理完成的消息
     * @param result 处理结果
     */
    void messageProcessed(ChatMessage* message, ProcessingResult result);

    /**
     * @brief 消息验证失败信号
     * @param data 消息数据
     * @param reason 失败原因
     */
    void messageValidationFailed(const QVariantMap& data, const QString& reason);

    /**
     * @brief 消息过滤信号
     * @param data 被过滤的消息数据
     */
    void messageFiltered(const QVariantMap& data);

    /**
     * @brief 处理启用状态改变信号
     * @param enabled 是否启用
     */
    void processingEnabledChanged(bool enabled);

    /**
     * @brief 处理状态改变信号
     * @param status 新状态
     */
    void processingStatusChanged(ProcessingStatus status);

    /**
     * @brief 队列大小改变信号
     * @param size 新的队列大小
     */
    void queueSizeChanged(int size);

    /**
     * @brief 已处理数量改变信号
     * @param count 新的已处理数量
     */
    void processedCountChanged(int count);

    /**
     * @brief 处理错误信号
     * @param error 错误信息
     */
    void processingError(const QString& error);

    /**
     * @brief 队列已满信号
     */
    void queueFull();

    /**
     * @brief 队列为空信号
     */
    void queueEmpty();

private slots:
    /**
     * @brief 处理下一个消息
     */
    void processNextMessage();

    /**
     * @brief 处理超时消息
     */
    void handleTimeoutMessages();

private:
    /**
     * @brief 内部处理消息
     * @param data 消息数据
     * @param priority 优先级
     * @return 处理结果
     */
    ProcessingResult internalProcessMessage(const QVariantMap& data, MessagePriority priority);

    /**
     * @brief 验证消息内容
     * @param data 消息数据
     * @return 验证结果和错误信息
     */
    QPair<bool, QString> validateMessageContent(const QVariantMap& data) const;

    /**
     * @brief 应用消息过滤器
     * @param data 消息数据
     * @return 是否通过过滤
     */
    bool applyMessageFilter(const QVariantMap& data) const;

    /**
     * @brief 应用消息转换器
     * @param data 消息数据
     * @return 转换后的数据
     */
    QVariantMap applyMessageTransformer(const QVariantMap& data) const;

    /**
     * @brief 设置处理状态
     * @param status 新状态
     */
    void setProcessingStatus(IMessageHandler::ProcessingStatus status);

    /**
     * @brief 更新统计信息
     * @param result 处理结果
     */
    void updateStatistics(IMessageHandler::ProcessingResult result);
    
    /**
     * @brief 内部处理消息
     * @param data 消息数据
     * @param priority 优先级
     * @return 处理结果
     */
    IMessageHandler::ProcessingResult internalProcessMessage(const QVariantMap& data, IMessageHandler::MessagePriority priority);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // MESSAGEHANDLER_H