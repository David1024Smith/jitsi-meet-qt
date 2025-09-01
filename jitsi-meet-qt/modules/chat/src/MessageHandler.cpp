#include "MessageHandler.h"
#include "IMessageStorage.h"
#include "ChatMessage.h"

#include <QDebug>
#include <QTimer>
#include <QQueue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <functional>

class MessageHandler::Private
{
public:
    Private(MessageHandler* q) : q_ptr(q) {}
    
    MessageHandler* q_ptr;
    
    // 状态管理
    IMessageHandler::ProcessingStatus status = IMessageHandler::Idle;
    bool processingEnabled = true;
    bool initialized = false;
    
    // 消息队列
    struct QueuedMessage {
        QVariantMap data;
        IMessageHandler::MessagePriority priority;
        QDateTime timestamp;
        int retryCount = 0;
    };
    
    QQueue<QueuedMessage> messageQueue;
    QList<QueuedMessage> failedMessages;
    QMutex queueMutex;
    
    // 组件
    IMessageStorage* messageStorage = nullptr;
    
    // 处理器和过滤器
    std::function<bool(const QVariantMap&)> messageFilter;
    std::function<QVariantMap(const QVariantMap&)> messageTransformer;
    QList<std::function<void(ChatMessage*)>> messageProcessors;
    
    // 统计信息
    int processedCount = 0;
    int successCount = 0;
    int failedCount = 0;
    int filteredCount = 0;
    QDateTime startTime;
    
    // 定时器
    QTimer* processingTimer = nullptr;
    QTimer* retryTimer = nullptr;
    
    // 配置
    int maxQueueSize = 1000;
    int maxRetryCount = 3;
    int processingInterval = 100; // ms
    int retryInterval = 5000; // ms
    
    void setProcessingStatus(IMessageHandler::ProcessingStatus newStatus) {
        if (status != newStatus) {
            status = newStatus;
            emit q_ptr->processingStatusChanged(static_cast<MessageHandler::ProcessingStatus>(newStatus));
        }
    }
    
    void updateStatistics(IMessageHandler::ProcessingResult result) {
        processedCount++;
        
        switch (result) {
        case IMessageHandler::Success:
            successCount++;
            break;
        case IMessageHandler::Failed:
            failedCount++;
            break;
        case IMessageHandler::Filtered:
            filteredCount++;
            break;
        default:
            break;
        }
        
        emit q_ptr->processedCountChanged(processedCount);
    }
    
    bool isQueueFull() const {
        return messageQueue.size() >= maxQueueSize;
    }
    
    void startProcessingTimer() {
        if (!processingTimer) {
            processingTimer = new QTimer(q_ptr);
            connect(processingTimer, &QTimer::timeout, q_ptr, &MessageHandler::processNextMessage);
        }
        
        if (processingEnabled && status == Processing) {
            processingTimer->start(processingInterval);
        }
    }
    
    void stopProcessingTimer() {
        if (processingTimer) {
            processingTimer->stop();
        }
    }
    
    void startRetryTimer() {
        if (!retryTimer) {
            retryTimer = new QTimer(q_ptr);
            retryTimer->setSingleShot(true);
            connect(retryTimer, &QTimer::timeout, q_ptr, &MessageHandler::retryFailedMessages);
        }
        
        if (!failedMessages.isEmpty()) {
            retryTimer->start(retryInterval);
        }
    }
};

MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(this))
{
    qDebug() << "MessageHandler created";
}

MessageHandler::~MessageHandler()
{
    stopProcessing();
    qDebug() << "MessageHandler destroyed";
}

