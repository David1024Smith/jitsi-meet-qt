#include "HistoryManager.h"
#include "MessageStorage.h"
#include "../models/ChatMessage.h"
#include <QTimer>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>

class HistoryManager::Private
{
public:
    MessageStorage* messageStorage;
    bool enabled;
    int retentionDays;
    int maxMessages;
    bool autoCleanupEnabled;
    CleanupStrategy cleanupStrategy;
    int autoCleanupInterval; // 小时
    int totalMessages;
    
    QTimer* autoCleanupTimer;
    QTimer* statisticsTimer;
    QMutex mutex;
    
    QVariantMap statistics;
    
    Private()
        : messageStorage(nullptr)
        , enabled(true)
        , retentionDays(365)
        , maxMessages(100000)
        , autoCleanupEnabled(true)
        , cleanupStrategy(ByAge)
        , autoCleanupInterval(24)
        , totalMessages(0)
    {
        autoCleanupTimer = new QTimer();
        statisticsTimer = new QTimer();
    }
    
    ~Private()
    {
        delete autoCleanupTimer;
        delete statisticsTimer;
    }
};

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    // 连接定时器
    connect(d->autoCleanupTimer, &QTimer::timeout, this, &HistoryManager::handleAutoCleanupTimer);
    connect(d->statisticsTimer, &QTimer::timeout, this, &HistoryManager::handleStatisticsTimer);
}

HistoryManager::~HistoryManager() = default;

bool HistoryManager::initialize(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (!validateConfiguration(config)) {
        return false;
    }
    
    // 处理配置参数
    if (config.contains("enabled")) {
        d->enabled = config.value("enabled").toBool();
    }
    
    if (config.contains("retentionDays")) {
        d->retentionDays = config.value("retentionDays").toInt();
    }
    
    if (config.contains("maxMessages")) {
        d->maxMessages = config.value("maxMessages").toInt();
    }
    
    if (config.contains("autoCleanupEnabled")) {
        d->autoCleanupEnabled = config.value("autoCleanupEnabled").toBool();
    }
    
    if (config.contains("cleanupStrategy")) {
        d->cleanupStrategy = static_cast<CleanupStrategy>(config.value("cleanupStrategy").toInt());
    }
    
    if (config.contains("autoCleanupInterval")) {
        d->autoCleanupInterval = config.value("autoCleanupInterval").toInt();
    }
    
    // 初始化定时器
    initializeTimers();
    
    // 更新统计信息
    updateInternalStatistics();
    
    return true;
}

void HistoryManager::setMessageStorage(MessageStorage* storage)
{
    QMutexLocker locker(&d->mutex);
    d->messageStorage = storage;
    
    if (storage) {
        // 连接存储信号
        connect(storage, &MessageStorage::messageStored, this, [this](const QString&) {
            updateInternalStatistics();
        });
        
        connect(storage, &MessageStorage::messageDeleted, this, [this](const QString&) {
            updateInternalStatistics();
        });
    }
}

MessageStorage* HistoryManager::messageStorage() const
{
    return d->messageStorage;
}

bool HistoryManager::isEnabled() const
{
    return d->enabled;
}

void HistoryManager::setEnabled(bool enabled)
{
    if (d->enabled != enabled) {
        d->enabled = enabled;
        emit enabledChanged(enabled);
        
        if (enabled) {
            initializeTimers();
        } else {
            d->autoCleanupTimer->stop();
            d->statisticsTimer->stop();
        }
    }
}

int HistoryManager::retentionDays() const
{
    return d->retentionDays;
}

void HistoryManager::setRetentionDays(int days)
{
    if (d->retentionDays != days) {
        d->retentionDays = days;
        emit retentionDaysChanged(days);
    }
}

int HistoryManager::maxMessages() const
{
    return d->maxMessages;
}

void HistoryManager::setMaxMessages(int maxMessages)
{
    if (d->maxMessages != maxMessages) {
        d->maxMessages = maxMessages;
        emit maxMessagesChanged(maxMessages);
    }
}

bool HistoryManager::isAutoCleanupEnabled() const
{
    return d->autoCleanupEnabled;
}

void HistoryManager::setAutoCleanupEnabled(bool enabled)
{
    if (d->autoCleanupEnabled != enabled) {
        d->autoCleanupEnabled = enabled;
        emit autoCleanupChanged(enabled);
        
        if (enabled && d->enabled) {
            d->autoCleanupTimer->start(d->autoCleanupInterval * 60 * 60 * 1000);
        } else {
            d->autoCleanupTimer->stop();
        }
    }
}

