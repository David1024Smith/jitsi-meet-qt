#include "MessageStorage.h"
#include "../models/ChatMessage.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

class MessageStorage::Private
{
public:
    QString databasePath;
    QSqlDatabase database;
    StorageStatus currentStatus;
    QMutex mutex;
    
    // 缓存相关
    bool cacheEnabled;
    int cacheLimit;
    QHash<QString, ChatMessage*> messageCache;
    QStringList cacheOrder;
    int cacheHits;
    int cacheMisses;
    
    // 自动清理
    QTimer* autoCleanupTimer;
    int autoCleanupInterval; // 小时
    
    // 存储限制
    qint64 maxStorageSize;
    
    // 统计信息
    int totalMessages;
    qint64 totalSize;
    
    Private()
        : currentStatus(Uninitialized)
        , cacheEnabled(true)
        , cacheLimit(1000)
        , cacheHits(0)
        , cacheMisses(0)
        , autoCleanupInterval(24)
        , maxStorageSize(1024 * 1024 * 1024) // 1GB
        , totalMessages(0)
        , totalSize(0)
    {
        autoCleanupTimer = new QTimer();
    }
    
    ~Private()
    {
        qDeleteAll(messageCache.values());
        delete autoCleanupTimer;
    }
};

MessageStorage::MessageStorage(QObject *parent)
    : IMessageStorage(parent)
    , d(std::make_unique<Private>())
{
    // 设置默认数据库路径
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    d->databasePath = QDir(dataPath).filePath("chat_messages.db");
    
    // 连接自动清理定时器
    connect(d->autoCleanupTimer, &QTimer::timeout, this, &MessageStorage::handleAutoCleanupTimer);
}

MessageStorage::~MessageStorage()
{
    close();
}

bool MessageStorage::initialize(const QVariantMap& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->currentStatus == Ready) {
        return true;
    }
    
    setStatus(Busy);
    
    // 处理配置参数
    if (config.contains("databasePath")) {
        d->databasePath = config.value("databasePath").toString();
    }
    
    if (config.contains("cacheEnabled")) {
        d->cacheEnabled = config.value("cacheEnabled").toBool();
    }
    
    if (config.contains("cacheLimit")) {
        d->cacheLimit = config.value("cacheLimit").toInt();
    }
    
    if (config.contains("maxStorageSize")) {
        d->maxStorageSize = config.value("maxStorageSize").toLongLong();
    }
    
    // 初始化数据库
    if (!initializeDatabase()) {
        setStatus(Error);
        return false;
    }
    
    // 更新统计信息
    updateStatistics();
    
    // 启动自动清理定时器
    if (d->autoCleanupInterval > 0) {
        d->autoCleanupTimer->start(d->autoCleanupInterval * 60 * 60 * 1000); // 转换为毫秒
    }
    
    setStatus(Ready);
    return true;
}

void MessageStorage::close()
{
    QMutexLocker locker(&d->mutex);
    
    d->autoCleanupTimer->stop();
    
    if (d->database.isOpen()) {
        d->database.close();
    }
    
    // 清理缓存
    qDeleteAll(d->messageCache.values());
    d->messageCache.clear();
    d->cacheOrder.clear();
    
    setStatus(Uninitialized);
}

IMessageStorage::StorageStatus MessageStorage::status() const
{
    return d->currentStatus;
}

bool MessageStorage::isReady() const
{
    return d->currentStatus == Ready;
}