bool MessageHandler::initialize(const QVariantMap& config)
{
    if (d->initialized) {
        qWarning() << "MessageHandler already initialized";
        return true;
    }
    
    qDebug() << "Initializing MessageHandler...";
    
    try {
        // 应用配置
        if (config.contains("maxQueueSize")) {
            d->maxQueueSize = config["maxQueueSize"].toInt();
        }
        if (config.contains("maxRetryCount")) {
            d->maxRetryCount = config["maxRetryCount"].toInt();
        }
        if (config.contains("processingInterval")) {
            d->processingInterval = config["processingInterval"].toInt();
        }
        if (config.contains("retryInterval")) {
            d->retryInterval = config["retryInterval"].toInt();
        }
        
        d->startTime = QDateTime::currentDateTime();
        d->initialized = true;
        
        qDebug() << "MessageHandler initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during MessageHandler initialization:" << e.what();
        return false;
    }
}

IMessageHandler::ProcessingResult MessageHandler::processIncomingMessage(const QVariantMap& data, IMessageHandler::MessagePriority priority)
{
    if (!d->initialized) {
        qWarning() << "MessageHandler not initialized";
        return static_cast<IMessageHandler::ProcessingResult>(Failed);
    }
    
    if (!d->processingEnabled) {
        qDebug() << "Processing disabled, message queued";
        return static_cast<IMessageHandler::ProcessingResult>(Queued);
    }
    
    // 验证消息
    auto validation = validateMessageContent(data);
    if (!validation.first) {
        emit messageValidationFailed(data, validation.second);
        d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Failed));
        return static_cast<IMessageHandler::ProcessingResult>(Failed);
    }
    
    // 应用过滤器
    if (!applyMessageFilter(data)) {
        emit messageFiltered(data);
        d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Filtered));
        return static_cast<IMessageHandler::ProcessingResult>(Filtered);
    }
    
    // 检查队列是否已满
    QMutexLocker locker(&d->queueMutex);
    if (d->isQueueFull()) {
        emit queueFull();
        return static_cast<IMessageHandler::ProcessingResult>(Rejected);
    }
    
    // 添加到队列
    Private::QueuedMessage queuedMsg;
    queuedMsg.data = data;
    queuedMsg.priority = priority;
    queuedMsg.timestamp = QDateTime::currentDateTime();
    
    // 按优先级插入
    bool inserted = false;
    for (int i = 0; i < d->messageQueue.size(); ++i) {
        if (priority > d->messageQueue.at(i).priority) {
            d->messageQueue.insert(i, queuedMsg);
            inserted = true;
            break;
        }
    }
    
    if (!inserted) {
        d->messageQueue.enqueue(queuedMsg);
    }
    
    emit queueSizeChanged(d->messageQueue.size());
    
    // 如果当前空闲，开始处理
    if (d->status == Idle) {
        startProcessing();
    }
    
    return static_cast<IMessageHandler::ProcessingResult>(Queued);
}

IMessageHandler::ProcessingResult MessageHandler::processOutgoingMessage(ChatMessage* message, IMessageHandler::MessagePriority priority)
{
    if (!message) {
        return static_cast<IMessageHandler::ProcessingResult>(Failed);
    }
    
    // 格式化消息
    QVariantMap data = formatMessage(message);
    
    // 应用转换器
    data = applyMessageTransformer(data);
    
    // 应用消息处理器
    for (const auto& processor : d->messageProcessors) {
        processor(message);
    }
    
    // 存储消息
    if (d->messageStorage) {
        // TODO: 存储到持久化存储
    }
    
    emit messageProcessed(message, Success);
    d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Success));
    
    return static_cast<IMessageHandler::ProcessingResult>(Success);
}

bool MessageHandler::validateMessage(const QVariantMap& data) const
{
    return validateMessageContent(data).first;
}