HistoryManager::CleanupStrategy HistoryManager::cleanupStrategy() const
{
    return d->cleanupStrategy;
}

void HistoryManager::setCleanupStrategy(CleanupStrategy strategy)
{
    d->cleanupStrategy = strategy;
}

int HistoryManager::autoCleanupInterval() const
{
    return d->autoCleanupInterval;
}

void HistoryManager::setAutoCleanupInterval(int hours)
{
    d->autoCleanupInterval = hours;
    if (d->autoCleanupEnabled && d->enabled && hours > 0) {
        d->autoCleanupTimer->start(hours * 60 * 60 * 1000);
    }
}

int HistoryManager::totalMessages() const
{
    return d->totalMessages;
}

QList<ChatMessage*> HistoryManager::getRoomHistory(const QString& roomId, int limit, int offset)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty()) {
        return QList<ChatMessage*>();
    }
    
    return d->messageStorage->getRoomMessages(roomId, limit, offset);
}

QList<ChatMessage*> HistoryManager::getHistoryByTimeRange(const QString& roomId, const QDateTime& startTime, const QDateTime& endTime, int limit)
{
    if (!d->enabled || !d->messageStorage) {
        return QList<ChatMessage*>();
    }
    
    return d->messageStorage->getMessagesByTimeRange(roomId, startTime, endTime, limit);
}

QList<ChatMessage*> HistoryManager::searchHistory(const QString& query, const QString& roomId, SearchOptions options, int limit)
{
    if (!d->enabled || !d->messageStorage || query.isEmpty()) {
        return QList<ChatMessage*>();
    }
    
    QList<ChatMessage*> results = executeSearch(query, roomId, options, limit);
    
    emit searchCompleted(query, results.size());
    return results;
}

QList<ChatMessage*> HistoryManager::advancedSearch(const QVariantMap& criteria)
{
    if (!d->enabled || !d->messageStorage || criteria.isEmpty()) {
        return QList<ChatMessage*>();
    }
    
    // 构建高级搜索条件
    QString query = criteria.value("query").toString();
    QString roomId = criteria.value("roomId").toString();
    QDateTime startTime = criteria.value("startTime").toDateTime();
    QDateTime endTime = criteria.value("endTime").toDateTime();
    QString senderId = criteria.value("senderId").toString();
    int messageType = criteria.value("messageType", -1).toInt();
    int limit = criteria.value("limit", 50).toInt();
    
    // 基于时间范围搜索
    QList<ChatMessage*> timeResults;
    if (startTime.isValid() && endTime.isValid()) {
        timeResults = getHistoryByTimeRange(roomId, startTime, endTime, limit * 2);
    } else {
        timeResults = getRoomHistory(roomId, limit * 2);
    }
    
    // 过滤结果
    QList<ChatMessage*> filteredResults;
    for (ChatMessage* message : timeResults) {
        bool matches = true;
        
        // 文本匹配
        if (!query.isEmpty() && !message->content().contains(query, Qt::CaseInsensitive)) {
            matches = false;
        }
        
        // 发送者匹配
        if (!senderId.isEmpty() && message->senderId() != senderId) {
            matches = false;
        }
        
        // 消息类型匹配
        if (messageType >= 0 && static_cast<int>(message->type()) != messageType) {
            matches = false;
        }
        
        if (matches) {
            filteredResults.append(message);
            if (filteredResults.size() >= limit) {
                break;
            }
        }
    }
    
    // 清理不需要的消息
    for (ChatMessage* message : timeResults) {
        if (!filteredResults.contains(message)) {
            delete message;
        }
    }
    
    return filteredResults;
}

QStringList HistoryManager::getSearchSuggestions(const QString& partialQuery, int limit)
{
    QStringList suggestions;
    
    if (!d->enabled || !d->messageStorage || partialQuery.length() < 2) {
        return suggestions;
    }
    
    // 简单的建议实现：基于历史搜索词
    // 这里可以扩展为更复杂的建议算法
    QList<ChatMessage*> recentMessages = d->messageStorage->getRoomMessages("", 100);
    
    QSet<QString> words;
    for (ChatMessage* message : recentMessages) {
        QStringList messageWords = message->content().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
        for (const QString& word : messageWords) {
            if (word.startsWith(partialQuery, Qt::CaseInsensitive) && word.length() > partialQuery.length()) {
                words.insert(word.toLower());
            }
        }
    }
    
    suggestions = words.values();
    if (suggestions.size() > limit) {
        suggestions = suggestions.mid(0, limit);
    }
    
    qDeleteAll(recentMessages);
    return suggestions;
}