IMessageStorage::OperationResult MessageStorage::storeMessage(ChatMessage* message)
{
    if (!message || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    if (!validateMessage(message)) {
        return Failed;
    }
    
    auto query = prepareQuery(
        "INSERT INTO messages (id, content, type, sender_id, sender_name, room_id, "
        "timestamp, status, priority, is_read, is_edited, edited_timestamp, "
        "file_info, file_url, file_size, mime_type, properties) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    
    if (!query) {
        return Failed;
    }
    
    bindMessageToQuery(*query, message);
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    // 添加到缓存
    if (d->cacheEnabled) {
        addToCache(message->clone(this));
    }
    
    d->totalMessages++;
    updateStatistics();
    
    emit messageStored(message->id());
    return Success;
}IMessa
geStorage::OperationResult MessageStorage::storeMessages(const QList<ChatMessage*>& messages)
{
    if (messages.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 开始事务
    d->database.transaction();
    
    bool allSuccess = true;
    for (ChatMessage* message : messages) {
        if (storeMessage(message) != Success) {
            allSuccess = false;
            break;
        }
    }
    
    if (allSuccess) {
        d->database.commit();
        return Success;
    } else {
        d->database.rollback();
        return Failed;
    }
}

ChatMessage* MessageStorage::getMessage(const QString& messageId)
{
    if (messageId.isEmpty() || !isReady()) {
        return nullptr;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 先从缓存查找
    if (d->cacheEnabled) {
        ChatMessage* cached = getFromCache(messageId);
        if (cached) {
            d->cacheHits++;
            return cached->clone(this);
        }
        d->cacheMisses++;
    }
    
    auto query = prepareQuery("SELECT * FROM messages WHERE id = ?");
    if (!query) {
        return nullptr;
    }
    
    query->addBindValue(messageId);
    
    if (!executeQuery(*query) || !query->next()) {
        return nullptr;
    }
    
    ChatMessage* message = createMessageFromQuery(*query);
    
    // 添加到缓存
    if (message && d->cacheEnabled) {
        addToCache(message->clone(this));
    }
    
    return message;
}

QList<ChatMessage*> MessageStorage::getRoomMessages(const QString& roomId, int limit, int offset, SortOrder order)
{
    QList<ChatMessage*> messages;
    
    if (roomId.isEmpty() || !isReady()) {
        return messages;
    }
    
    QMutexLocker locker(&d->mutex);
    
    QString orderClause = (order == Ascending) ? "ASC" : "DESC";
    QString sql = QString("SELECT * FROM messages WHERE room_id = ? ORDER BY timestamp %1 LIMIT ? OFFSET ?").arg(orderClause);
    
    auto query = prepareQuery(sql);
    if (!query) {
        return messages;
    }
    
    query->addBindValue(roomId);
    query->addBindValue(limit);
    query->addBindValue(offset);
    
    if (!executeQuery(*query)) {
        return messages;
    }
    
    while (query->next()) {
        ChatMessage* message = createMessageFromQuery(*query);
        if (message) {
            messages.append(message);
        }
    }
    
    return messages;
}

QList<ChatMessage*> MessageStorage::getMessagesByTimeRange(const QString& roomId, const QDateTime& startTime, const QDateTime& endTime, int limit)
{
    QList<ChatMessage*> messages;
    
    if (!isReady()) {
        return messages;
    }
    
    QMutexLocker locker(&d->mutex);
    
    QString sql = "SELECT * FROM messages WHERE timestamp BETWEEN ? AND ?";
    if (!roomId.isEmpty()) {
        sql += " AND room_id = ?";
    }
    sql += " ORDER BY timestamp DESC LIMIT ?";
    
    auto query = prepareQuery(sql);
    if (!query) {
        return messages;
    }
    
    query->addBindValue(startTime);
    query->addBindValue(endTime);
    if (!roomId.isEmpty()) {
        query->addBindValue(roomId);
    }
    query->addBindValue(limit);
    
    if (!executeQuery(*query)) {
        return messages;
    }
    
    while (query->next()) {
        ChatMessage* message = createMessageFromQuery(*query);
        if (message) {
            messages.append(message);
        }
    }
    
    return messages;
}

QList<ChatMessage*> MessageStorage::searchMessages(const QString& query, const QString& roomId, int limit)
{
    QList<ChatMessage*> messages;
    
    if (query.isEmpty() || !isReady()) {
        return messages;
    }
    
    QMutexLocker locker(&d->mutex);
    
    QString sql = "SELECT * FROM messages WHERE content LIKE ?";
    if (!roomId.isEmpty()) {
        sql += " AND room_id = ?";
    }
    sql += " ORDER BY timestamp DESC LIMIT ?";
    
    auto sqlQuery = prepareQuery(sql);
    if (!sqlQuery) {
        return messages;
    }
    
    sqlQuery->addBindValue(QString("%%1%").arg(query));
    if (!roomId.isEmpty()) {
        sqlQuery->addBindValue(roomId);
    }
    sqlQuery->addBindValue(limit);
    
    if (!executeQuery(*sqlQuery)) {
        return messages;
    }
    
    while (sqlQuery->next()) {
        ChatMessage* message = createMessageFromQuery(*sqlQuery);
        if (message) {
            messages.append(message);
        }
    }
    
    return messages;
}

IMessageStorage::OperationResult MessageStorage::updateMessage(ChatMessage* message)
{
    if (!message || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    if (!validateMessage(message)) {
        return Failed;
    }
    
    auto query = prepareQuery(
        "UPDATE messages SET content = ?, type = ?, sender_name = ?, "
        "status = ?, priority = ?, is_read = ?, is_edited = ?, edited_timestamp = ?, "
        "file_info = ?, file_url = ?, file_size = ?, mime_type = ?, properties = ? "
        "WHERE id = ?"
    );
    
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(message->content());
    query->addBindValue(static_cast<int>(message->type()));
    query->addBindValue(message->senderName());
    query->addBindValue(static_cast<int>(message->status()));
    query->addBindValue(static_cast<int>(message->priority()));
    query->addBindValue(message->isRead());
    query->addBindValue(message->isEdited());
    query->addBindValue(message->editedTimestamp());
    
    QJsonDocument fileInfoDoc = QJsonDocument::fromVariant(message->fileInfo());
    query->addBindValue(fileInfoDoc.toJson(QJsonDocument::Compact));
    
    query->addBindValue(message->fileUrl().toString());
    query->addBindValue(message->fileSize());
    query->addBindValue(message->mimeType());
    
    QJsonDocument propertiesDoc = QJsonDocument::fromVariant(message->properties());
    query->addBindValue(propertiesDoc.toJson(QJsonDocument::Compact));
    
    query->addBindValue(message->id());
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    // 更新缓存
    if (d->cacheEnabled) {
        removeFromCache(message->id());
        addToCache(message->clone(this));
    }
    
    emit messageUpdated(message->id());
    return Success;
}

IMessageStorage::OperationResult MessageStorage::deleteMessage(const QString& messageId)
{
    if (messageId.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("DELETE FROM messages WHERE id = ?");
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(messageId);
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    // 从缓存移除
    if (d->cacheEnabled) {
        removeFromCache(messageId);
    }
    
    d->totalMessages--;
    updateStatistics();
    
    emit messageDeleted(messageId);
    return Success;
}

IMessageStorage::OperationResult MessageStorage::deleteRoomMessages(const QString& roomId)
{
    if (roomId.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("DELETE FROM messages WHERE room_id = ?");
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(roomId);
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    // 清理缓存中的相关消息
    if (d->cacheEnabled) {
        QStringList toRemove;
        for (auto it = d->messageCache.begin(); it != d->messageCache.end(); ++it) {
            if (it.value()->roomId() == roomId) {
                toRemove.append(it.key());
            }
        }
        for (const QString& id : toRemove) {
            removeFromCache(id);
        }
    }
    
    updateStatistics();
    return Success;
}

IMessageStorage::OperationResult MessageStorage::deleteMessagesBefore(const QString& roomId, const QDateTime& before)
{
    if (!isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    QString sql = "DELETE FROM messages WHERE timestamp < ?";
    if (!roomId.isEmpty()) {
        sql += " AND room_id = ?";
    }
    
    auto query = prepareQuery(sql);
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(before);
    if (!roomId.isEmpty()) {
        query->addBindValue(roomId);
    }
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    updateStatistics();
    return Success;
}

int MessageStorage::getMessageCount(const QString& roomId)
{
    if (!isReady()) {
        return 0;
    }
    
    QMutexLocker locker(&d->mutex);
    
    QString sql = "SELECT COUNT(*) FROM messages";
    if (!roomId.isEmpty()) {
        sql += " WHERE room_id = ?";
    }
    
    auto query = prepareQuery(sql);
    if (!query) {
        return 0;
    }
    
    if (!roomId.isEmpty()) {
        query->addBindValue(roomId);
    }
    
    if (!executeQuery(*query) || !query->next()) {
        return 0;
    }
    
    return query->value(0).toInt();
}

QStringList MessageStorage::getRoomList()
{
    QStringList rooms;
    
    if (!isReady()) {
        return rooms;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("SELECT DISTINCT room_id FROM messages ORDER BY room_id");
    if (!query) {
        return rooms;
    }
    
    if (!executeQuery(*query)) {
        return rooms;
    }
    
    while (query->next()) {
        rooms.append(query->value(0).toString());
    }
    
    return rooms;
}

ChatMessage* MessageStorage::getLastMessage(const QString& roomId)
{
    if (roomId.isEmpty() || !isReady()) {
        return nullptr;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("SELECT * FROM messages WHERE room_id = ? ORDER BY timestamp DESC LIMIT 1");
    if (!query) {
        return nullptr;
    }
    
    query->addBindValue(roomId);
    
    if (!executeQuery(*query) || !query->next()) {
        return nullptr;
    }
    
    return createMessageFromQuery(*query);
}

int MessageStorage::getUnreadCount(const QString& roomId, const QString& userId)
{
    if (roomId.isEmpty() || userId.isEmpty() || !isReady()) {
        return 0;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("SELECT COUNT(*) FROM messages WHERE room_id = ? AND sender_id != ? AND is_read = 0");
    if (!query) {
        return 0;
    }
    
    query->addBindValue(roomId);
    query->addBindValue(userId);
    
    if (!executeQuery(*query) || !query->next()) {
        return 0;
    }
    
    return query->value(0).toInt();
}

IMessageStorage::OperationResult MessageStorage::markAsRead(const QString& messageId, const QString& userId)
{
    if (messageId.isEmpty() || userId.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("UPDATE messages SET is_read = 1 WHERE id = ?");
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(messageId);
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    // 更新缓存
    if (d->cacheEnabled) {
        ChatMessage* cached = getFromCache(messageId);
        if (cached) {
            cached->setRead(true);
        }
    }
    
    return Success;
}

IMessageStorage::OperationResult MessageStorage::markRoomAsRead(const QString& roomId, const QString& userId)
{
    if (roomId.isEmpty() || userId.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("UPDATE messages SET is_read = 1 WHERE room_id = ? AND sender_id != ?");
    if (!query) {
        return Failed;
    }
    
    query->addBindValue(roomId);
    query->addBindValue(userId);
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    return Success;
}

QVariantMap MessageStorage::getStatistics()
{
    QMutexLocker locker(&d->mutex);
    
    QVariantMap stats;
    stats["totalMessages"] = d->totalMessages;
    stats["totalSize"] = d->totalSize;
    stats["databasePath"] = d->databasePath;
    stats["cacheEnabled"] = d->cacheEnabled;
    stats["cacheSize"] = d->messageCache.size();
    stats["cacheHitRate"] = cacheHitRate();
    stats["maxStorageSize"] = d->maxStorageSize;
    stats["availableSpace"] = availableSpace();
    
    return stats;
}

IMessageStorage::OperationResult MessageStorage::compact()
{
    if (!isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("VACUUM");
    if (!query) {
        return Failed;
    }
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    updateStatistics();
    return Success;
}

IMessageStorage::OperationResult MessageStorage::backup(const QString& backupPath)
{
    if (backupPath.isEmpty() || !isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 确保备份目录存在
    QFileInfo fileInfo(backupPath);
    QDir().mkpath(fileInfo.absolutePath());
    
    // 复制数据库文件
    if (!QFile::copy(d->databasePath, backupPath)) {
        return Failed;
    }
    
    emit backupCompleted(backupPath, true);
    return Success;
}

IMessageStorage::OperationResult MessageStorage::restore(const QString& backupPath)
{
    if (backupPath.isEmpty() || !QFile::exists(backupPath)) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    // 关闭当前数据库
    if (d->database.isOpen()) {
        d->database.close();
    }
    
    // 备份当前数据库
    QString currentBackup = d->databasePath + ".backup";
    QFile::copy(d->databasePath, currentBackup);
    
    // 恢复数据库
    if (!QFile::copy(backupPath, d->databasePath)) {
        // 恢复失败，还原原数据库
        QFile::copy(currentBackup, d->databasePath);
        QFile::remove(currentBackup);
        return Failed;
    }
    
    // 重新初始化数据库
    if (!initializeDatabase()) {
        // 初始化失败，还原原数据库
        QFile::copy(currentBackup, d->databasePath);
        QFile::remove(currentBackup);
        initializeDatabase();
        return Failed;
    }
    
    QFile::remove(currentBackup);
    updateStatistics();
    
    emit restoreCompleted(backupPath, true);
    return Success;
}

void MessageStorage::cleanupOldMessages(int days)
{
    if (!isReady()) {
        return;
    }
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-days);
    deleteMessagesBefore(QString(), cutoffDate);
}

void MessageStorage::optimize()
{
    if (!isReady()) {
        return;
    }
    
    compact();
    rebuildIndexes();
    analyzeDatabase();
}

void MessageStorage::refreshCache()
{
    QMutexLocker locker(&d->mutex);
    
    // 清空缓存
    qDeleteAll(d->messageCache.values());
    d->messageCache.clear();
    d->cacheOrder.clear();
    d->cacheHits = 0;
    d->cacheMisses = 0;
}// 扩展功能实现

QString MessageStorage::databasePath() const
{
    return d->databasePath;
}

void MessageStorage::setDatabasePath(const QString& path)
{
    if (d->databasePath != path) {
        d->databasePath = path;
        emit databasePathChanged(path);
    }
}

qint64 MessageStorage::totalSize() const
{
    return d->totalSize;
}

int MessageStorage::messageCount() const
{
    return d->totalMessages;
}

bool MessageStorage::isCacheEnabled() const
{
    return d->cacheEnabled;
}

void MessageStorage::setCacheEnabled(bool enabled)
{
    if (d->cacheEnabled != enabled) {
        d->cacheEnabled = enabled;
        if (!enabled) {
            clearCache();
        }
        emit cacheEnabledChanged(enabled);
    }
}

int MessageStorage::cacheLimit() const
{
    return d->cacheLimit;
}

void MessageStorage::setCacheLimit(int limit)
{
    d->cacheLimit = limit;
    if (d->messageCache.size() > limit) {
        cleanupCache();
    }
}

double MessageStorage::cacheHitRate() const
{
    int total = d->cacheHits + d->cacheMisses;
    return total > 0 ? static_cast<double>(d->cacheHits) / total : 0.0;
}

void MessageStorage::setAutoCleanupInterval(int hours)
{
    d->autoCleanupInterval = hours;
    if (hours > 0) {
        d->autoCleanupTimer->start(hours * 60 * 60 * 1000);
    } else {
        d->autoCleanupTimer->stop();
    }
}

int MessageStorage::autoCleanupInterval() const
{
    return d->autoCleanupInterval;
}

void MessageStorage::setMaxStorageSize(qint64 size)
{
    d->maxStorageSize = size;
}

qint64 MessageStorage::maxStorageSize() const
{
    return d->maxStorageSize;
}

bool MessageStorage::hasEnoughSpace() const
{
    return d->totalSize < d->maxStorageSize;
}

qint64 MessageStorage::availableSpace() const
{
    return qMax(0LL, d->maxStorageSize - d->totalSize);
}

IMessageStorage::OperationResult MessageStorage::performMaintenance()
{
    if (!isReady()) {
        return Failed;
    }
    
    bool success = true;
    
    // 压缩数据库
    if (compact() != Success) {
        success = false;
    }
    
    // 重建索引
    if (rebuildIndexes() != Success) {
        success = false;
    }
    
    // 分析数据库
    if (analyzeDatabase() != Success) {
        success = false;
    }
    
    // 检查完整性
    if (!checkIntegrity()) {
        success = false;
    }
    
    emit maintenanceCompleted(success);
    return success ? Success : Failed;
}

IMessageStorage::OperationResult MessageStorage::rebuildIndexes()
{
    if (!isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("REINDEX");
    if (!query) {
        return Failed;
    }
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    return Success;
}

IMessageStorage::OperationResult MessageStorage::analyzeDatabase()
{
    if (!isReady()) {
        return Failed;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("ANALYZE");
    if (!query) {
        return Failed;
    }
    
    if (!executeQuery(*query)) {
        return Failed;
    }
    
    return Success;
}

bool MessageStorage::checkIntegrity()
{
    if (!isReady()) {
        return false;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("PRAGMA integrity_check");
    if (!query) {
        return false;
    }
    
    if (!executeQuery(*query) || !query->next()) {
        return false;
    }
    
    QString result = query->value(0).toString();
    return result == "ok";
}

IMessageStorage::OperationResult MessageStorage::repairDatabase()
{
    // SQLite 自动修复功能有限，主要通过重建来修复
    return performMaintenance();
}

void MessageStorage::clearCache()
{
    QMutexLocker locker(&d->mutex);
    
    qDeleteAll(d->messageCache.values());
    d->messageCache.clear();
    d->cacheOrder.clear();
    d->cacheHits = 0;
    d->cacheMisses = 0;
}

void MessageStorage::preloadRoomMessages(const QString& roomId, int limit)
{
    if (!d->cacheEnabled || roomId.isEmpty()) {
        return;
    }
    
    QList<ChatMessage*> messages = getRoomMessages(roomId, limit);
    for (ChatMessage* message : messages) {
        addToCache(message);
    }
}

void MessageStorage::performAutoCleanup()
{
    if (d->autoCleanupInterval > 0) {
        cleanupOldMessages(30); // 默认清理30天前的消息
    }
}

void MessageStorage::syncToDisk()
{
    if (!isReady()) {
        return;
    }
    
    QMutexLocker locker(&d->mutex);
    
    auto query = prepareQuery("PRAGMA synchronous = FULL");
    if (query) {
        executeQuery(*query);
    }
}

void MessageStorage::handleAutoCleanupTimer()
{
    performAutoCleanup();
}

void MessageStorage::handleCacheCleanupTimer()
{
    cleanupCache();
}

// 私有方法实现
bool MessageStorage::initializeDatabase()
{
    // 确保数据库目录存在
    QFileInfo fileInfo(d->databasePath);
    QDir().mkpath(fileInfo.absolutePath());
    
    // 创建数据库连接
    d->database = QSqlDatabase::addDatabase("QSQLITE", "MessageStorage");
    d->database.setDatabaseName(d->databasePath);
    
    if (!d->database.open()) {
        handleDatabaseError("Failed to open database: " + d->database.lastError().text());
        return false;
    }
    
    // 创建表和索引
    if (!createTables()) {
        return false;
    }
    
    if (!createIndexes()) {
        return false;
    }
    
    // 检查数据库版本并升级
    int currentVersion = getDatabaseVersion();
    const int targetVersion = 1;
    
    if (currentVersion < targetVersion) {
        if (!upgradeSchema(currentVersion, targetVersion)) {
            return false;
        }
        setDatabaseVersion(targetVersion);
    }
    
    return true;
}

bool MessageStorage::createTables()
{
    QStringList createStatements = {
        "CREATE TABLE IF NOT EXISTS messages ("
        "id TEXT PRIMARY KEY, "
        "content TEXT NOT NULL, "
        "type INTEGER NOT NULL, "
        "sender_id TEXT NOT NULL, "
        "sender_name TEXT, "
        "room_id TEXT NOT NULL, "
        "timestamp DATETIME NOT NULL, "
        "status INTEGER NOT NULL, "
        "priority INTEGER DEFAULT 1, "
        "is_read BOOLEAN DEFAULT 0, "
        "is_edited BOOLEAN DEFAULT 0, "
        "edited_timestamp DATETIME, "
        "file_info TEXT, "
        "file_url TEXT, "
        "file_size INTEGER DEFAULT 0, "
        "mime_type TEXT, "
        "properties TEXT"
        ")",
        
        "CREATE TABLE IF NOT EXISTS read_status ("
        "message_id TEXT, "
        "user_id TEXT, "
        "read_timestamp DATETIME, "
        "PRIMARY KEY (message_id, user_id), "
        "FOREIGN KEY (message_id) REFERENCES messages(id) ON DELETE CASCADE"
        ")",
        
        "CREATE TABLE IF NOT EXISTS metadata ("
        "key TEXT PRIMARY KEY, "
        "value TEXT"
        ")"
    };
    
    for (const QString& sql : createStatements) {
        auto query = prepareQuery(sql);
        if (!query || !executeQuery(*query)) {
            return false;
        }
    }
    
    return true;
}

bool MessageStorage::createIndexes()
{
    QStringList indexStatements = {
        "CREATE INDEX IF NOT EXISTS idx_messages_room_id ON messages(room_id)",
        "CREATE INDEX IF NOT EXISTS idx_messages_timestamp ON messages(timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_messages_sender_id ON messages(sender_id)",
        "CREATE INDEX IF NOT EXISTS idx_messages_room_timestamp ON messages(room_id, timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_messages_content ON messages(content)",
        "CREATE INDEX IF NOT EXISTS idx_read_status_user ON read_status(user_id)"
    };
    
    for (const QString& sql : indexStatements) {
        auto query = prepareQuery(sql);
        if (!query || !executeQuery(*query)) {
            return false;
        }
    }
    
    return true;
}

bool MessageStorage::upgradeSchema(int fromVersion, int toVersion)
{
    // 目前只有版本1，未来版本升级在这里实现
    Q_UNUSED(fromVersion)
    Q_UNUSED(toVersion)
    return true;
}

int MessageStorage::getDatabaseVersion()
{
    auto query = prepareQuery("SELECT value FROM metadata WHERE key = 'version'");
    if (!query || !executeQuery(*query) || !query->next()) {
        return 0;
    }
    
    return query->value(0).toInt();
}

void MessageStorage::setDatabaseVersion(int version)
{
    auto query = prepareQuery("INSERT OR REPLACE INTO metadata (key, value) VALUES ('version', ?)");
    if (query) {
        query->addBindValue(version);
        executeQuery(*query);
    }
}

bool MessageStorage::executeQuery(QSqlQuery& query)
{
    if (!query.exec()) {
        handleDatabaseError("Query execution failed: " + query.lastError().text());
        return false;
    }
    return true;
}

std::unique_ptr<QSqlQuery> MessageStorage::prepareQuery(const QString& sql)
{
    auto query = std::make_unique<QSqlQuery>(d->database);
    if (!query->prepare(sql)) {
        handleDatabaseError("Query preparation failed: " + query->lastError().text());
        return nullptr;
    }
    return query;
}

ChatMessage* MessageStorage::createMessageFromQuery(QSqlQuery& query)
{
    ChatMessage* message = new ChatMessage(this);
    
    // 基本属性
    message->d->id = query.value("id").toString();
    message->setContent(query.value("content").toString());
    message->setType(static_cast<ChatMessage::MessageType>(query.value("type").toInt()));
    message->setSenderId(query.value("sender_id").toString());
    message->setSenderName(query.value("sender_name").toString());
    message->setRoomId(query.value("room_id").toString());
    message->setTimestamp(query.value("timestamp").toDateTime());
    message->setStatus(static_cast<ChatMessage::MessageStatus>(query.value("status").toInt()));
    message->setPriority(static_cast<ChatMessage::MessagePriority>(query.value("priority").toInt()));
    message->setRead(query.value("is_read").toBool());
    
    // 编辑信息
    bool isEdited = query.value("is_edited").toBool();
    if (isEdited) {
        message->d->isEdited = true;
        message->d->editedTimestamp = query.value("edited_timestamp").toDateTime();
    }
    
    // 文件信息
    QString fileInfoJson = query.value("file_info").toString();
    if (!fileInfoJson.isEmpty()) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(fileInfoJson.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            message->setFileInfo(doc.toVariant().toMap());
        }
    }
    
    message->setFileUrl(QUrl(query.value("file_url").toString()));
    message->setFileSize(query.value("file_size").toLongLong());
    message->setMimeType(query.value("mime_type").toString());
    
    // 扩展属性
    QString propertiesJson = query.value("properties").toString();
    if (!propertiesJson.isEmpty()) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(propertiesJson.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            message->setProperties(doc.toVariant().toMap());
        }
    }
    
    return message;
}

void MessageStorage::bindMessageToQuery(QSqlQuery& query, ChatMessage* message)
{
    query.addBindValue(message->id());
    query.addBindValue(message->content());
    query.addBindValue(static_cast<int>(message->type()));
    query.addBindValue(message->senderId());
    query.addBindValue(message->senderName());
    query.addBindValue(message->roomId());
    query.addBindValue(message->timestamp());
    query.addBindValue(static_cast<int>(message->status()));
    query.addBindValue(static_cast<int>(message->priority()));
    query.addBindValue(message->isRead());
    query.addBindValue(message->isEdited());
    query.addBindValue(message->editedTimestamp());
    
    // 文件信息
    QJsonDocument fileInfoDoc = QJsonDocument::fromVariant(message->fileInfo());
    query.addBindValue(fileInfoDoc.toJson(QJsonDocument::Compact));
    
    query.addBindValue(message->fileUrl().toString());
    query.addBindValue(message->fileSize());
    query.addBindValue(message->mimeType());
    
    // 扩展属性
    QJsonDocument propertiesDoc = QJsonDocument::fromVariant(message->properties());
    query.addBindValue(propertiesDoc.toJson(QJsonDocument::Compact));
}

ChatMessage* MessageStorage::getFromCache(const QString& messageId)
{
    auto it = d->messageCache.find(messageId);
    if (it != d->messageCache.end()) {
        // 更新访问顺序
        d->cacheOrder.removeAll(messageId);
        d->cacheOrder.prepend(messageId);
        return it.value();
    }
    return nullptr;
}

void MessageStorage::addToCache(ChatMessage* message)
{
    if (!message) {
        return;
    }
    
    const QString& messageId = message->id();
    
    // 如果已存在，先移除
    if (d->messageCache.contains(messageId)) {
        removeFromCache(messageId);
    }
    
    // 检查缓存大小限制
    while (d->messageCache.size() >= d->cacheLimit) {
        QString oldestId = d->cacheOrder.takeLast();
        delete d->messageCache.take(oldestId);
    }
    
    // 添加到缓存
    d->messageCache[messageId] = message;
    d->cacheOrder.prepend(messageId);
}

void MessageStorage::removeFromCache(const QString& messageId)
{
    auto it = d->messageCache.find(messageId);
    if (it != d->messageCache.end()) {
        delete it.value();
        d->messageCache.erase(it);
        d->cacheOrder.removeAll(messageId);
    }
}

void MessageStorage::cleanupCache()
{
    while (d->messageCache.size() > d->cacheLimit) {
        QString oldestId = d->cacheOrder.takeLast();
        delete d->messageCache.take(oldestId);
    }
}

void MessageStorage::updateStatistics()
{
    // 更新消息总数
    d->totalMessages = getMessageCount();
    
    // 更新总大小
    QFileInfo fileInfo(d->databasePath);
    d->totalSize = fileInfo.size();
    
    emit totalSizeChanged(d->totalSize);
    emit messageCountChanged(d->totalMessages);
}

void MessageStorage::setStatus(StorageStatus status)
{
    if (d->currentStatus != status) {
        d->currentStatus = status;
        emit statusChanged(status);
    }
}

void MessageStorage::handleDatabaseError(const QString& error)
{
    qWarning() << "MessageStorage database error:" << error;
    emit storageError(error);
}

bool MessageStorage::validateMessage(ChatMessage* message)
{
    if (!message) {
        return false;
    }
    
    return message->validate();
}

QString MessageStorage::generateBackupFileName(const QString& basePath)
{
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMdd_hhmmss");
    return QString("%1_backup_%2.db").arg(basePath).arg(timestamp);
}