QVariantMap MessageHandler::formatMessage(ChatMessage* message) const
{
    if (!message) {
        return QVariantMap();
    }
    
    QVariantMap data;
    data["id"] = message->id();
    data["content"] = message->content();
    data["type"] = static_cast<int>(message->type());
    data["senderId"] = message->senderId();
    data["senderName"] = message->senderName();
    data["roomId"] = message->roomId();
    data["timestamp"] = message->timestamp().toString(Qt::ISODate);
    data["status"] = static_cast<int>(message->status());
    data["priority"] = static_cast<int>(message->priority());
    data["isRead"] = message->isRead();
    data["isEdited"] = message->isEdited();
    
    // 添加文件信息（如果是文件消息）
    if (message->type() == ChatMessage::FileMessage) {
        data["fileInfo"] = message->fileInfo();
        data["fileUrl"] = message->fileUrl().toString();
        data["fileSize"] = message->fileSize();
        data["mimeType"] = message->mimeType();
    }
    
    // 添加扩展属性
    data["properties"] = message->properties();
    
    return data;
}

ChatMessage* MessageHandler::parseMessage(const QVariantMap& data) const
{
    if (!validateMessage(data)) {
        return nullptr;
    }
    
    ChatMessage* message = new ChatMessage();
    
    // 设置基本属性
    message->setContent(data["content"].toString());
    message->setType(static_cast<ChatMessage::MessageType>(data["type"].toInt()));
    message->setSenderId(data["senderId"].toString());
    message->setSenderName(data["senderName"].toString());
    message->setRoomId(data["roomId"].toString());
    
    if (data.contains("timestamp")) {
        QDateTime timestamp = QDateTime::fromString(data["timestamp"].toString(), Qt::ISODate);
        message->setTimestamp(timestamp);
    }
    
    if (data.contains("status")) {
        message->setStatus(static_cast<ChatMessage::MessageStatus>(data["status"].toInt()));
    }
    
    if (data.contains("priority")) {
        message->setPriority(static_cast<ChatMessage::MessagePriority>(data["priority"].toInt()));
    }
    
    if (data.contains("isRead")) {
        message->setRead(data["isRead"].toBool());
    }
    
    // 设置文件信息（如果是文件消息）
    if (message->type() == ChatMessage::FileMessage) {
        if (data.contains("fileInfo")) {
            message->setFileInfo(data["fileInfo"].toMap());
        }
        if (data.contains("fileUrl")) {
            message->setFileUrl(QUrl(data["fileUrl"].toString()));
        }
        if (data.contains("fileSize")) {
            message->setFileSize(data["fileSize"].toLongLong());
        }
        if (data.contains("mimeType")) {
            message->setMimeType(data["mimeType"].toString());
        }
    }
    
    // 设置扩展属性
    if (data.contains("properties")) {
        message->setProperties(data["properties"].toMap());
    }
    
    return message;
}

bool MessageHandler::isProcessingEnabled() const
{
    return d->processingEnabled;
}

void MessageHandler::setProcessingEnabled(bool enabled)
{
    if (d->processingEnabled != enabled) {
        d->processingEnabled = enabled;
        emit processingEnabledChanged(enabled);
        
        if (enabled && d->status == Paused) {
            resumeProcessing();
        } else if (!enabled && d->status == Processing) {
            pauseProcessing();
        }
    }
}

IMessageHandler::ProcessingStatus MessageHandler::processingStatus() const
{
    return d->status;
}

int MessageHandler::queueSize() const
{
    QMutexLocker locker(&d->queueMutex);
    return d->messageQueue.size();
}

int MessageHandler::processedCount() const
{
    return d->processedCount;
}

QVariantMap MessageHandler::getStatistics() const
{
    QVariantMap stats;
    stats["processedCount"] = d->processedCount;
    stats["successCount"] = d->successCount;
    stats["failedCount"] = d->failedCount;
    stats["filteredCount"] = d->filteredCount;
    stats["queueSize"] = queueSize();
    stats["failedMessagesCount"] = d->failedMessages.size();
    
    if (d->startTime.isValid()) {
        stats["uptime"] = d->startTime.secsTo(QDateTime::currentDateTime());
    }
    
    return stats;
}

void MessageHandler::setMessageStorage(IMessageStorage* storage)
{
    d->messageStorage = storage;
}

