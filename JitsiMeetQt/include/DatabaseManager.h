#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMutex>

/**
 * @brief SQLite数据库管理类
 * 
 * 负责管理会议历史记录的持久化存储，包括：
 * - 数据库初始化和连接管理
 * - 会议记录的增删改查操作
 * - 数据库版本管理和升级
 * - 线程安全的数据库操作
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取数据库管理器单例实例
     * @return DatabaseManager实例指针
     */
    static DatabaseManager* instance();
    
    /**
     * @brief 析构函数
     */
    ~DatabaseManager();
    
    /**
     * @brief 初始化数据库
     * @param dbPath 数据库文件路径，如果为空则使用默认路径
     * @return 初始化是否成功
     */
    bool initialize(const QString& dbPath = QString());
    
    /**
     * @brief 关闭数据库连接
     */
    void close();
    
    /**
     * @brief 添加会议记录
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @param displayName 显示名称
     * @param timestamp 时间戳（可选，默认为当前时间）
     * @return 添加是否成功
     */
    bool addMeetingRecord(const QString& roomName, const QString& serverUrl, 
                         const QString& displayName, const QDateTime& timestamp = QDateTime::currentDateTime());
    
    /**
     * @brief 删除会议记录
     * @param id 记录ID
     * @return 删除是否成功
     */
    bool deleteMeetingRecord(int id);
    
    /**
     * @brief 根据房间名和服务器URL删除会议记录
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @return 删除是否成功
     */
    bool deleteMeetingRecord(const QString& roomName, const QString& serverUrl);
    
    /**
     * @brief 获取最近的会议记录
     * @param maxCount 最大记录数量
     * @return 会议记录列表（JSON格式）
     */
    QJsonObject getRecentMeetings(int maxCount = 50) const;
    
    /**
     * @brief 清除所有会议历史记录
     * @return 清除是否成功
     */
    bool clearMeetingHistory();
    
    /**
     * @brief 更新会议记录的最后访问时间
     * @param roomName 房间名称
     * @param serverUrl 服务器URL
     * @return 更新是否成功
     */
    bool updateMeetingLastAccess(const QString& roomName, const QString& serverUrl);
    
    /**
     * @brief 检查数据库是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;
    
    /**
     * @brief 获取数据库文件路径
     * @return 数据库文件路径
     */
    QString getDatabasePath() const;

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit DatabaseManager(QObject *parent = nullptr);
    
    /**
     * @brief 创建数据库表
     * @return 创建是否成功
     */
    bool createTables();
    
    /**
     * @brief 检查并升级数据库版本
     * @return 升级是否成功
     */
    bool upgradeDatabase();
    
    /**
     * @brief 获取数据库版本
     * @return 数据库版本号
     */
    int getDatabaseVersion() const;
    
    /**
     * @brief 设置数据库版本
     * @param version 版本号
     * @return 设置是否成功
     */
    bool setDatabaseVersion(int version);
    
    /**
     * @brief 获取默认数据库路径
     * @return 默认数据库文件路径
     */
    QString getDefaultDatabasePath() const;
    
    /**
     * @brief 执行SQL查询并处理错误
     * @param query SQL查询对象
     * @param operation 操作描述（用于日志）
     * @return 执行是否成功
     */
    bool executeQuery(QSqlQuery& query, const QString& operation) const;

private:
    static DatabaseManager* m_instance;    ///< 单例实例
    QSqlDatabase m_database;               ///< 数据库连接
    QString m_databasePath;                ///< 数据库文件路径
    bool m_initialized;                    ///< 是否已初始化
    mutable QMutex m_mutex;                ///< 线程安全互斥锁
    
    // 数据库常量
    static const QString DATABASE_NAME;    ///< 数据库连接名称
    static const QString TABLE_MEETINGS;   ///< 会议记录表名
    static const QString TABLE_VERSION;    ///< 版本信息表名
    static const int CURRENT_DB_VERSION;   ///< 当前数据库版本
};

#endif // DATABASEMANAGER_H