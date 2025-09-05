#include "DatabaseManager.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlRecord>
#include <QVariant>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QCoreApplication>

// 静态成员初始化
DatabaseManager* DatabaseManager::m_instance = nullptr;
const QString DatabaseManager::DATABASE_NAME = "JitsiMeetQt_DB";
const QString DatabaseManager::TABLE_MEETINGS = "meeting_history";
const QString DatabaseManager::TABLE_VERSION = "db_version";
const int DatabaseManager::CURRENT_DB_VERSION = 1;

/**
 * @brief 获取数据库管理器单例实例
 */
DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

/**
 * @brief 私有构造函数（单例模式）
 */
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
    // 构造函数中不进行数据库初始化，由外部调用initialize()方法
}

/**
 * @brief 析构函数
 */
DatabaseManager::~DatabaseManager()
{
    close();
}

/**
 * @brief 初始化数据库
 */
bool DatabaseManager::initialize(const QString& dbPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        Logger::instance().info("数据库已经初始化");
        return true;
    }
    
    // 确定数据库路径
    m_databasePath = dbPath.isEmpty() ? getDefaultDatabasePath() : dbPath;
    Logger::instance().info("数据库路径: " + m_databasePath);
    
    // 确保数据库目录存在
    QDir dbDir = QFileInfo(m_databasePath).absoluteDir();
    if (!dbDir.exists()) {
        if (!dbDir.mkpath(".")) {
            Logger::instance().error("无法创建数据库目录: " + dbDir.absolutePath());
            return false;
        }
    }
    
    // 检查可用的SQL驱动
    QStringList availableDrivers = QSqlDatabase::drivers();
    Logger::instance().info("可用的SQL驱动: " + availableDrivers.join(", "));
    
    if (!availableDrivers.contains("QSQLITE")) {
        Logger::instance().error("QSQLITE驱动不可用！可用驱动: " + availableDrivers.join(", "));
        return false;
    }
    
    // 创建数据库连接
    Logger::instance().info("正在创建数据库连接...");
    m_database = QSqlDatabase::addDatabase("QSQLITE", DATABASE_NAME);
    m_database.setDatabaseName(m_databasePath);
    Logger::instance().info("数据库名称已设置: " + m_databasePath);
    
    // 检查数据库对象是否有效
    if (!m_database.isValid()) {
        Logger::instance().error("数据库对象无效: " + m_database.lastError().text());
        return false;
    }
    
    if (!m_database.open()) {
        Logger::instance().error("无法打开数据库: " + m_database.lastError().text());
        return false;
    }
    
    Logger::instance().info("数据库连接成功: " + m_databasePath);
    
    // 创建表结构
    if (!createTables()) {
        Logger::instance().error("创建数据库表失败");
        close();
        return false;
    }
    
    // 检查并升级数据库
    if (!upgradeDatabase()) {
        Logger::instance().error("数据库升级失败");
        close();
        return false;
    }
    
    m_initialized = true;
    Logger::instance().info("数据库初始化完成");
    return true;
}

/**
 * @brief 关闭数据库连接
 */
void DatabaseManager::close()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_database.isOpen()) {
        m_database.close();
        Logger::instance().info("数据库连接已关闭");
    }
    
    QSqlDatabase::removeDatabase(DATABASE_NAME);
    m_initialized = false;
}

/**
 * @brief 创建数据库表
 */