IMessageStorage* MessageHandler::messageStorage() const
{
    return d->messageStorage;
}

void MessageHandler::setMessageFilter(std::function<bool(const QVariantMap&)> filter)
{
    d->messageFilter = filter;
}

void MessageHandler::setMessageTransformer(std::function<QVariantMap(const QVariantMap&)> transformer)
{
    d->messageTransformer = transformer;
}

void MessageHandler::addMessageProcessor(std::function<void(ChatMessage*)> processor)
{
    d->messageProcessors.append(processor);
}

void MessageHandler::clearQueue()
{
    QMutexLocker locker(&d->queueMutex);
    int oldSize = d->messageQueue.size();
    d->messageQueue.clear();
    
    if (oldSize > 0) {
        emit queueSizeChanged(0);
        emit queueEmpty();
    }
}

QList<QVariantMap> MessageHandler::getQueuedMessages() const
{
    QMutexLocker locker(&d->queueMutex);
    QList<QVariantMap> messages;
    
    for (const auto& queuedMsg : d->messageQueue) {
        messages.append(queuedMsg.data);
    }
    
    return messages;
}

void MessageHandler::startProcessing()
{
    if (d->status == Processing) {
        return;
    }
    
    qDebug() << "Starting message processing...";
    d->setProcessingStatus(static_cast<IMessageHandler::ProcessingStatus>(Processing));
    d->startProcessingTimer();
}

void MessageHandler::stopProcessing()
{
    if (d->status == Idle) {
        return;
    }
    
    qDebug() << "Stopping message processing...";
    d->stopProcessingTimer();
    d->setProcessingStatus(static_cast<IMessageHandler::ProcessingStatus>(Idle));
}

void MessageHandler::pauseProcessing()
{
    if (d->status != Processing) {
        return;
    }
    
    qDebug() << "Pausing message processing...";
    d->stopProcessingTimer();
    d->setProcessingStatus(static_cast<IMessageHandler::ProcessingStatus>(Paused));
}

void MessageHandler::resumeProcessing()
{
    if (d->status != Paused) {
        return;
    }
    
    qDebug() << "Resuming message processing...";
    d->setProcessingStatus(static_cast<IMessageHandler::ProcessingStatus>(Processing));
    d->startProcessingTimer();
}

void MessageHandler::processQueue()
{
    if (!d->processingEnabled || d->status != Processing) {
        return;
    }
    
    QMutexLocker locker(&d->queueMutex);
    
    while (!d->messageQueue.isEmpty()) {
        Private::QueuedMessage queuedMsg = d->messageQueue.dequeue();
        locker.unlock();
        
        ProcessingResult result = static_cast<ProcessingResult>(internalProcessMessage(queuedMsg.data, queuedMsg.priority));
        
        if (result == Failed && queuedMsg.retryCount < d->maxRetryCount) {
            queuedMsg.retryCount++;
            d->failedMessages.append(queuedMsg);
            d->startRetryTimer();
        }
        
        locker.relock();
        emit queueSizeChanged(d->messageQueue.size());
    }
    
    if (d->messageQueue.isEmpty()) {
        emit queueEmpty();
    }
}

void MessageHandler::retryFailedMessages()
{
    if (d->failedMessages.isEmpty()) {
        return;
    }
    
    qDebug() << "Retrying" << d->failedMessages.size() << "failed messages...";
    
    QList<Private::QueuedMessage> retryMessages = d->failedMessages;
    d->failedMessages.clear();
    
    for (const auto& queuedMsg : retryMessages) {
        ProcessingResult result = static_cast<ProcessingResult>(internalProcessMessage(queuedMsg.data, queuedMsg.priority));
        
        if (result == Failed && queuedMsg.retryCount < d->maxRetryCount) {
            Private::QueuedMessage retryMsg = queuedMsg;
            retryMsg.retryCount++;
            d->failedMessages.append(retryMsg);
        }
    }
    
    // 如果还有失败的消息，继续重试
    if (!d->failedMessages.isEmpty()) {
        d->startRetryTimer();
    }
}

