#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QTimer>
#include <memory>

// Forward declarations
class ChatMessage;
class MessageStorage;

/**
 * @brief 历史管理器类
 * 
 * HistoryManager负责管理聊天历史记录，包括历史记录的
 * 加载、搜索、清理和导出功能。
 */
class HistoryManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int retentionDays READ retentionDays WRITE setRetentionDays NOTIFY retentionDaysChanged)
    Q_PROPERTY(int maxMessages READ maxMessages WRITE setMaxMessages NOTIFY maxMessagesChanged)
    Q_PROPERTY(bool autoCleanup READ isAutoCleanupEnabled WRITE setAutoCleanupEnabled NOTIFY autoCleanupChanged)
    Q_PROPERTY(int totalMessages READ totalMessages NOTIFY totalMessagesChanged)

public:
    /**
     * @brief 搜索选项枚举
     */
    enum SearchOption {
        CaseSensitive = 0x01,       ///< 区分大小写
        WholeWords = 0x02,          ///< 全词匹配
        RegularExpression = 0x04,   ///< 正则表达式
        IncludeDeleted = 0x08       ///< 包含已删除消息
    };
    Q_ENUM(SearchOption)
    Q_DECLARE_FLAGS(SearchOptions, SearchOption)

    /**
     * @brief 导出格式枚举
     */
    enum ExportFormat {
        PlainText,          ///< 纯文本
        HTML,               ///< HTML格式
        JSON,               ///< JSON格式
        CSV,                ///< CSV格式
        XML                 ///< XML格式
    };
    Q_ENUM(ExportFormat)

    /**
     * @brief 清理策略枚举
     */
    enum CleanupStrategy {
        ByAge,              ///< 按时间清理
        ByCount,            ///< 按数量清理
        BySize,             ///< 按大小清理
        Manual              ///< 手动清理
    };
    Q_ENUM(CleanupStrategy)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit HistoryManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~HistoryManager();

    /**
     * @brief 初始化历史管理器
     * @param config 配置参数
     * @return 初始化是否成功
     */
    bool initialize(const QVariantMap& config = QVariantMap());

    /**
     * @brief 设置消息存储
     * @param storage 存储对象
     */
    void setMessageStorage(MessageStorage* storage);

    /**
     * @brief 获取消息存储
     * @return 存储对象
     */
    MessageStorage* messageStorage() const;

    /**
     * @brief 检查是否启用
     * @return 是否启用
     */
    bool isEnabled() const;

    /**
     * @brief 设置启用状态
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

    /**
     * @brief 获取保留天数
     * @return 保留天数
     */
    int retentionDays() const;

    /**
     * @brief 设置保留天数
     * @param days 保留天数
     */
    void setRetentionDays(int days);

    /**
     * @brief 获取最大消息数量
     * @return 最大消息数量
     */
    int maxMessages() const;

    /**
     * @brief 设置最大消息数量
     * @param maxMessages 最大消息数量
     */
    void setMaxMessages(int maxMessages);

    /**
     * @brief 检查是否启用自动清理
     * @return 是否启用自动清理
     */
    bool isAutoCleanupEnabled() const;

    /**
     * @brief 设置自动清理启用状态
     * @param enabled 是否启用
     */
    void setAutoCleanupEnabled(bool enabled);

    /**
     * @brief 获取清理策略
     * @return 清理策略
     */
    CleanupStrategy cleanupStrategy() const;

    /**
     * @brief 设置清理策略
     * @param strategy 清理策略
     */
    void setCleanupStrategy(CleanupStrategy strategy);

    /**
     * @brief 获取自动清理间隔（小时）
     * @return 清理间隔
     */
    int autoCleanupInterval() const;

    /**
     * @brief 设置自动清理间隔（小时）
     * @param hours 清理间隔
     */
    void setAutoCleanupInterval(int hours);

    /**
     * @brief 获取总消息数量
     * @return 总消息数量
     */
    int totalMessages() const;

    /**
     * @brief 获取房间历史记录
     * @param roomId 房间ID
     * @param limit 数量限制
     * @param offset 偏移量
     * @return 消息列表
     */
    QList<ChatMessage*> getRoomHistory(const QString& roomId, int limit = 50, int offset = 0);

    /**
     * @brief 获取时间范围内的历史记录
     * @param roomId 房间ID
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param limit 数量限制
     * @return 消息列表
     */
    QList<ChatMessage*> getHistoryByTimeRange(const QString& roomId, const QDateTime& startTime, const QDateTime& endTime, int limit = 100);

    /**
     * @brief 搜索历史记录
     * @param query 搜索关键词
     * @param roomId 房间ID（可选）
     * @param options 搜索选项
     * @param limit 数量限制
     * @return 匹配的消息列表
     */
    QList<ChatMessage*> searchHistory(const QString& query, const QString& roomId = QString(), SearchOptions options = SearchOptions(), int limit = 50);

    /**
     * @brief 高级搜索
     * @param criteria 搜索条件
     * @return 匹配的消息列表
     */
    QList<ChatMessage*> advancedSearch(const QVariantMap& criteria);

    /**
     * @brief 获取搜索建议
     * @param partialQuery 部分查询
     * @param limit 建议数量限制
     * @return 建议列表
     */
    QStringList getSearchSuggestions(const QString& partialQuery, int limit = 10);

    /**
     * @brief 添加消息到历史记录
     * @param message 消息对象
     * @return 添加是否成功
     */
    bool addMessage(ChatMessage* message);

    /**
     * @brief 更新历史记录中的消息
     * @param message 消息对象
     * @return 更新是否成功
     */
    bool updateMessage(ChatMessage* message);

    /**
     * @brief 从历史记录中删除消息
     * @param messageId 消息ID
     * @return 删除是否成功
     */
    bool deleteMessage(const QString& messageId);

    /**
     * @brief 清除房间历史记录
     * @param roomId 房间ID
     * @return 清除是否成功
     */
    bool clearRoomHistory(const QString& roomId);

    /**
     * @brief 清除所有历史记录
     * @return 清除是否成功
     */
    bool clearAllHistory();

    /**
     * @brief 清除过期历史记录
     * @param days 保留天数
     * @return 清除的消息数量
     */
    int cleanupExpiredHistory(int days = -1);

    /**
     * @brief 导出历史记录
     * @param roomId 房间ID
     * @param filePath 导出文件路径
     * @param format 导出格式
     * @param startTime 开始时间（可选）
     * @param endTime 结束时间（可选）
     * @return 导出是否成功
     */
    bool exportHistory(const QString& roomId, const QString& filePath, ExportFormat format, const QDateTime& startTime = QDateTime(), const QDateTime& endTime = QDateTime());

    /**
     * @brief 导入历史记录
     * @param filePath 导入文件路径
     * @param format 文件格式
     * @return 导入是否成功
     */
    bool importHistory(const QString& filePath, ExportFormat format);

    /**
     * @brief 获取历史统计信息
     * @param roomId 房间ID（可选）
     * @return 统计信息
     */
    QVariantMap getHistoryStatistics(const QString& roomId = QString());

    /**
     * @brief 获取房间列表
     * @return 有历史记录的房间ID列表
     */
    QStringList getRoomsWithHistory();

    /**
     * @brief 获取房间消息数量
     * @param roomId 房间ID
     * @return 消息数量
     */
    int getRoomMessageCount(const QString& roomId);

    /**
     * @brief 获取房间最早消息时间
     * @param roomId 房间ID
     * @return 最早消息时间
     */
    QDateTime getRoomEarliestMessage(const QString& roomId);

    /**
     * @brief 获取房间最新消息时间
     * @param roomId 房间ID
     * @return 最新消息时间
     */
    QDateTime getRoomLatestMessage(const QString& roomId);

    /**
     * @brief 检查历史记录完整性
     * @return 检查结果
     */
    bool checkIntegrity();

    /**
     * @brief 修复历史记录
     * @return 修复结果
     */
    bool repairHistory();

    /**
     * @brief 压缩历史记录
     * @return 压缩结果
     */
    bool compactHistory();

    /**
     * @brief 重建索引
     * @return 重建结果
     */
    bool rebuildIndexes();

