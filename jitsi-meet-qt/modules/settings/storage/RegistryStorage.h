#ifndef REGISTRYSTORAGE_H
#define REGISTRYSTORAGE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>

#ifdef Q_OS_WIN
#include <QSettings>
#include <QWinEventNotifier>
#include <windows.h>
#endif

#include <memory>

/**
 * @brief 注册表存储后端类
 * 
 * 提供Windows注册表的设置存储功能，支持系统级和用户级设置。
 * 包含注册表监控、权限管理、备份恢复等功能。
 * 
 * 注意：此类仅在Windows平台可用，其他平台将使用替代实现。
 */
class RegistryStorage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString registryPath READ registryPath WRITE setRegistryPath NOTIFY registryPathChanged)
    Q_PROPERTY(RegistryScope scope READ scope WRITE setScope NOTIFY scopeChanged)
    Q_PROPERTY(bool monitoringEnabled READ isMonitoringEnabled WRITE setMonitoringEnabled NOTIFY monitoringEnabledChanged)

public:
    /**
     * @brief 注册表作用域枚举
     */
    enum RegistryScope {
        CurrentUser,    ///< 当前用户 (HKEY_CURRENT_USER)
        LocalMachine,   ///< 本地机器 (HKEY_LOCAL_MACHINE)
        ClassesRoot,    ///< 类根 (HKEY_CLASSES_ROOT)
        Users,          ///< 用户 (HKEY_USERS)
        CurrentConfig   ///< 当前配置 (HKEY_CURRENT_CONFIG)
    };
    Q_ENUM(RegistryScope)

    /**
     * @brief 注册表访问权限
     */
    enum AccessRights {
        ReadOnly,       ///< 只读
        ReadWrite,      ///< 读写
        FullControl     ///< 完全控制
    };
    Q_ENUM(AccessRights)

    /**
     * @brief 数据类型枚举
     */
    enum DataType {
        StringType,     ///< 字符串 (REG_SZ)
        DWordType,      ///< 双字 (REG_DWORD)
        QWordType,      ///< 四字 (REG_QWORD)
        BinaryType,     ///< 二进制 (REG_BINARY)
        MultiStringType,///< 多字符串 (REG_MULTI_SZ)
        ExpandStringType ///< 可扩展字符串 (REG_EXPAND_SZ)
    };
    Q_ENUM(DataType)

    /**
     * @brief 存储状态枚举
     */
    enum StorageStatus {
        NotInitialized, ///< 未初始化
        Ready,          ///< 就绪状态
        AccessDenied,   ///< 访问被拒绝
        KeyNotFound,    ///< 键未找到
        Error           ///< 错误状态
    };
    Q_ENUM(StorageStatus)

    explicit RegistryStorage(QObject* parent = nullptr);
    explicit RegistryStorage(const QString& registryPath, RegistryScope scope = CurrentUser, QObject* parent = nullptr);
    ~RegistryStorage();

    /**
     * @brief 初始化注册表存储
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 获取存储状态
     * @return 当前状态
     */
    StorageStatus status() const;

    /**
     * @brief 获取注册表路径
     * @return 注册表路径
     */
    QString registryPath() const;

    /**
     * @brief 设置注册表路径
     * @param path 注册表路径
     */
    void setRegistryPath(const QString& path);

    /**
     * @brief 获取注册表作用域
     * @return 作用域
     */
    RegistryScope scope() const;

    /**
     * @brief 设置注册表作用域
     * @param scope 作用域
     */
    void setScope(RegistryScope scope);

    /**
     * @brief 检查是否启用监控
     * @return 是否启用监控
     */
    bool isMonitoringEnabled() const;

    /**
     * @brief 启用/禁用注册表监控
     * @param enabled 是否启用
     */
    void setMonitoringEnabled(bool enabled);

    // 权限管理
    /**
     * @brief 设置访问权限
     * @param rights 访问权限
     */
    void setAccessRights(AccessRights rights);

    /**
     * @brief 获取访问权限
     * @return 访问权限
     */
    AccessRights accessRights() const;

    /**
     * @brief 检查是否有读权限
     * @return 是否有读权限
     */
    bool hasReadAccess() const;

    /**
     * @brief 检查是否有写权限
     * @return 是否有写权限
     */
    bool hasWriteAccess() const;

    /**
     * @brief 请求管理员权限
     * @return 请求是否成功
     */
    bool requestElevatedAccess();

    // 数据操作
    /**
     * @brief 设置值
     * @param key 键
     * @param value 值
     * @param dataType 数据类型（可选）
     */
    void setValue(const QString& key, const QVariant& value, DataType dataType = StringType);

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

    // 注册表特定操作
    /**
     * @brief 创建注册表键
     * @param keyPath 键路径
     * @return 创建是否成功
     */
    bool createKey(const QString& keyPath);

    /**
     * @brief 删除注册表键
     * @param keyPath 键路径
     * @param recursive 是否递归删除
     * @return 删除是否成功
     */
    bool deleteKey(const QString& keyPath, bool recursive = false);

    /**
     * @brief 检查键是否存在
     * @param keyPath 键路径
     * @return 键是否存在
     */
    bool keyExists(const QString& keyPath) const;

    /**
     * @brief 获取键的数据类型
     * @param key 键
     * @return 数据类型
     */
    DataType getDataType(const QString& key) const;

    /**
     * @brief 设置键的数据类型
     * @param key 键
     * @param dataType 数据类型
     */
    void setDataType(const QString& key, DataType dataType);

    /**
     * @brief 获取键的大小
     * @param key 键
     * @return 数据大小（字节）
     */
    qint64 getDataSize(const QString& key) const;

    // 备份和恢复
    /**
     * @brief 导出注册表到文件
     * @param filePath 导出文件路径
     * @param format 导出格式（.reg 或 .json）
     * @return 导出是否成功
     */
    bool exportToFile(const QString& filePath, const QString& format = "reg") const;

    /**
     * @brief 从文件导入注册表
     * @param filePath 导入文件路径
     * @param merge 是否合并（否则替换）
     * @return 导入是否成功
     */
    bool importFromFile(const QString& filePath, bool merge = false);

    /**
     * @brief 创建备份
     * @param backupName 备份名称
     * @return 备份是否成功
     */
    bool createBackup(const QString& backupName);

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

    // 监控和通知