bool HistoryManager::addMessage(ChatMessage* message)
{
    if (!d->enabled || !d->messageStorage || !message) {
        return false;
    }
    
    bool success = d->messageStorage->storeMessage(message) == IMessageStorage::Success;
    
    if (success) {
        emit messageAdded(message);
        updateInternalStatistics();
    }
    
    return success;
}

bool HistoryManager::updateMessage(ChatMessage* message)
{
    if (!d->enabled || !d->messageStorage || !message) {
        return false;
    }
    
    bool success = d->messageStorage->updateMessage(message) == IMessageStorage::Success;
    
    if (success) {
        emit messageUpdated(message);
    }
    
    return success;
}

bool HistoryManager::deleteMessage(const QString& messageId)
{
    if (!d->enabled || !d->messageStorage || messageId.isEmpty()) {
        return false;
    }
    
    bool success = d->messageStorage->deleteMessage(messageId) == IMessageStorage::Success;
    
    if (success) {
        emit messageDeleted(messageId);
        updateInternalStatistics();
    }
    
    return success;
}

bool HistoryManager::clearRoomHistory(const QString& roomId)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty()) {
        return false;
    }
    
    bool success = d->messageStorage->deleteRoomMessages(roomId) == IMessageStorage::Success;
    
    if (success) {
        emit roomHistoryCleared(roomId);
        updateInternalStatistics();
    }
    
    return success;
}

bool HistoryManager::clearAllHistory()
{
    if (!d->enabled || !d->messageStorage) {
        return false;
    }
    
    QStringList rooms = d->messageStorage->getRoomList();
    bool success = true;
    
    for (const QString& roomId : rooms) {
        if (d->messageStorage->deleteRoomMessages(roomId) != IMessageStorage::Success) {
            success = false;
        }
    }
    
    if (success) {
        emit allHistoryCleared();
        updateInternalStatistics();
    }
    
    return success;
}

int HistoryManager::cleanupExpiredHistory(int days)
{
    if (!d->enabled || !d->messageStorage) {
        return 0;
    }
    
    if (days < 0) {
        days = d->retentionDays;
    }
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-days);
    
    // 获取要删除的消息数量
    int beforeCount = d->messageStorage->getMessageCount();
    
    // 执行清理
    d->messageStorage->deleteMessagesBefore("", cutoffDate);
    
    int afterCount = d->messageStorage->getMessageCount();
    int deletedCount = beforeCount - afterCount;
    
    if (deletedCount > 0) {
        updateInternalStatistics();
        emit cleanupCompleted(deletedCount);
    }
    
    return deletedCount;
}

bool HistoryManager::exportHistory(const QString& roomId, const QString& filePath, ExportFormat format, const QDateTime& startTime, const QDateTime& endTime)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty() || filePath.isEmpty()) {
        return false;
    }
    
    // 获取消息
    QList<ChatMessage*> messages;
    if (startTime.isValid() && endTime.isValid()) {
        messages = getHistoryByTimeRange(roomId, startTime, endTime);
    } else {
        messages = getRoomHistory(roomId, -1); // 获取所有消息
    }
    
    bool success = false;
    
    switch (format) {
    case PlainText:
        success = exportAsPlainText(messages, filePath);
        break;
    case HTML:
        success = exportAsHTML(messages, filePath);
        break;
    case JSON:
        success = exportAsJSON(messages, filePath);
        break;
    case CSV:
        success = exportAsCSV(messages, filePath);
        break;
    case XML:
        success = exportAsXML(messages, filePath);
        break;
    }
    
    qDeleteAll(messages);
    emit exportCompleted(filePath, success);
    
    return success;
}

