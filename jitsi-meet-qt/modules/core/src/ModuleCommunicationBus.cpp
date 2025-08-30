#include "ModuleCommunicationBus.h"
#include <QDebug>
#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>
#include <QCoreApplication>
#include <QDateTime>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCompressor>
#include <QRegularExpression>

ModuleCommunicationBus* ModuleCommunicationBus::s_instance = nullptr;
QMutex ModuleCommunicationBus::s_mutex;

ModuleCommunicationBus* ModuleCommunicationBus::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ModuleCommunicationBus();
    }
    return s_instance;
}

ModuleCommunicationBus::ModuleCommunicationBus(QObject* parent)
    : QObject(parent)
    , m_maxQueueSize(10000)
    , m_batchSize(100)
    , m_processingInterval(10)  // 10ms
    , m_compressionEnabled(false)
    , m_messageTTL(300)  // 5 minutes
    , m_running(false)
    , m_paused(false)
    , m_processingTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_metricsTimer(new QTimer(this))
    , m_threadPool(new QThreadPool(this))
    , m_lastMetricsUpdate(0)
{
    initializeSystem();
}

ModuleCommunicationBus::~ModuleCommunicationBus()
{
    shutdownSystem();
}

void ModuleCommunicationBus::initializeSystem()
{
    // 初始化性能指标
    m_metrics = PerformanceMetrics();
    m_metrics.totalMessages = 0;
    m_metrics.processedMessages = 0;
    m_metrics.droppedMessages = 0;
    m_metrics.averageLatency = 0;
    m_metrics.peakLatency = 0;
    m_metrics.throughput = 0.0;
    m_metrics.queueSize = 0;
    m_metrics.memoryUsage = 0;

    // 配置定时器
    m_processingTimer->setSingleShot(false);
    m_processingTimer->setInterval(m_processingInterval);
    connect(m_processingTimer, &QTimer::timeout, this, &ModuleCommunicationBus::processMessageQueue);

    m_cleanupTimer->setSingleShot(false);
    m_cleanupTimer->setInterval(60000);  // 1分钟清理一次
    connect(m_cleanupTimer, &QTimer::timeout, this, &ModuleCommunicationBus::cleanupExpiredMessages);

    m_metricsTimer->setSingleShot(false);
    m_metricsTimer->setInterval(5000);  // 5秒更新一次指标
    connect(m_metricsTimer, &QTimer::timeout, this, &ModuleCommunicationBus::updatePerformanceMetrics);

    // 配置线程池
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());

    qDebug() << "ModuleCommunicationBus initialized";
}

void ModuleCommunicationBus::shutdownSystem()
{
    stop();
    
    // 等待所有任务完成
    m_threadPool->waitForDone(5000);
    
    // 清理资源
    clear();
    
    qDebug() << "ModuleCommunicationBus shutdown completed";
}

void ModuleCommunicationBus::start()
{
    if (m_running) {
        return;
    }

    m_running = true;
    m_paused = false;
    
    m_processingTimer->start();
    m_cleanupTimer->start();
    m_metricsTimer->start();
    
    qDebug() << "ModuleCommunicationBus started";
}

void ModuleCommunicationBus::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;
    
    m_processingTimer->stop();
    m_cleanupTimer->stop();
    m_metricsTimer->stop();
    
    // 处理剩余消息
    flush();
    
    qDebug() << "ModuleCommunicationBus stopped";
}

void ModuleCommunicationBus::pause()
{
    m_paused = true;
    qDebug() << "ModuleCommunicationBus paused";
}

void ModuleCommunicationBus::resume()
{
    m_paused = false;
    qDebug() << "ModuleCommunicationBus resumed";
}

