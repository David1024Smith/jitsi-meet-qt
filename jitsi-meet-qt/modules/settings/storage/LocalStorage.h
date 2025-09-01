#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QFileSystemWatcher>
#include <QMutex>
#include <memory>

/**
 * @brief 本地存储后端类
 * 
 * 提供本地文件系统的设置存储功能，支持多种文件格式（JSON、INI、XML）。
 * 包含文件监控、原子写入、备份恢复等高级功能。
 */
class LocalStorage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(StorageFormat format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(bool autoBackup READ isAutoBackupEnabled WRITE setAutoBackupEnabled NOTIFY autoBackupChanged)
    Q_PROPERTY(bool fileWatching READ isFileWatchingEnabled WRITE setFileWatchingEnabled NOTIFY fileWatchingChanged)

public:
    /**
     * @brief 存储格式枚举
     */
    enum StorageFormat {
        JsonFormat,     ///< JSON 格式
        IniFormat,      ///< INI 格式
        XmlFormat,      ///< XML 格式
        BinaryFormat    ///< 二进制格式
    };
    Q_ENUM(StorageFormat)

    /**
     * @brief 存储状态枚举
     */
    enum StorageStatus {
        NotInitialized, ///< 未初始化
        Ready,          ///< 就绪状态
        Loading,        ///< 加载中
        Saving,         ///< 保存中
        Error           ///< 错误状态
    };
    Q_ENUM(StorageStatus)

    /**
     * @brief 备份策略枚举
     */
    enum BackupStrategy {
        NoBackup,       ///< 不备份
        SingleBackup,   ///< 单个备份
        MultipleBackup, ///< 多个备份
        TimestampBackup ///< 时间戳备份
    };
    Q_ENUM(BackupStrategy)

    explicit LocalStorage(QObject* parent = nullptr);
    explicit LocalStorage(const QString& filePath, StorageFormat format = JsonFormat, QObject* parent = nullptr);
    ~LocalStorage();

    /**
     * @brief 初始化存储
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 获取存储状态
     * @return 当前状态
     */
    StorageStatus status() const;

    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    QString filePath() const;

    /**
     * @brief 设置文件路径
     * @param path 文件路径
     */
    void setFilePath(const QString& path);

    /**
     * @brief 获取存储格式
     * @return 存储格式
     */
    StorageFormat format() const;

    /**
     * @brief 设置存储格式
     * @param format 存储格式
     */
    void setFormat(StorageFormat format);

    /**
     * @brief 检查是否启用自动备份
     * @return 是否启用自动备份
     */
    bool isAutoBackupEnabled() const;

    /**
     * @brief 启用/禁用自动备份
     * @param enabled 是否启用
     */
    void setAutoBackupEnabled(bool enabled);

    /**
     * @brief 检查是否启用文件监控
     * @return 是否启用文件监控
     */
    bool isFileWatchingEnabled() const;

    /**
     * @brief 启用/禁用文件监控
     * @param enabled 是否启用
     */
    void setFileWatchingEnabled(bool enabled);

    // 数据操作
    /**
     * @brief 设置值
     * @param key 键
     * @param value 值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取值
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 检查是否包含键
     * @param key 键
     * @return 是否包含
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除键值对
     * @param key 键
     */
    void remove(const QString& key);

    /**
     * @brief 获取所有键
     * @return 键列表
     */
    QStringList allKeys() const;

    /**
     * @brief 获取子键
     * @param group 组名
     * @return 子键列表
     */
    QStringList childKeys(const QString& group) const;

    /**
     * @brief 获取子组
     * @param group 组名
     * @return 子组列表
     */
    QStringList childGroups(const QString& group) const;

    /**
     * @brief 清除所有数据
     */
    void clear();

    // 文件操作
    /**
     * @brief 加载数据
     * @return 加载是否成功
     */
    bool load();

    /**
     * @brief 保存数据
     * @return 保存是否成功
     */
    bool save();

    /**
     * @brief 同步数据（强制保存）
     * @return 同步是否成功
     */
    bool sync();

    /**
     * @brief 重新加载数据
     * @return 重新加载是否成功
     */
    bool reload();

    // 备份和恢复
    /**
     * @brief 设置备份策略
     * @param strategy 备份策略
     * @param maxBackups 最大备份数量
     */
    void setBackupStrategy(BackupStrategy strategy, int maxBackups = 5);

    /**
     * @brief 获取备份策略
     * @return 备份策略
     */
    BackupStrategy backupStrategy() const;

    /**
     * @brief 创建备份
     * @param backupName 备份名称（可选）
     * @return 备份是否成功
     */
    bool createBackup(const QString& backupName = QString());

    /**
     * @brief 恢复备份
     * @param backupName 备份名称
     * @return 恢复是否成功
     */
    bool restoreBackup(const QString& backupName);

    /**
     * @brief 获取可用备份列表
     * @return 备份列表
     */
    QStringList availableBackups() const;

    /**
     * @brief 删除备份
     * @param backupName 备份名称
     * @return 删除是否成功
     */
    bool deleteBackup(const QString& backupName);

    /**
     * @brief 清理旧备份
     */
    void cleanupOldBackups();

    // 导入导出
    /**
     * @brief 导出到文件
     * @param exportPath 导出路径
     * @param exportFormat 导出格式
     * @return 导出是否成功
     */
    bool exportToFile(const QString& exportPath, StorageFormat exportFormat = JsonFormat) const;

    /**
     * @brief 从文件导入
     * @param importPath 导入路径
     * @param merge 是否合并（否则替换）
     * @return 导入是否成功
     */
    bool importFromFile(const QString& importPath, bool merge = false);

    /**
     * @brief 导出到JSON对象
     * @return JSON对象
     */
    QJsonObject exportToJson() const;

    /**
     * @brief 从JSON对象导入
     * @param json JSON对象
     * @param merge 是否合并
     * @return 导入是否成功
     */
    bool importFromJson(const QJsonObject& json, bool merge = false);

    // 工具方法
    /**
     * @brief 检查文件是否存在
     * @return 文件是否存在
     */
    bool fileExists() const;

    /**
     * @brief 获取文件大小
     * @return 文件大小（字节）
     */
    qint64 fileSize() const;

    /**
     * @brief 获取最后修改时间
     * @return 最后修改时间
     */
    QDateTime lastModified() const;

    /**
     * @brief 检查文件是否可读
     * @return 是否可读
     */
    bool isReadable() const;

    /**
     * @brief 检查文件是否可写
     * @return 是否可写
     */
    bool isWritable() const;

    /**
     * @brief 获取存储统计信息
     * @return 统计信息
     */
    QVariantMap statistics() const;

    /**
     * @brief 验证文件完整性
     * @return 验证是否通过
     */
    bool validateIntegrity() const;

    /**
     * @brief 修复损坏的文件
     * @return 修复是否成功
     */
    bool repairCorruption();

public slots:
    /**
     * @brief 强制同步
     */
    void forceSync();

    /**
     * @brief 刷新数据
     */
    void refresh();

    /**
     * @brief 压缩存储
     */
    void compact();

signals:
    /**
     * @brief 文件路径变化信号
     * @param path 新路径
     */
    void filePathChanged(const QString& path);

    /**
     * @brief 格式变化信号
     * @param format 新格式
     */
    void formatChanged(StorageFormat format);

    /**
     * @brief 自动备份设置变化信号
     * @param enabled 是否启用
     */
    void autoBackupChanged(bool enabled);

    /**
     * @brief 文件监控设置变化信号
     * @param enabled 是否启用
     */
    void fileWatchingChanged(bool enabled);

    /**
     * @brief 数据变化信号
     * @param key 变化的键
     * @param value 新值
     */
    void dataChanged(const QString& key, const QVariant& value);

    /**
     * @brief 数据加载完成信号
     * @param success 是否成功
     */
    void dataLoaded(bool success);

    /**
     * @brief 数据保存完成信号
     * @param success 是否成功
     */
    void dataSaved(bool success);

    /**
     * @brief 文件变化信号
     * @param path 文件路径
     */
    void fileChanged(const QString& path);

    /**
     * @brief 备份创建信号
     * @param backupName 备份名称
     * @param success 是否成功
     */
    void backupCreated(const QString& backupName, bool success);

    /**
     * @brief 备份恢复信号
     * @param backupName 备份名称
     * @param success 是否成功
     */
    void backupRestored(const QString& backupName, bool success);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    void onFileChanged(const QString& path);

private:
    void setStatus(StorageStatus newStatus);
    QString getBackupPath(const QString& backupName) const;
    QString generateBackupName() const;
    bool writeToFile(const QVariant& data, const QString& filePath) const;
    bool writeToFile(const QVariant& data, const QString& filePath, StorageFormat format) const;
    QVariant readFromFile(const QString& filePath) const;
    bool atomicWrite(const QByteArray& data, const QString& filePath) const;
    QByteArray formatData(const QVariant& data, StorageFormat format) const;
    QVariant parseData(const QByteArray& data, StorageFormat format) const;
    QString formatToExtension(StorageFormat format) const;
    StorageFormat extensionToFormat(const QString& extension) const;
    void setupFileWatcher();
    void updateStatistics(const QString& operation) const;

    class Private;
    std::unique_ptr<Private> d;
};

#endif // LOCALSTORAGE_H