bool HistoryManager::importHistory(const QString& filePath, ExportFormat format)
{
    if (!d->enabled || !d->messageStorage || filePath.isEmpty() || !QFile::exists(filePath)) {
        return false;
    }
    
    // 目前只支持JSON格式的导入
    if (format != JSON) {
        emit importCompleted(filePath, false, 0);
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit importCompleted(filePath, false, 0);
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError) {
        emit importCompleted(filePath, false, 0);
        return false;
    }
    
    QJsonArray messagesArray = doc.array();
    int importedCount = 0;
    
    for (const QJsonValue& value : messagesArray) {
        QJsonObject messageObj = value.toObject();
        ChatMessage* message = ChatMessage::fromVariantMap(messageObj.toVariantObject(), this);
        
        if (message && addMessage(message)) {
            importedCount++;
        }
        
        if (message) {
            delete message;
        }
    }
    
    emit importCompleted(filePath, true, importedCount);
    return true;
}

QVariantMap HistoryManager::getHistoryStatistics(const QString& roomId)
{
    QVariantMap stats;
    
    if (!d->enabled || !d->messageStorage) {
        return stats;
    }
    
    if (roomId.isEmpty()) {
        // 全局统计
        stats = d->statistics;
    } else {
        // 房间统计
        stats["messageCount"] = getRoomMessageCount(roomId);
        stats["earliestMessage"] = getRoomEarliestMessage(roomId);
        stats["latestMessage"] = getRoomLatestMessage(roomId);
    }
    
    return stats;
}

QStringList HistoryManager::getRoomsWithHistory()
{
    if (!d->enabled || !d->messageStorage) {
        return QStringList();
    }
    
    return d->messageStorage->getRoomList();
}

int HistoryManager::getRoomMessageCount(const QString& roomId)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty()) {
        return 0;
    }
    
    return d->messageStorage->getMessageCount(roomId);
}

QDateTime HistoryManager::getRoomEarliestMessage(const QString& roomId)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty()) {
        return QDateTime();
    }
    
    QList<ChatMessage*> messages = d->messageStorage->getRoomMessages(roomId, 1, 0, IMessageStorage::Ascending);
    if (messages.isEmpty()) {
        return QDateTime();
    }
    
    QDateTime earliest = messages.first()->timestamp();
    qDeleteAll(messages);
    
    return earliest;
}

QDateTime HistoryManager::getRoomLatestMessage(const QString& roomId)
{
    if (!d->enabled || !d->messageStorage || roomId.isEmpty()) {
        return QDateTime();
    }
    
    ChatMessage* lastMessage = d->messageStorage->getLastMessage(roomId);
    if (!lastMessage) {
        return QDateTime();
    }
    
    QDateTime latest = lastMessage->timestamp();
    delete lastMessage;
    
    return latest;
}

bool HistoryManager::checkIntegrity()
{
    if (!d->enabled || !d->messageStorage) {
        return false;
    }
    
    return d->messageStorage->checkIntegrity();
}

bool HistoryManager::repairHistory()
{
    if (!d->enabled || !d->messageStorage) {
        return false;
    }
    
    return d->messageStorage->repairDatabase() == IMessageStorage::Success;
}

bool HistoryManager::compactHistory()
{
    if (!d->enabled || !d->messageStorage) {
        return false;
    }
    
    return d->messageStorage->compact() == IMessageStorage::Success;
}

bool HistoryManager::rebuildIndexes()
{
    if (!d->enabled || !d->messageStorage) {
        return false;
    }
    
    return d->messageStorage->rebuildIndexes() == IMessageStorage::Success;
}

void HistoryManager::performAutoCleanup()
{
    if (!d->autoCleanupEnabled || !d->enabled) {
        return;
    }
    
    int deletedCount = 0;
    
    switch (d->cleanupStrategy) {
    case ByAge:
        deletedCount = cleanupExpiredHistory(d->retentionDays);
        break;
    case ByCount:
        // 实现按数量清理
        if (d->totalMessages > d->maxMessages) {
            // 这里需要更复杂的逻辑来删除最旧的消息
            deletedCount = cleanupExpiredHistory(d->retentionDays);
        }
        break;
    case BySize:
        // 实现按大小清理
        // 这里需要检查存储大小并清理
        deletedCount = cleanupExpiredHistory(d->retentionDays);
        break;
    case Manual:
        // 手动清理，不执行自动清理
        break;
    }
    
    if (deletedCount > 0) {
        emit cleanupCompleted(deletedCount);
    }
}

void HistoryManager::refreshStatistics()
{
    updateInternalStatistics();
}