void MessageHandler::clearStatistics()
{
    d->processedCount = 0;
    d->successCount = 0;
    d->failedCount = 0;
    d->filteredCount = 0;
    d->startTime = QDateTime::currentDateTime();
    
    emit processedCountChanged(0);
}

void MessageHandler::processNextMessage()
{
    if (!d->processingEnabled || d->status != Processing) {
        return;
    }
    
    QMutexLocker locker(&d->queueMutex);
    
    if (d->messageQueue.isEmpty()) {
        locker.unlock();
        stopProcessing();
        return;
    }
    
    Private::QueuedMessage queuedMsg = d->messageQueue.dequeue();
    locker.unlock();
    
    ProcessingResult result = static_cast<ProcessingResult>(internalProcessMessage(queuedMsg.data, queuedMsg.priority));
    
    if (result == Failed && queuedMsg.retryCount < d->maxRetryCount) {
        queuedMsg.retryCount++;
        d->failedMessages.append(queuedMsg);
        d->startRetryTimer();
    }
    
    emit queueSizeChanged(queueSize());
}

void MessageHandler::handleTimeoutMessages()
{
    // TODO: 处理超时消息
    qDebug() << "Handling timeout messages...";
}

IMessageHandler::ProcessingResult MessageHandler::internalProcessMessage(const QVariantMap& data, IMessageHandler::MessagePriority priority)
{
    try {
        // 解析消息
        ChatMessage* message = parseMessage(data);
        if (!message) {
            d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Failed));
            return static_cast<IMessageHandler::ProcessingResult>(Failed);
        }
        
        // 应用消息处理器
        for (const auto& processor : d->messageProcessors) {
            processor(message);
        }
        
        // 存储消息
        if (d->messageStorage) {
            // TODO: 存储到持久化存储
        }
        
        emit messageProcessed(message, Success);
        d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Success));
        
        return static_cast<IMessageHandler::ProcessingResult>(Success);
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during message processing:" << e.what();
        emit processingError(QString("Processing exception: %1").arg(e.what()));
        d->updateStatistics(static_cast<IMessageHandler::ProcessingResult>(Failed));
        return static_cast<IMessageHandler::ProcessingResult>(Failed);
    }
}

QPair<bool, QString> MessageHandler::validateMessageContent(const QVariantMap& data) const
{
    // 检查必需字段
    if (!data.contains("content")) {
        return qMakePair(false, "Missing content field");
    }
    
    if (!data.contains("senderId")) {
        return qMakePair(false, "Missing senderId field");
    }
    
    if (!data.contains("roomId")) {
        return qMakePair(false, "Missing roomId field");
    }
    
    // 检查内容长度
    QString content = data["content"].toString();
    if (content.length() > 10000) { // 10KB limit
        return qMakePair(false, "Content too long");
    }
    
    // 检查ID格式
    QString senderId = data["senderId"].toString();
    if (senderId.isEmpty() || senderId.length() > 255) {
        return qMakePair(false, "Invalid senderId");
    }
    
    QString roomId = data["roomId"].toString();
    if (roomId.isEmpty() || roomId.length() > 255) {
        return qMakePair(false, "Invalid roomId");
    }
    
    return qMakePair(true, QString());
}

bool MessageHandler::applyMessageFilter(const QVariantMap& data) const
{
    if (d->messageFilter) {
        return d->messageFilter(data);
    }
    return true;
}

QVariantMap MessageHandler::applyMessageTransformer(const QVariantMap& data) const
{
    if (d->messageTransformer) {
        return d->messageTransformer(data);
    }
    return data;
}

void MessageHandler::setProcessingStatus(IMessageHandler::ProcessingStatus status)
{
    d->setProcessingStatus(status);
}

void MessageHandler::updateStatistics(IMessageHandler::ProcessingResult result)
{
    d->updateStatistics(result);
}