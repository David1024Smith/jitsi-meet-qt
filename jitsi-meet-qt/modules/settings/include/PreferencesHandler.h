#ifndef PREFERENCESHANDLER_H
#define PREFERENCESHANDLER_H

#include "../interfaces/IPreferencesHandler.h"
#include <QJsonDocument>
#include <QMutex>
#include <memory>

// Forward declarations
class SettingsManager;
class QTimer;

/**
 * @brief 偏好设置处理器实现类
 * 
 * IPreferencesHandler 接口的具体实现，提供高级的用户偏好管理功能。
 * 支持用户配置文件、偏好分类管理、优先级处理和配置文件切换。
 */
class PreferencesHandler : public IPreferencesHandler
{
    Q_OBJECT

public:
    /**
     * @brief 偏好存储格式
     */
    enum StorageFormat {
        JsonFormat,     ///< JSON 格式
        IniFormat,      ///< INI 格式
        XmlFormat       ///< XML 格式
    };
    Q_ENUM(StorageFormat)

    /**
     * @brief 冲突解决策略
     */
    enum ConflictResolution {
        KeepLocal,      ///< 保留本地值
        KeepRemote,     ///< 保留远程值
        MergeValues,    ///< 合并值
        AskUser         ///< 询问用户
    };
    Q_ENUM(ConflictResolution)

    explicit PreferencesHandler(QObject* parent = nullptr);
    ~PreferencesHandler();

    /**
     * @brief 获取单例实例
     * @return 处理器实例
     */
    static PreferencesHandler* instance();

    // IPreferencesHandler 接口实现
    bool initialize() override;
    
    void setPreference(PreferenceCategory category, const QString& key, 
                      const QVariant& value, PreferencePriority priority = NormalPriority) override;
    void setPreference(const QString& category, const QString& key, 
                      const QVariant& value, PreferencePriority priority = NormalPriority) override;
    
    QVariant preference(PreferenceCategory category, const QString& key, 
                       const QVariant& defaultValue = QVariant()) const override;
    QVariant preference(const QString& category, const QString& key, 
                       const QVariant& defaultValue = QVariant()) const override;
    
    QStringList categories() const override;
    QStringList keys(const QString& category) const override;
    PreferenceStatus preferenceStatus(const QString& category, const QString& key) const override;
    bool hasPreference(const QString& category, const QString& key) const override;
    
    void removePreference(const QString& category, const QString& key) override;
    void resetCategory(const QString& category) override;
    void resetAll() override;
    
    QVariantMap categoryPreferences(const QString& category) const override;
    void setCategoryPreferences(const QString& category, const QVariantMap& preferences) override;
    
    QJsonObject exportToJson(const QString& category = QString()) const override;
    bool importFromJson(const QJsonObject& json, const QString& category = QString()) override;
    
    bool createProfile(const QString& profileName) override;
    bool switchToProfile(const QString& profileName) override;
    bool deleteProfile(const QString& profileName) override;
    QStringList availableProfiles() const override;
    QString currentProfile() const override;
    
    bool sync() override;

    // 扩展功能
    /**
     * @brief 设置设置管理器
     * @param manager 设置管理器实例
     */
    void setSettingsManager(SettingsManager* manager);

    /**
     * @brief 获取设置管理器
     * @return 设置管理器实例
     */
    SettingsManager* settingsManager() const;

    /**
     * @brief 设置存储格式
     * @param format 存储格式
     */
    void setStorageFormat(StorageFormat format);

    /**
     * @brief 获取存储格式
     * @return 存储格式
     */
    StorageFormat storageFormat() const;

    /**
     * @brief 设置冲突解决策略
     * @param strategy 解决策略
     */
    void setConflictResolution(ConflictResolution strategy);

    /**
     * @brief 获取冲突解决策略
     * @return 解决策略
     */
    ConflictResolution conflictResolution() const;

    /**
     * @brief 设置偏好优先级
     * @param category 类别名称
     * @param key 偏好键
     * @param priority 优先级
     */
    void setPreferencePriority(const QString& category, const QString& key, PreferencePriority priority);

    /**
     * @brief 获取偏好优先级
     * @param category 类别名称
     * @param key 偏好键
     * @return 优先级
     */
    PreferencePriority preferencePriority(const QString& category, const QString& key) const;

    /**
     * @brief 复制配置文件
     * @param sourceProfile 源配置文件
     * @param targetProfile 目标配置文件
     * @return 复制是否成功
     */
    bool copyProfile(const QString& sourceProfile, const QString& targetProfile);

    /**
     * @brief 重命名配置文件
     * @param oldName 旧名称
     * @param newName 新名称
     * @return 重命名是否成功
     */
    bool renameProfile(const QString& oldName, const QString& newName);

    /**
     * @brief 导出配置文件
     * @param profileName 配置文件名称
     * @param filePath 导出路径
     * @return 导出是否成功
     */
    bool exportProfile(const QString& profileName, const QString& filePath) const;

    /**
     * @brief 导入配置文件
     * @param filePath 导入路径
     * @param profileName 目标配置文件名称
     * @return 导入是否成功
     */
    bool importProfile(const QString& filePath, const QString& profileName);

    /**
     * @brief 获取配置文件信息
     * @param profileName 配置文件名称
     * @return 配置文件信息
     */
    QVariantMap profileInfo(const QString& profileName) const;

    /**
     * @brief 设置默认配置文件
     * @param profileName 配置文件名称
     */
    void setDefaultProfile(const QString& profileName);

    /**
     * @brief 获取默认配置文件
     * @return 默认配置文件名称
     */
    QString defaultProfile() const;

    /**
     * @brief 启用/禁用自动备份
     * @param enabled 是否启用
     * @param interval 备份间隔（分钟）
     */
    void setAutoBackup(bool enabled, int interval = 60);

    /**
     * @brief 检查是否启用自动备份
     * @return 是否启用自动备份
     */
    bool isAutoBackupEnabled() const;

    /**
     * @brief 创建偏好备份
     * @param backupName 备份名称
     * @return 备份是否成功
     */
    bool createBackup(const QString& backupName);

    /**
     * @brief 恢复偏好备份
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

public slots:
    /**
     * @brief 刷新偏好设置
     */
    void refresh();

    /**
     * @brief 清理过期数据
     */
    void cleanup();

    /**
     * @brief 压缩存储
     */
    void compact();

private slots:
    void onAutoBackupTimer();
    void onSettingsChanged(const QString& key, const QVariant& value);

private:
    QString categoryToString(PreferenceCategory category) const;
    PreferenceCategory stringToCategory(const QString& category) const;
    QString priorityToString(PreferencePriority priority) const;
    PreferencePriority stringToPriority(const QString& priority) const;
    QString statusToString(PreferenceStatus status) const;
    PreferenceStatus stringToStatus(const QString& status) const;
    
    QString getPreferenceKey(const QString& category, const QString& key) const;
    QString getMetaKey(const QString& category, const QString& key, const QString& metaType) const;
    QString getProfilePath(const QString& profileName) const;
    QString getBackupPath(const QString& backupName) const;
    
    void loadProfile(const QString& profileName);
    void saveProfile(const QString& profileName);
    void createDefaultCategories();
    void migrateOldPreferences();
    
    bool resolveConflict(const QString& category, const QString& key, 
                        const QVariant& localValue, const QVariant& remoteValue);

    class Private;
    std::unique_ptr<Private> d;
};

#endif // PREFERENCESHANDLER_H