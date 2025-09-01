#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "../interfaces/ISettingsManager.h"
#include <QSettings>
#include <QTimer>
#include <QMutex>
#include <memory>

// Forward declarations
class IConfigValidator;
class QFileSystemWatcher;

/**
 * @brief 设置管理器实现类
 * 
 * ISettingsManager 接口的具体实现，提供完整的设置管理功能。
 * 支持多种存储后端、自动同步、配置验证和线程安全操作。
 */
class SettingsManager : public ISettingsManager
{
    Q_OBJECT

public:
    /**
     * @brief 存储后端类型
     */
    enum StorageBackend {
        LocalFile,      ///< 本地文件存储
        Registry,       ///< 系统注册表
        CloudSync,      ///< 云端同步
        Memory          ///< 内存存储（临时）
    };
    Q_ENUM(StorageBackend)

    /**
     * @brief 同步策略
     */
    enum SyncStrategy {
        Manual,         ///< 手动同步
        Automatic,      ///< 自动同步
        OnChange,       ///< 值变化时同步
        Periodic        ///< 定期同步
    };
    Q_ENUM(SyncStrategy)

    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager();

    /**
     * @brief 获取单例实例
     * @return 管理器实例
     */
    static SettingsManager* instance();

    // ISettingsManager 接口实现
    bool initialize() override;
    ManagerStatus status() const override;
    
    void setValue(const QString& key, const QVariant& value, SettingsScope scope = UserScope) override;
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant(), SettingsScope scope = UserScope) const override;
    bool contains(const QString& key, SettingsScope scope = UserScope) const override;
    void remove(const QString& key, SettingsScope scope = UserScope) override;
    
    QStringList allKeys(SettingsScope scope = UserScope) const override;
    QStringList childKeys(const QString& group, SettingsScope scope = UserScope) const override;
    QStringList childGroups(const QString& group, SettingsScope scope = UserScope) const override;
    
    bool sync() override;
    bool validate() const override;
    void reset(SettingsScope scope = UserScope) override;
    void resetGroup(const QString& group, SettingsScope scope = UserScope) override;
    
    bool exportSettings(const QString& filePath, SettingsScope scope = UserScope) const override;
    bool importSettings(const QString& filePath, SettingsScope scope = UserScope) override;

    // 扩展功能
    /**
     * @brief 设置存储后端
     * @param backend 后端类型
     * @param parameters 后端参数
     */
    void setStorageBackend(StorageBackend backend, const QVariantMap& parameters = QVariantMap());

    /**
     * @brief 获取当前存储后端
     * @return 后端类型
     */
    StorageBackend storageBackend() const;

    /**
     * @brief 设置配置验证器
     * @param validator 验证器实例
     */
    void setValidator(IConfigValidator* validator);

    /**
     * @brief 获取配置验证器
     * @return 验证器实例
     */
    IConfigValidator* validator() const;

    /**
     * @brief 设置同步策略
     * @param strategy 同步策略
     * @param interval 同步间隔（毫秒，仅对 Periodic 策略有效）
     */
    void setSyncStrategy(SyncStrategy strategy, int interval = 30000);

    /**
     * @brief 获取同步策略
     * @return 同步策略
     */
    SyncStrategy syncStrategy() const;

    /**
     * @brief 启用/禁用加密
     * @param enabled 是否启用
     * @param key 加密密钥
     */
    void setEncryption(bool enabled, const QString& key = QString());

    /**
     * @brief 检查是否启用加密
     * @return 是否启用加密
     */
    bool isEncryptionEnabled() const;

    /**
     * @brief 设置配置文件路径
     * @param path 文件路径
     */
    void setConfigPath(const QString& path);

    /**
     * @brief 获取配置文件路径
     * @return 文件路径
     */
    QString configPath() const;

    /**
     * @brief 启用/禁用文件监控
     * @param enabled 是否启用
     */
    void setFileWatchingEnabled(bool enabled);

    /**
     * @brief 检查是否启用文件监控
     * @return 是否启用文件监控
     */
    bool isFileWatchingEnabled() const;

    /**
     * @brief 获取设置统计信息
     * @return 统计信息
     */
    QVariantMap statistics() const;

    /**
     * @brief 清除所有缓存
     */
    void clearCache();

    /**
     * @brief 开始批量操作
     */
    void beginBatch();

    /**
     * @brief 结束批量操作
     * @param commit 是否提交更改
     */
    void endBatch(bool commit = true);

    /**
     * @brief 检查是否在批量操作中
     * @return 是否在批量操作
     */
    bool isBatchMode() const;

public slots:
    /**
     * @brief 强制同步
     */
    void forceSync();

    /**
     * @brief 重新加载设置
     */
    void reload();

    /**
     * @brief 备份设置
     * @param backupPath 备份路径
     */
    void backup(const QString& backupPath);

    /**
     * @brief 恢复设置
     * @param backupPath 备份路径
     */
    void restore(const QString& backupPath);

private slots:
    void onSyncTimer();
    void onFileChanged(const QString& path);
    void onValidationCompleted(bool success, const QStringList& errors);

private:
    void setStatus(ManagerStatus newStatus);
    QSettings* getSettings(SettingsScope scope) const;
    QString scopeToString(SettingsScope scope) const;
    void setupFileWatcher();
    void performAutoSync();
    QVariant encryptValue(const QVariant& value) const;
    QVariant decryptValue(const QVariant& value) const;
    void updateStatistics(const QString& operation);

    class Private;
    std::unique_ptr<Private> d;
};

#endif // SETTINGSMANAGER_H