bool DatabaseManager::createTables()
{
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    
    // 创建会议历史记录表
    QString createMeetingsTable = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_name TEXT NOT NULL, "
        "server_url TEXT NOT NULL, "
        "display_name TEXT, "
        "full_url TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "last_access DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "UNIQUE(room_name, server_url)"
        ")").arg(TABLE_MEETINGS);
    
    query.prepare(createMeetingsTable);
    if (!executeQuery(query, "创建会议历史记录表")) {
        return false;
    }
    
    // 创建版本信息表
    QString createVersionTable = QString(
        "CREATE TABLE IF NOT EXISTS %1 ("
        "version INTEGER PRIMARY KEY"
        ")").arg(TABLE_VERSION);
    
    query.prepare(createVersionTable);
    if (!executeQuery(query, "创建版本信息表")) {
        return false;
    }
    
    // 创建索引以提高查询性能
    QString createIndex = QString(
        "CREATE INDEX IF NOT EXISTS idx_last_access ON %1 (last_access DESC)"
    ).arg(TABLE_MEETINGS);
    
    query.prepare(createIndex);
    if (!executeQuery(query, "创建索引")) {
        return false;
    }
    
    Logger::instance().info("数据库表创建成功");
    return true;
}

/**
 * @brief 检查并升级数据库版本
 */
bool DatabaseManager::upgradeDatabase()
{
    int currentVersion = getDatabaseVersion();
    
    if (currentVersion == 0) {
        // 首次创建，设置当前版本
        return setDatabaseVersion(CURRENT_DB_VERSION);
    }
    
    if (currentVersion < CURRENT_DB_VERSION) {
        Logger::instance().info(QString("数据库需要升级：从版本 %1 到版本 %2")
                               .arg(currentVersion).arg(CURRENT_DB_VERSION));
        
        // 这里可以添加数据库升级逻辑
        // 目前只有一个版本，暂时不需要升级逻辑
        
        return setDatabaseVersion(CURRENT_DB_VERSION);
    }
    
    return true;
}

/**
 * @brief 获取数据库版本
 */
int DatabaseManager::getDatabaseVersion() const
{
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString("SELECT version FROM %1 LIMIT 1").arg(TABLE_VERSION));
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0; // 表示未设置版本或表不存在
}

/**
 * @brief 设置数据库版本
 */
bool DatabaseManager::setDatabaseVersion(int version)
{
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    
    // 先删除旧版本记录
    query.prepare(QString("DELETE FROM %1").arg(TABLE_VERSION));
    if (!executeQuery(query, "删除旧版本记录")) {
        return false;
    }
    
    // 插入新版本记录
    query.prepare(QString("INSERT INTO %1 (version) VALUES (?)").arg(TABLE_VERSION));
    query.addBindValue(version);
    
    return executeQuery(query, "设置数据库版本");
}

/**
 * @brief 添加会议记录
 */
bool DatabaseManager::addMeetingRecord(const QString& roomName, const QString& serverUrl, 
                                      const QString& displayName, const QDateTime& timestamp)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        return false;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    
    // 构建完整URL
    QString fullUrl = serverUrl;
    if (!fullUrl.endsWith("/")) {
        fullUrl += "/";
    }
    fullUrl += roomName;
    
    // 使用REPLACE语句，如果记录已存在则更新，否则插入
    query.prepare(QString(
        "INSERT OR REPLACE INTO %1 "
        "(room_name, server_url, display_name, full_url, created_at, last_access) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ).arg(TABLE_MEETINGS));
    
    query.addBindValue(roomName);
    query.addBindValue(serverUrl);
    query.addBindValue(displayName);
    query.addBindValue(fullUrl);
    query.addBindValue(timestamp.toString(Qt::ISODate));
    query.addBindValue(timestamp.toString(Qt::ISODate));
    
    if (executeQuery(query, "添加会议记录")) {
        Logger::instance().info(QString("会议记录已添加: %1@%2").arg(roomName, serverUrl));
        return true;
    }
    
    return false;
}

/**
 * @brief 删除会议记录（根据ID）
 */
bool DatabaseManager::deleteMeetingRecord(int id)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        return false;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(TABLE_MEETINGS));
    query.addBindValue(id);
    
    if (executeQuery(query, "删除会议记录")) {
        Logger::instance().info(QString("会议记录已删除: ID=%1").arg(id));
        return true;
    }
    
    return false;
}