public slots:
    /**
     * @brief 执行自动清理
     */
    void performAutoCleanup();

    /**
     * @brief 刷新统计信息
     */
    void refreshStatistics();

    /**
     * @brief 优化历史记录存储
     */
    void optimizeStorage();

    /**
     * @brief 重新加载配置
     */
    void reloadConfiguration();

    /**
     * @brief 暂停自动清理
     */
    void pauseAutoCleanup();

    /**
     * @brief 恢复自动清理
     */
    void resumeAutoCleanup();

signals:
    /**
     * @brief 启用状态改变信号
     * @param enabled 是否启用
     */
    void enabledChanged(bool enabled);

    /**
     * @brief 保留天数改变信号
     * @param days 新的保留天数
     */
    void retentionDaysChanged(int days);

    /**
     * @brief 最大消息数量改变信号
     * @param maxMessages 新的最大消息数量
     */
    void maxMessagesChanged(int maxMessages);

    /**
     * @brief 自动清理状态改变信号
     * @param enabled 是否启用自动清理
     */
    void autoCleanupChanged(bool enabled);

    /**
     * @brief 总消息数量改变信号
     * @param count 新的总消息数量
     */
    void totalMessagesChanged(int count);

    /**
     * @brief 消息添加信号
     * @param message 添加的消息
     */
    void messageAdded(ChatMessage* message);

    /**
     * @brief 消息更新信号
     * @param message 更新的消息
     */
    void messageUpdated(ChatMessage* message);

    /**
     * @brief 消息删除信号
     * @param messageId 删除的消息ID
     */
    void messageDeleted(const QString& messageId);

    /**
     * @brief 房间历史清除信号
     * @param roomId 房间ID
     */
    void roomHistoryCleared(const QString& roomId);

    /**
     * @brief 所有历史清除信号
     */
    void allHistoryCleared();

    /**
     * @brief 清理完成信号
     * @param deletedCount 删除的消息数量
     */
    void cleanupCompleted(int deletedCount);

    /**
     * @brief 导出完成信号
     * @param filePath 导出文件路径
     * @param success 是否成功
     */
    void exportCompleted(const QString& filePath, bool success);

    /**
     * @brief 导入完成信号
     * @param filePath 导入文件路径
     * @param success 是否成功
     * @param importedCount 导入的消息数量
     */
    void importCompleted(const QString& filePath, bool success, int importedCount);

    /**
     * @brief 搜索完成信号
     * @param query 搜索关键词
     * @param resultCount 结果数量
     */
    void searchCompleted(const QString& query, int resultCount);

    /**
     * @brief 统计信息更新信号
     * @param statistics 新的统计信息
     */
    void statisticsUpdated(const QVariantMap& statistics);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 处理自动清理定时器
     */
    void handleAutoCleanupTimer();

    /**
     * @brief 处理统计更新定时器
     */
    void handleStatisticsTimer();