void HistoryManager::optimizeStorage()
{
    if (!d->enabled || !d->messageStorage) {
        return;
    }
    
    d->messageStorage->optimize();
}

void HistoryManager::reloadConfiguration()
{
    // 重新加载配置的实现
    // 这里可以从配置文件重新读取设置
}

void HistoryManager::pauseAutoCleanup()
{
    d->autoCleanupTimer->stop();
}

void HistoryManager::resumeAutoCleanup()
{
    if (d->autoCleanupEnabled && d->enabled && d->autoCleanupInterval > 0) {
        d->autoCleanupTimer->start(d->autoCleanupInterval * 60 * 60 * 1000);
    }
}

void HistoryManager::handleAutoCleanupTimer()
{
    performAutoCleanup();
}

void HistoryManager::handleStatisticsTimer()
{
    updateInternalStatistics();
}// 私有方法
实现
void HistoryManager::initializeTimers()
{
    // 启动自动清理定时器
    if (d->autoCleanupEnabled && d->autoCleanupInterval > 0) {
        d->autoCleanupTimer->start(d->autoCleanupInterval * 60 * 60 * 1000);
    }
    
    // 启动统计更新定时器（每5分钟更新一次）
    d->statisticsTimer->start(5 * 60 * 1000);
}

QList<ChatMessage*> HistoryManager::executeSearch(const QString& query, const QString& roomId, SearchOptions options, int limit)
{
    if (!d->messageStorage) {
        return QList<ChatMessage*>();
    }
    
    // 构建搜索SQL
    QString sql = buildSearchSQL(query, roomId, options);
    
    // 目前使用简单的搜索实现
    return d->messageStorage->searchMessages(query, roomId, limit);
}

QString HistoryManager::buildSearchSQL(const QString& query, const QString& roomId, SearchOptions options)
{
    QString sql = "SELECT * FROM messages WHERE ";
    QStringList conditions;
    
    // 文本搜索条件
    if (options.testFlag(RegularExpression)) {
        conditions.append("content REGEXP ?");
    } else if (options.testFlag(WholeWords)) {
        conditions.append("content REGEXP '\\b' || ? || '\\b'");
    } else {
        if (options.testFlag(CaseSensitive)) {
            conditions.append("content LIKE ?");
        } else {
            conditions.append("LOWER(content) LIKE LOWER(?)");
        }
    }
    
    // 房间条件
    if (!roomId.isEmpty()) {
        conditions.append("room_id = ?");
    }
    
    // 是否包含已删除消息
    if (!options.testFlag(IncludeDeleted)) {
        conditions.append("status != ?"); // 假设有删除状态
    }
    
    sql += conditions.join(" AND ");
    sql += " ORDER BY timestamp DESC LIMIT ?";
    
    return sql;
}

bool HistoryManager::exportAsPlainText(const QList<ChatMessage*>& messages, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    for (ChatMessage* message : messages) {
        stream << message->timestamp().toString("yyyy-MM-dd hh:mm:ss") << " ";
        stream << "[" << message->senderName() << "] ";
        stream << message->content() << "\n";
    }
    
    return true;
}

bool HistoryManager::exportAsHTML(const QList<ChatMessage*>& messages, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    stream << "<!DOCTYPE html>\n";
    stream << "<html>\n<head>\n";
    stream << "<meta charset=\"UTF-8\">\n";
    stream << "<title>Chat History</title>\n";
    stream << "<style>\n";
    stream << "body { font-family: Arial, sans-serif; }\n";
    stream << ".message { margin: 10px 0; padding: 5px; border-left: 3px solid #ccc; }\n";
    stream << ".timestamp { color: #666; font-size: 0.9em; }\n";
    stream << ".sender { font-weight: bold; }\n";
    stream << "</style>\n";
    stream << "</head>\n<body>\n";
    stream << "<h1>Chat History</h1>\n";
    
    for (ChatMessage* message : messages) {
        stream << "<div class=\"message\">\n";
        stream << "<div class=\"timestamp\">" << message->timestamp().toString("yyyy-MM-dd hh:mm:ss") << "</div>\n";
        stream << "<div class=\"sender\">" << message->senderName().toHtmlEscaped() << "</div>\n";
        stream << "<div class=\"content\">" << message->content().toHtmlEscaped() << "</div>\n";
        stream << "</div>\n";
    }
    
    stream << "</body>\n</html>\n";
    return true;
}