#ifdef Q_OS_WIN
    /**
     * @brief 开始监控注册表变化
     * @param watchSubtree 是否监控子树
     * @return 监控是否成功启动
     */
    bool startMonitoring(bool watchSubtree = true);

    /**
     * @brief 停止监控
     */
    void stopMonitoring();

    /**
     * @brief 检查是否正在监控
     * @return 是否正在监控
     */
    bool isMonitoring() const;
#endif

    // 工具方法
    /**
     * @brief 获取完整注册表路径
     * @return 完整路径
     */
    QString fullRegistryPath() const;

    /**
     * @brief 获取注册表统计信息
     * @return 统计信息
     */
    QVariantMap statistics() const;

    /**
     * @brief 验证注册表完整性
     * @return 验证是否通过
     */
    bool validateIntegrity() const;

    /**
     * @brief 压缩注册表
     * @return 压缩是否成功
     */
    bool compactRegistry();

    // 平台兼容性
    /**
     * @brief 检查是否支持注册表
     * @return 是否支持
     */
    static bool isSupported();

    /**
     * @brief 获取替代存储路径（非Windows平台）
     * @return 替代路径
     */
    static QString alternativeStoragePath();

    // 导入导出
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

public slots:
    /**
     * @brief 同步数据
     */
    void sync();

    /**
     * @brief 刷新数据
     */
    void refresh();

    /**
     * @brief 清理无效项
     */
    void cleanup();

signals:
    /**
     * @brief 注册表路径变化信号
     * @param path 新路径
     */
    void registryPathChanged(const QString& path);

    /**
     * @brief 作用域变化信号
     * @param scope 新作用域
     */
    void scopeChanged(RegistryScope scope);

    /**
     * @brief 监控启用状态变化信号
     * @param enabled 是否启用
     */
    void monitoringEnabledChanged(bool enabled);

    /**
     * @brief 数据变化信号
     * @param key 变化的键
     * @param value 新值
     */
    void dataChanged(const QString& key, const QVariant& value);

    /**
     * @brief 键创建信号
     * @param keyPath 键路径
     */
    void keyCreated(const QString& keyPath);

    /**
     * @brief 键删除信号
     * @param keyPath 键路径
     */
    void keyDeleted(const QString& keyPath);

    /**
     * @brief 访问权限变化信号
     * @param rights 新权限
     */
    void accessRightsChanged(AccessRights rights);

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

#ifdef Q_OS_WIN
private slots:
    void onRegistryChanged();
#endif

private:
    void setStatus(StorageStatus newStatus);
    QString scopeToString(RegistryScope scope) const;
    RegistryScope stringToScope(const QString& str) const;
    QString dataTypeToString(DataType type) const;
    DataType stringToDataType(const QString& str) const;
    QString getBackupPath(const QString& backupName) const;
    
#ifdef Q_OS_WIN
    HKEY getScopeHandle(RegistryScope scope) const;
    REGSAM getAccessMask(AccessRights rights) const;
    bool setupRegistryNotification();
    void cleanupRegistryNotification();
#endif

    void initializeAlternativeStorage();
    void loadData();
    void updateStatistics(const QString& operation) const;

    class Private;
    std::unique_ptr<Private> d;
};

#endif // REGISTRYSTORAGE_H