private:
    /**
     * @brief 初始化定时器
     */
    void initializeTimers();

    /**
     * @brief 执行搜索查询
     * @param query 搜索关键词
     * @param roomId 房间ID
     * @param options 搜索选项
     * @param limit 数量限制
     * @return 匹配的消息列表
     */
    QList<ChatMessage*> executeSearch(const QString& query, const QString& roomId, SearchOptions options, int limit);

    /**
     * @brief 构建搜索SQL
     * @param query 搜索关键词
     * @param roomId 房间ID
     * @param options 搜索选项
     * @return SQL语句
     */
    QString buildSearchSQL(const QString& query, const QString& roomId, SearchOptions options);

    /**
     * @brief 导出为纯文本
     * @param messages 消息列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    bool exportAsPlainText(const QList<ChatMessage*>& messages, const QString& filePath);

    /**
     * @brief 导出为HTML
     * @param messages 消息列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    bool exportAsHTML(const QList<ChatMessage*>& messages, const QString& filePath);

    /**
     * @brief 导出为JSON
     * @param messages 消息列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    bool exportAsJSON(const QList<ChatMessage*>& messages, const QString& filePath);

    /**
     * @brief 导出为CSV
     * @param messages 消息列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    bool exportAsCSV(const QList<ChatMessage*>& messages, const QString& filePath);

    /**
     * @brief 导出为XML
     * @param messages 消息列表
     * @param filePath 文件路径
     * @return 导出是否成功
     */
    bool exportAsXML(const QList<ChatMessage*>& messages, const QString& filePath);

    /**
     * @brief 更新内部统计信息
     */
    void updateInternalStatistics();

    /**
     * @brief 验证配置参数
     * @param config 配置参数
     * @return 验证是否通过
     */
    bool validateConfiguration(const QVariantMap& config);

private:
    class Private;
    std::unique_ptr<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HistoryManager::SearchOptions)
Q_DECLARE_METATYPE(HistoryManager::SearchOption)
Q_DECLARE_METATYPE(HistoryManager::SearchOptions)
Q_DECLARE_METATYPE(HistoryManager::ExportFormat)
Q_DECLARE_METATYPE(HistoryManager::CleanupStrategy)

#endif // HISTORYMANAGER_H