bool HistoryManager::exportAsJSON(const QList<ChatMessage*>& messages, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonArray messagesArray;
    for (ChatMessage* message : messages) {
        QJsonObject messageObj = QJsonObject::fromVariantMap(message->toVariantMap());
        messagesArray.append(messageObj);
    }
    
    QJsonDocument doc(messagesArray);
    file.write(doc.toJson());
    
    return true;
}

bool HistoryManager::exportAsCSV(const QList<ChatMessage*>& messages, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // CSV 头部
    stream << "Timestamp,Sender ID,Sender Name,Room ID,Type,Content\n";
    
    for (ChatMessage* message : messages) {
        stream << "\"" << message->timestamp().toString("yyyy-MM-dd hh:mm:ss") << "\",";
        stream << "\"" << message->senderId() << "\",";
        stream << "\"" << message->senderName() << "\",";
        stream << "\"" << message->roomId() << "\",";
        stream << "\"" << static_cast<int>(message->type()) << "\",";
        stream << "\"" << message->content().replace("\"", "\"\"") << "\"\n";
    }
    
    return true;
}

bool HistoryManager::exportAsXML(const QList<ChatMessage*>& messages, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    
    writer.writeStartElement("chatHistory");
    
    for (ChatMessage* message : messages) {
        writer.writeStartElement("message");
        writer.writeAttribute("id", message->id());
        writer.writeAttribute("timestamp", message->timestamp().toString(Qt::ISODate));
        writer.writeAttribute("type", QString::number(static_cast<int>(message->type())));
        
        writer.writeTextElement("senderId", message->senderId());
        writer.writeTextElement("senderName", message->senderName());
        writer.writeTextElement("roomId", message->roomId());
        writer.writeTextElement("content", message->content());
        
        writer.writeEndElement(); // message
    }
    
    writer.writeEndElement(); // chatHistory
    writer.writeEndDocument();
    
    return true;
}

void HistoryManager::updateInternalStatistics()
{
    if (!d->messageStorage) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 更新总消息数
    int newTotal = d->messageStorage->getMessageCount();
    if (d->totalMessages != newTotal) {
        d->totalMessages = newTotal;
        emit totalMessagesChanged(newTotal);
    }
    
    // 更新统计信息
    d->statistics.clear();
    d->statistics["totalMessages"] = d->totalMessages;
    d->statistics["retentionDays"] = d->retentionDays;
    d->statistics["maxMessages"] = d->maxMessages;
    d->statistics["autoCleanupEnabled"] = d->autoCleanupEnabled;
    d->statistics["cleanupStrategy"] = static_cast<int>(d->cleanupStrategy);
    
    // 获取房间统计
    QStringList rooms = d->messageStorage->getRoomList();
    d->statistics["roomCount"] = rooms.size();
    
    QVariantMap roomStats;
    for (const QString& roomId : rooms) {
        QVariantMap roomInfo;
        roomInfo["messageCount"] = getRoomMessageCount(roomId);
        roomInfo["earliestMessage"] = getRoomEarliestMessage(roomId);
        roomInfo["latestMessage"] = getRoomLatestMessage(roomId);
        roomStats[roomId] = roomInfo;
    }
    d->statistics["rooms"] = roomStats;
    
    // 获取存储统计
    QVariantMap storageStats = d->messageStorage->getStatistics();
    d->statistics["storage"] = storageStats;
    
    emit statisticsUpdated(d->statistics);
}

bool HistoryManager::validateConfiguration(const QVariantMap& config)
{
    // 验证配置参数的有效性
    if (config.contains("retentionDays")) {
        int days = config.value("retentionDays").toInt();
        if (days < 1 || days > 3650) { // 1天到10年
            emit errorOccurred("Invalid retention days: " + QString::number(days));
            return false;
        }
    }
    
    if (config.contains("maxMessages")) {
        int maxMsg = config.value("maxMessages").toInt();
        if (maxMsg < 1000 || maxMsg > 10000000) { // 1K到10M
            emit errorOccurred("Invalid max messages: " + QString::number(maxMsg));
            return false;
        }
    }
    
    if (config.contains("autoCleanupInterval")) {
        int interval = config.value("autoCleanupInterval").toInt();
        if (interval < 1 || interval > 168) { // 1小时到1周
            emit errorOccurred("Invalid cleanup interval: " + QString::number(interval));
            return false;
        }
    }
    
    return true;
}