/**
 * @brief 删除会议记录（根据房间名和服务器URL）
 */
bool DatabaseManager::deleteMeetingRecord(const QString& roomName, const QString& serverUrl)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        return false;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString("DELETE FROM %1 WHERE room_name = ? AND server_url = ?").arg(TABLE_MEETINGS));
    query.addBindValue(roomName);
    query.addBindValue(serverUrl);
    
    if (executeQuery(query, "删除会议记录")) {
        Logger::instance().info(QString("会议记录已删除: %1@%2").arg(roomName, serverUrl));
        return true;
    }
    
    return false;
}

/**
 * @brief 获取最近的会议记录
 */
QJsonObject DatabaseManager::getRecentMeetings(int maxCount) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject result;
    QJsonArray meetings;
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        result["meetings"] = meetings;
        result["count"] = 0;
        return result;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString(
        "SELECT id, room_name, server_url, display_name, full_url, created_at, last_access "
        "FROM %1 ORDER BY last_access DESC LIMIT ?"
    ).arg(TABLE_MEETINGS));
    query.addBindValue(maxCount);
    
    if (!query.exec()) {
        Logger::instance().error("查询会议记录失败: " + query.lastError().text());
        result["meetings"] = meetings;
        result["count"] = 0;
        return result;
    }
    
    while (query.next()) {
        QJsonObject meeting;
        meeting["id"] = query.value("id").toInt();
        meeting["roomName"] = query.value("room_name").toString();
        meeting["serverUrl"] = query.value("server_url").toString();
        meeting["displayName"] = query.value("display_name").toString();
        meeting["fullUrl"] = query.value("full_url").toString();
        meeting["timestamp"] = query.value("created_at").toString();
        meeting["lastAccess"] = query.value("last_access").toString();
        
        meetings.append(meeting);
    }
    
    result["meetings"] = meetings;
    result["count"] = meetings.size();
    
    Logger::instance().info(QString("查询到 %1 条会议记录").arg(meetings.size()));
    return result;
}

/**
 * @brief 清除所有会议历史记录
 */
bool DatabaseManager::clearMeetingHistory()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        return false;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString("DELETE FROM %1").arg(TABLE_MEETINGS));
    
    if (executeQuery(query, "清除会议历史记录")) {
        Logger::instance().info("所有会议历史记录已清除");
        return true;
    }
    
    return false;
}

/**
 * @brief 更新会议记录的最后访问时间
 */
bool DatabaseManager::updateMeetingLastAccess(const QString& roomName, const QString& serverUrl)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        Logger::instance().error("数据库未初始化");
        return false;
    }
    
    QSqlQuery query(QSqlDatabase::database(DATABASE_NAME));
    query.prepare(QString(
        "UPDATE %1 SET last_access = ? WHERE room_name = ? AND server_url = ?"
    ).arg(TABLE_MEETINGS));
    
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(roomName);
    query.addBindValue(serverUrl);
    
    if (executeQuery(query, "更新最后访问时间")) {
        Logger::instance().info(QString("已更新最后访问时间: %1@%2").arg(roomName, serverUrl));
        return true;
    }
    
    return false;
}

/**
 * @brief 检查数据库是否已初始化
 */
bool DatabaseManager::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_initialized;
}

/**
 * @brief 获取数据库文件路径
 */
QString DatabaseManager::getDatabasePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_databasePath;
}

/**
 * @brief 获取默认数据库路径
 */
QString DatabaseManager::getDefaultDatabasePath() const
{
    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    return QDir(appDir).absoluteFilePath("db.db");
}

/**
 * @brief 执行SQL查询并处理错误
 */
bool DatabaseManager::executeQuery(QSqlQuery& query, const QString& operation) const
{
    if (!query.exec()) {
        Logger::instance().error(QString("%1失败: %2").arg(operation, query.lastError().text()));
        return false;
    }
    return true;
}