bool ModuleCommunicationBus::sendMessage(const Message& message)
{
    if (!validateMessage(message)) {
        qWarning() << "Invalid message rejected:" << message.id;
        return false;
    }

    Message msg = message;
    if (msg.id.isEmpty()) {
        msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (msg.timestamp == 0) {
        msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    if (msg.expireTime == 0 && m_messageTTL > 0) {
        msg.expireTime = msg.timestamp + (m_messageTTL * 1000);
    }

    // 压缩大消息
    if (m_compressionEnabled) {
        compressPayload(msg);
    }

    enqueueMessage(msg);
    
    {
        QMutexLocker locker(&m_metricsLock);
        m_metrics.totalMessages++;
    }

    return true;
}

bool ModuleCommunicationBus::sendCommand(const QString& receiver, const QString& command, const QVariant& data)
{
    Message msg;
    msg.receiver = receiver;
    msg.type = Command;
    msg.priority = High;
    msg.payload = data;
    msg.metadata["command"] = command;
    
    return sendMessage(msg);
}

bool ModuleCommunicationBus::sendEvent(const QString& eventName, const QVariant& data)
{
    Message msg;
    msg.type = Event;
    msg.priority = Normal;
    msg.payload = data;
    msg.metadata["event"] = eventName;
    
    return sendMessage(msg);
}

bool ModuleCommunicationBus::sendRequest(const QString& receiver, const QString& request, const QVariant& data)
{
    Message msg;
    msg.receiver = receiver;
    msg.type = Request;
    msg.priority = High;
    msg.payload = data;
    msg.correlationId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.metadata["request"] = request;
    
    return sendMessage(msg);
}

bool ModuleCommunicationBus::sendResponse(const QString& correlationId, const QVariant& data)
{
    Message msg;
    msg.type = Response;
    msg.priority = High;
    msg.payload = data;
    msg.correlationId = correlationId;
    
    return sendMessage(msg);
}

bool ModuleCommunicationBus::broadcast(const QString& eventName, const QVariant& data)
{
    Message msg;
    msg.type = Broadcast;
    msg.priority = Normal;
    msg.payload = data;
    msg.metadata["event"] = eventName;
    
    return sendMessage(msg);
}

void ModuleCommunicationBus::sendMessageAsync(const Message& message)
{
    AsyncMessageTask* task = new AsyncMessageTask(this, message);
    m_threadPool->start(task);
}

void ModuleCommunicationBus::sendCommandAsync(const QString& receiver, const QString& command, const QVariant& data)
{
    Message msg;
    msg.receiver = receiver;
    msg.type = Command;
    msg.priority = High;
    msg.payload = data;
    msg.metadata["command"] = command;
    
    sendMessageAsync(msg);
}

void ModuleCommunicationBus::sendEventAsync(const QString& eventName, const QVariant& data)
{
    Message msg;
    msg.type = Event;
    msg.priority = Normal;
    msg.payload = data;
    msg.metadata["event"] = eventName;
    
    sendMessageAsync(msg);
}

bool ModuleCommunicationBus::sendBatch(const QList<Message>& messages)
{
    bool allSuccess = true;
    
    for (const Message& msg : messages) {
        if (!sendMessage(msg)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

void ModuleCommunicationBus::sendBatchAsync(const QList<Message>& messages)
{
    BatchMessageTask* task = new BatchMessageTask(this, messages);
    m_threadPool->start(task);
}

bool ModuleCommunicationBus::subscribe(const QString& moduleName, const QString& eventPattern)
{
    QWriteLocker locker(&m_subscriptionLock);
    
    if (!m_subscriptions.contains(moduleName)) {
        m_subscriptions[moduleName] = QStringList();
    }
    
    if (!m_subscriptions[moduleName].contains(eventPattern)) {
        m_subscriptions[moduleName].append(eventPattern);
        qDebug() << "Module" << moduleName << "subscribed to" << eventPattern;
        return true;
    }
    
    return false;
}

bool ModuleCommunicationBus::unsubscribe(const QString& moduleName, const QString& eventPattern)
{
    QWriteLocker locker(&m_subscriptionLock);
    
    if (m_subscriptions.contains(moduleName)) {
        bool removed = m_subscriptions[moduleName].removeAll(eventPattern) > 0;
        if (removed) {
            qDebug() << "Module" << moduleName << "unsubscribed from" << eventPattern;
        }
        return removed;
    }
    
    return false;
}

bool ModuleCommunicationBus::subscribeToAll(const QString& moduleName)
{
    return subscribe(moduleName, "*");
}

bool ModuleCommunicationBus::unsubscribeFromAll(const QString& moduleName)
{
    QWriteLocker locker(&m_subscriptionLock);
    
    if (m_subscriptions.contains(moduleName)) {
        m_subscriptions.remove(moduleName);
        qDebug() << "Module" << moduleName << "unsubscribed from all events";
        return true;
    }
    
    return false;
}

void ModuleCommunicationBus::addMessageFilter(const QString& filterId, std::function<bool(const Message&)> filter)
{
    QMutexLocker locker(&m_filterLock);
    m_messageFilters[filterId] = filter;
    qDebug() << "Message filter added:" << filterId;
}

void ModuleCommunicationBus::removeMessageFilter(const QString& filterId)
{
    QMutexLocker locker(&m_filterLock);
    if (m_messageFilters.remove(filterId) > 0) {
        qDebug() << "Message filter removed:" << filterId;
    }
}

void ModuleCommunicationBus::setMaxQueueSize(int size)
{
    m_maxQueueSize = qMax(100, size);
    qDebug() << "Max queue size set to:" << m_maxQueueSize;
}

void ModuleCommunicationBus::setBatchSize(int size)
{
    m_batchSize = qMax(1, qMin(1000, size));
    qDebug() << "Batch size set to:" << m_batchSize;
}

void ModuleCommunicationBus::setProcessingInterval(int milliseconds)
{
    m_processingInterval = qMax(1, milliseconds);
    m_processingTimer->setInterval(m_processingInterval);
    qDebug() << "Processing interval set to:" << m_processingInterval << "ms";
}

void ModuleCommunicationBus::setCompressionEnabled(bool enabled)
{
    m_compressionEnabled = enabled;
    qDebug() << "Compression" << (enabled ? "enabled" : "disabled");
}

void ModuleCommunicationBus::setMessageTTL(int seconds)
{
    m_messageTTL = qMax(0, seconds);
    qDebug() << "Message TTL set to:" << m_messageTTL << "seconds";
}

ModuleCommunicationBus::PerformanceMetrics ModuleCommunicationBus::getPerformanceMetrics() const
{
    QMutexLocker locker(&m_metricsLock);
    return m_metrics;
}

int ModuleCommunicationBus::getQueueSize() const
{
    QReadLocker locker(&m_queueLock);
    int totalSize = 0;
    for (auto it = m_messageQueues.begin(); it != m_messageQueues.end(); ++it) {
        totalSize += it.value().size();
    }
    return totalSize;
}

int ModuleCommunicationBus::getSubscriberCount() const
{
    QReadLocker locker(&m_subscriptionLock);
    return m_subscriptions.size();
}

QStringList ModuleCommunicationBus::getActiveModules() const
{
    QReadLocker locker(&m_subscriptionLock);
    return m_subscriptions.keys();
}

void ModuleCommunicationBus::flush()
{
    while (hasMessages()) {
        processMessageQueue();
        QCoreApplication::processEvents();
    }
}

void ModuleCommunicationBus::clear()
{
    QWriteLocker locker(&m_queueLock);
    m_messageQueues.clear();
    
    {
        QMutexLocker metricsLocker(&m_metricsLock);
        m_metrics.queueSize = 0;
    }
    
    emit queueSizeChanged(0);
}

bool ModuleCommunicationBus::validateMessage(const Message& message) const
{
    // 基本验证
    if (message.type == Command || message.type == Request) {
        if (message.receiver.isEmpty()) {
            return false;
        }
    }
    
    // 检查消息大小
    QJsonDocument doc = QJsonDocument::fromVariant(message.payload);
    if (doc.toJson().size() > 1024 * 1024) {  // 1MB limit
        qWarning() << "Message payload too large:" << doc.toJson().size();
        return false;
    }
    
    // 应用消息过滤器
    QMutexLocker locker(&m_filterLock);
    for (auto it = m_messageFilters.begin(); it != m_messageFilters.end(); ++it) {
        if (!it.value()(message)) {
            return false;
        }
    }
    
    return true;
}

void ModuleCommunicationBus::enqueueMessage(const Message& message)
{
    QWriteLocker locker(&m_queueLock);
    
    // 检查队列大小限制
    int currentSize = getQueueSize();
    if (currentSize >= m_maxQueueSize) {
        // 丢弃最老的低优先级消息
        for (int priority = Background; priority >= Low; --priority) {
            MessagePriority p = static_cast<MessagePriority>(priority);
            if (m_messageQueues.contains(p) && !m_messageQueues[p].isEmpty()) {
                m_messageQueues[p].dequeue();
                {
                    QMutexLocker metricsLocker(&m_metricsLock);
                    m_metrics.droppedMessages++;
                }
                break;
            }
        }
    }
    
    // 添加到对应优先级队列
    m_messageQueues[message.priority].enqueue(message);
    
    // 更新统计
    {
        QMutexLocker metricsLocker(&m_metricsLock);
        m_metrics.queueSize = getQueueSize();
    }
    
    emit queueSizeChanged(m_metrics.queueSize);
}

ModuleCommunicationBus::Message ModuleCommunicationBus::dequeueMessage()
{
    QWriteLocker locker(&m_queueLock);
    
    // 按优先级顺序处理消息
    for (int priority = Critical; priority <= Background; ++priority) {
        MessagePriority p = static_cast<MessagePriority>(priority);
        if (m_messageQueues.contains(p) && !m_messageQueues[p].isEmpty()) {
            Message msg = m_messageQueues[p].dequeue();
            
            {
                QMutexLocker metricsLocker(&m_metricsLock);
                m_metrics.queueSize = getQueueSize();
            }
            
            return msg;
        }
    }
    
    return Message();  // 空消息
}

bool ModuleCommunicationBus::hasMessages() const
{
    QReadLocker locker(&m_queueLock);
    for (auto it = m_messageQueues.begin(); it != m_messageQueues.end(); ++it) {
        if (!it.value().isEmpty()) {
            return true;
        }
    }
    return false;
}

void ModuleCommunicationBus::processMessageQueue()
{
    if (!m_running || m_paused) {
        return;
    }

    int processed = 0;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 批量处理消息
    while (hasMessages() && processed < m_batchSize) {
        Message msg = dequeueMessage();
        if (msg.id.isEmpty()) {
            break;  // 无效消息
        }
        
        // 检查消息是否过期
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (msg.expireTime > 0 && currentTime > msg.expireTime) {
            {
                QMutexLocker locker(&m_metricsLock);
                m_metrics.droppedMessages++;
            }
            continue;
        }
        
        processMessage(msg);
        processed++;
        
        // 计算延迟
        qint64 latency = currentTime - msg.timestamp;
        updateLatencyMetrics(latency);
    }
    
    // 更新处理统计
    if (processed > 0) {
        QMutexLocker locker(&m_metricsLock);
        m_metrics.processedMessages += processed;
        
        qint64 processingTime = QDateTime::currentMSecsSinceEpoch() - startTime;
        if (processingTime > 0) {
            m_metrics.throughput = (double)processed / (processingTime / 1000.0);
        }
    }
}

void ModuleCommunicationBus::processMessage(const Message& message)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    try {
        // 解压缩消息
        Message msg = message;
        if (m_compressionEnabled) {
            decompressPayload(msg);
        }
        
        // 根据消息类型处理
        switch (msg.type) {
        case Command:
        case Request:
        case Response:
            if (!msg.receiver.isEmpty()) {
                deliverMessage(msg.receiver, msg);
            }
            break;
            
        case Event:
        case Broadcast:
            broadcastMessage(msg);
            break;
            
        case Data:
            if (!msg.receiver.isEmpty()) {
                deliverMessage(msg.receiver, msg);
            } else {
                broadcastMessage(msg);
            }
            break;
        }
        
        emit messageProcessed(msg.id, true);
        
    } catch (const std::exception& e) {
        qWarning() << "Message processing failed:" << message.id << e.what();
        emit messageProcessed(message.id, false);
    }
    
    qint64 processingTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    updateLatencyMetrics(processingTime);
}

void ModuleCommunicationBus::deliverMessage(const QString& receiver, const Message& message)
{
    emit messageReceived(receiver, message);
}

void ModuleCommunicationBus::broadcastMessage(const Message& message)
{
    QString eventName = message.metadata.value("event").toString();
    if (eventName.isEmpty()) {
        return;
    }
    
    QStringList subscribers = getSubscribers(eventName);
    for (const QString& subscriber : subscribers) {
        emit messageReceived(subscriber, message);
    }
}

bool ModuleCommunicationBus::matchesPattern(const QString& eventName, const QString& pattern) const
{
    if (pattern == "*") {
        return true;
    }
    
    // 支持简单的通配符匹配
    QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
    return regex.match(eventName).hasMatch();
}

QStringList ModuleCommunicationBus::getSubscribers(const QString& eventName) const
{
    QReadLocker locker(&m_subscriptionLock);
    QStringList subscribers;
    
    for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
        const QString& moduleName = it.key();
        const QStringList& patterns = it.value();
        
        for (const QString& pattern : patterns) {
            if (matchesPattern(eventName, pattern)) {
                subscribers.append(moduleName);
                break;  // 避免重复添加
            }
        }
    }
    
    return subscribers;
}

void ModuleCommunicationBus::compressPayload(Message& message) const
{
    // 简化的压缩实现 - 实际项目中应使用专业的压缩库
    QJsonDocument doc = QJsonDocument::fromVariant(message.payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    
    if (data.size() > 1024) {  // 只压缩大于1KB的数据
        // 这里应该使用真正的压缩算法
        message.metadata["compressed"] = true;
        message.metadata["originalSize"] = data.size();
    }
}

void ModuleCommunicationBus::decompressPayload(Message& message) const
{
    if (message.metadata.value("compressed", false).toBool()) {
        // 这里应该使用对应的解压缩算法
        message.metadata.remove("compressed");
        message.metadata.remove("originalSize");
    }
}

void ModuleCommunicationBus::updateLatencyMetrics(qint64 latency)
{
    QMutexLocker locker(&m_metricsLock);
    
    m_latencyHistory.enqueue(latency);
    if (m_latencyHistory.size() > 1000) {  // 保持最近1000个样本
        m_latencyHistory.dequeue();
    }
    
    // 更新峰值延迟
    if (latency > m_metrics.peakLatency) {
        m_metrics.peakLatency = latency;
    }
    
    // 计算平均延迟
    qint64 total = 0;
    for (qint64 l : m_latencyHistory) {
        total += l;
    }
    m_metrics.averageLatency = total / m_latencyHistory.size();
}

void ModuleCommunicationBus::cleanupExpiredMessages()
{
    if (!m_running || m_messageTTL <= 0) {
        return;
    }
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    int cleaned = 0;
    
    QWriteLocker locker(&m_queueLock);
    
    for (auto& queue : m_messageQueues) {
        QQueue<Message> cleanQueue;
        
        while (!queue.isEmpty()) {
            Message msg = queue.dequeue();
            if (msg.expireTime == 0 || currentTime <= msg.expireTime) {
                cleanQueue.enqueue(msg);
            } else {
                cleaned++;
            }
        }
        
        queue = cleanQueue;
    }
    
    if (cleaned > 0) {
        QMutexLocker metricsLocker(&m_metricsLock);
        m_metrics.droppedMessages += cleaned;
        m_metrics.queueSize = getQueueSize();
        
        qDebug() << "Cleaned up" << cleaned << "expired messages";
    }
}

void ModuleCommunicationBus::updatePerformanceMetrics()
{
    QMutexLocker locker(&m_metricsLock);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 计算吞吐量
    if (m_lastMetricsUpdate > 0) {
        qint64 timeDiff = currentTime - m_lastMetricsUpdate;
        if (timeDiff > 0) {
            double messagesPerSecond = (double)m_metrics.processedMessages / (timeDiff / 1000.0);
            m_metrics.throughput = messagesPerSecond;
        }
    }
    
    m_lastMetricsUpdate = currentTime;
    
    // 检查性能阈值
    checkPerformanceThresholds();
}

void ModuleCommunicationBus::checkPerformanceThresholds()
{
    // 检查队列大小
    if (m_metrics.queueSize > m_maxQueueSize * 0.8) {
        emit performanceAlert(QString("Queue size approaching limit: %1/%2")
                             .arg(m_metrics.queueSize).arg(m_maxQueueSize));
    }
    
    // 检查延迟
    if (m_metrics.averageLatency > 1000) {  // 1秒
        emit performanceAlert(QString("High average latency: %1ms").arg(m_metrics.averageLatency));
    }
    
    // 检查丢包率
    if (m_metrics.totalMessages > 0) {
        double dropRate = (double)m_metrics.droppedMessages / m_metrics.totalMessages;
        if (dropRate > 0.05) {  // 5%
            emit performanceAlert(QString("High message drop rate: %1%")
                                 .arg(dropRate * 100, 0, 'f', 2));
        }
    }
}

// AsyncMessageTask 实现
AsyncMessageTask::AsyncMessageTask(ModuleCommunicationBus* bus, const ModuleCommunicationBus::Message& message)
    : m_bus(bus)
    , m_message(message)
{
    setAutoDelete(true);
}

void AsyncMessageTask::run()
{
    if (auto bus = m_bus.lock()) {
        bus->sendMessage(m_message);
    }
}

// BatchMessageTask 实现
BatchMessageTask::BatchMessageTask(ModuleCommunicationBus* bus, const QList<ModuleCommunicationBus::Message>& messages)
    : m_bus(bus)
    , m_messages(messages)
{
    setAutoDelete(true);
}

void BatchMessageTask::run()
{
    if (auto bus = m_bus.lock()) {
        bus->sendBatch(m_messages);
    }
}