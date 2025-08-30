#ifndef MODULECOMMUNICATIONBUS_H
#define MODULECOMMUNICATIONBUS_H

#include <QObject>
#include <QVariant>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QHash>
#include <QReadWriteLock>

/**
 * @brief 模块间通信总线 - 优化的异步通信系统
 * 
 * 提供高性能的模块间通信，支持：
 * - 异步消息传递
 * - 事件广播
 * - 数据缓存和共享
 * - 优先级队列
 * - 批量处理
 */
class ModuleCommunicationBus : public QObject
{
    Q_OBJECT

public:
    enum MessagePriority {
        Critical = 0,    // 关键消息，立即处理
        High = 1,        // 高优先级
        Normal = 2,      // 普通优先级
        Low = 3,         // 低优先级
        Background = 4   // 后台处理
    };

    enum MessageType {
        Command,         // 命令消息
        Event,           // 事件消息
        Data,            // 数据消息
        Request,         // 请求消息
        Response,        // 响应消息
        Broadcast        // 广播消息
    };

    struct Message {
        QString id;
        QString sender;
        QString receiver;
        MessageType type;
        MessagePriority priority;
        QVariant payload;
        QVariantMap metadata;
        qint64 timestamp;
        qint64 expireTime;
        QString correlationId;  // 用于请求-响应关联
        
        Message() : type(Event), priority(Normal), timestamp(0), expireTime(0) {}
    };

    struct PerformanceMetrics {
        qint64 totalMessages;
        qint64 processedMessages;
        qint64 droppedMessages;
        qint64 averageLatency;
        qint64 peakLatency;
        double throughput;  // messages per second
        qint64 queueSize;
        qint64 memoryUsage;
    };

    static ModuleCommunicationBus* instance();
    ~ModuleCommunicationBus();

    // 消息发送接口
    bool sendMessage(const Message& message);
    bool sendCommand(const QString& receiver, const QString& command, const QVariant& data = QVariant());
    bool sendEvent(const QString& eventName, const QVariant& data = QVariant());
    bool sendRequest(const QString& receiver, const QString& request, const QVariant& data = QVariant());
    bool sendResponse(const QString& correlationId, const QVariant& data = QVariant());
    bool broadcast(const QString& eventName, const QVariant& data = QVariant());

    // 异步发送接口
    void sendMessageAsync(const Message& message);
    void sendCommandAsync(const QString& receiver, const QString& command, const QVariant& data = QVariant());
    void sendEventAsync(const QString& eventName, const QVariant& data = QVariant());

    // 批量发送接口
    bool sendBatch(const QList<Message>& messages);
    void sendBatchAsync(const QList<Message>& messages);

    // 订阅和取消订阅
    bool subscribe(const QString& moduleName, const QString& eventPattern);
    bool unsubscribe(const QString& moduleName, const QString& eventPattern);
    bool subscribeToAll(const QString& moduleName);
    bool unsubscribeFromAll(const QString& moduleName);

    // 消息过滤器
    void addMessageFilter(const QString& filterId, std::function<bool(const Message&)> filter);
    void removeMessageFilter(const QString& filterId);

    // 性能优化控制
    void setMaxQueueSize(int size);
    void setBatchSize(int size);
    void setProcessingInterval(int milliseconds);
    void setCompressionEnabled(bool enabled);
    void setMessageTTL(int seconds);

    // 统计和监控
    PerformanceMetrics getPerformanceMetrics() const;
    int getQueueSize() const;
    int getSubscriberCount() const;
    QStringList getActiveModules() const;

    // 系统控制
    void start();
    void stop();
    void pause();
    void resume();
    void flush();  // 强制处理所有待处理消息
    void clear();  // 清空消息队列

signals:
    void messageReceived(const QString& moduleName, const ModuleCommunicationBus::Message& message);
    void messageProcessed(const QString& messageId, bool success);
    void queueSizeChanged(int size);
    void performanceAlert(const QString& alert);

private slots:
    void processMessageQueue();
    void cleanupExpiredMessages();
    void updatePerformanceMetrics();

private:
    explicit ModuleCommunicationBus(QObject* parent = nullptr);
    
    void initializeSystem();
    void shutdownSystem();
    
    bool validateMessage(const Message& message) const;
    void enqueueMessage(const Message& message);
    Message dequeueMessage();
    bool hasMessages() const;
    
    void processMessage(const Message& message);
    void deliverMessage(const QString& receiver, const Message& message);
    void broadcastMessage(const Message& message);
    
    bool matchesPattern(const QString& eventName, const QString& pattern) const;
    QStringList getSubscribers(const QString& eventName) const;
    
    void compressPayload(Message& message) const;
    void decompressPayload(Message& message) const;
    
    void updateLatencyMetrics(qint64 latency);
    void checkPerformanceThresholds();

    static ModuleCommunicationBus* s_instance;
    static QMutex s_mutex;

    // 消息队列 - 按优先级分组
    QHash<MessagePriority, QQueue<Message>> m_messageQueues;
    mutable QReadWriteLock m_queueLock;

    // 订阅管理
    QHash<QString, QStringList> m_subscriptions;  // moduleName -> eventPatterns
    mutable QReadWriteLock m_subscriptionLock;

    // 消息过滤器
    QHash<QString, std::function<bool(const Message&)>> m_messageFilters;
    mutable QMutex m_filterLock;

    // 性能配置
    int m_maxQueueSize;
    int m_batchSize;
    int m_processingInterval;
    bool m_compressionEnabled;
    int m_messageTTL;

    // 系统状态
    bool m_running;
    bool m_paused;
    QTimer* m_processingTimer;
    QTimer* m_cleanupTimer;
    QTimer* m_metricsTimer;
    QThreadPool* m_threadPool;

    // 性能统计
    mutable PerformanceMetrics m_metrics;
    mutable QMutex m_metricsLock;
    QQueue<qint64> m_latencyHistory;
    qint64 m_lastMetricsUpdate;
};

/**
 * @brief 异步消息处理任务
 */
class AsyncMessageTask : public QRunnable
{
public:
    AsyncMessageTask(ModuleCommunicationBus* bus, const ModuleCommunicationBus::Message& message);
    void run() override;

private:
    QWeakPointer<ModuleCommunicationBus> m_bus;
    ModuleCommunicationBus::Message m_message;
};

/**
 * @brief 批量消息处理任务
 */
class BatchMessageTask : public QRunnable
{
public:
    BatchMessageTask(ModuleCommunicationBus* bus, const QList<ModuleCommunicationBus::Message>& messages);
    void run() override;

private:
    QWeakPointer<ModuleCommunicationBus> m_bus;
    QList<ModuleCommunicationBus::Message> m_messages;
};

#endif // MODULECOMMUNICATIONBUS_H