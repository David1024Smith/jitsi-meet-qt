#ifndef IPREFERENCESHANDLER_H
#define IPREFERENCESHANDLER_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>

/**
 * @brief 偏好设置处理器接口
 * 
 * 定义了用户偏好设置的管理接口，支持分类管理、用户配置文件和偏好同步。
 * 提供了比基础设置管理器更高级的用户偏好管理功能。
 */
class IPreferencesHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 偏好设置类别枚举
     */
    enum PreferenceCategory {
        AudioPreferences,       ///< 音频偏好
        VideoPreferences,       ///< 视频偏好
        UIPreferences,          ///< 界面偏好
        NetworkPreferences,     ///< 网络偏好
        SecurityPreferences,    ///< 安全偏好
        PerformancePreferences, ///< 性能偏好
        CustomPreferences       ///< 自定义偏好
    };
    Q_ENUM(PreferenceCategory)

    /**
     * @brief 偏好设置优先级
     */
    enum PreferencePriority {
        LowPriority,    ///< 低优先级
        NormalPriority, ///< 普通优先级
        HighPriority,   ///< 高优先级
        CriticalPriority ///< 关键优先级
    };
    Q_ENUM(PreferencePriority)

    /**
     * @brief 偏好设置状态
     */
    enum PreferenceStatus {
        Default,    ///< 默认值
        Modified,   ///< 已修改
        Synced,     ///< 已同步
        Conflict    ///< 冲突状态
    };
    Q_ENUM(PreferenceStatus)

    virtual ~IPreferencesHandler() = default;

    /**
     * @brief 初始化偏好处理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 设置偏好值
     * @param category 偏好类别
     * @param key 偏好键
     * @param value 偏好值
     * @param priority 优先级
     */
    virtual void setPreference(PreferenceCategory category, const QString& key, 
                              const QVariant& value, PreferencePriority priority = NormalPriority) = 0;

    /**
     * @brief 设置偏好值（使用字符串类别）
     * @param category 类别名称
     * @param key 偏好键
     * @param value 偏好值
     * @param priority 优先级
     */
    virtual void setPreference(const QString& category, const QString& key, 
                              const QVariant& value, PreferencePriority priority = NormalPriority) = 0;

    /**
     * @brief 获取偏好值
     * @param category 偏好类别
     * @param key 偏好键
     * @param defaultValue 默认值
     * @return 偏好值
     */
    virtual QVariant preference(PreferenceCategory category, const QString& key, 
                               const QVariant& defaultValue = QVariant()) const = 0;

    /**
     * @brief 获取偏好值（使用字符串类别）
     * @param category 类别名称
     * @param key 偏好键
     * @param defaultValue 默认值
     * @return 偏好值
     */
    virtual QVariant preference(const QString& category, const QString& key, 
                               const QVariant& defaultValue = QVariant()) const = 0;

    /**
     * @brief 获取所有类别
     * @return 类别列表
     */
    virtual QStringList categories() const = 0;

    /**
     * @brief 获取指定类别的所有键
     * @param category 类别名称
     * @return 键列表
     */
    virtual QStringList keys(const QString& category) const = 0;

    /**
     * @brief 获取偏好状态
     * @param category 类别名称
     * @param key 偏好键
     * @return 偏好状态
     */
    virtual PreferenceStatus preferenceStatus(const QString& category, const QString& key) const = 0;

    /**
     * @brief 检查偏好是否存在
     * @param category 类别名称
     * @param key 偏好键
     * @return 是否存在
     */
    virtual bool hasPreference(const QString& category, const QString& key) const = 0;

    /**
     * @brief 移除偏好
     * @param category 类别名称
     * @param key 偏好键
     */
    virtual void removePreference(const QString& category, const QString& key) = 0;

    /**
     * @brief 重置类别的所有偏好
     * @param category 类别名称
     */
    virtual void resetCategory(const QString& category) = 0;

    /**
     * @brief 重置所有偏好
     */
    virtual void resetAll() = 0;

    /**
     * @brief 获取类别的所有偏好
     * @param category 类别名称
     * @return 偏好映射
     */
    virtual QVariantMap categoryPreferences(const QString& category) const = 0;

    /**
     * @brief 设置类别的所有偏好
     * @param category 类别名称
     * @param preferences 偏好映射
     */
    virtual void setCategoryPreferences(const QString& category, const QVariantMap& preferences) = 0;

    /**
     * @brief 导出偏好到JSON
     * @param category 类别名称（空表示所有类别）
     * @return JSON对象
     */
    virtual QJsonObject exportToJson(const QString& category = QString()) const = 0;

    /**
     * @brief 从JSON导入偏好
     * @param json JSON对象
     * @param category 目标类别（空表示按JSON结构导入）
     * @return 导入是否成功
     */
    virtual bool importFromJson(const QJsonObject& json, const QString& category = QString()) = 0;

    /**
     * @brief 创建用户配置文件
     * @param profileName 配置文件名称
     * @return 创建是否成功
     */
    virtual bool createProfile(const QString& profileName) = 0;

    /**
     * @brief 切换到指定配置文件
     * @param profileName 配置文件名称
     * @return 切换是否成功
     */
    virtual bool switchToProfile(const QString& profileName) = 0;

    /**
     * @brief 删除配置文件
     * @param profileName 配置文件名称
     * @return 删除是否成功
     */
    virtual bool deleteProfile(const QString& profileName) = 0;

    /**
     * @brief 获取所有配置文件
     * @return 配置文件列表
     */
    virtual QStringList availableProfiles() const = 0;

    /**
     * @brief 获取当前配置文件
     * @return 当前配置文件名称
     */
    virtual QString currentProfile() const = 0;

    /**
     * @brief 同步偏好设置
     * @return 同步是否成功
     */
    virtual bool sync() = 0;

signals:
    /**
     * @brief 偏好值变化信号
     * @param category 类别名称
     * @param key 偏好键
     * @param value 新值
     */
    void preferenceChanged(const QString& category, const QString& key, const QVariant& value);

    /**
     * @brief 类别重置信号
     * @param category 类别名称
     */
    void categoryReset(const QString& category);

    /**
     * @brief 所有偏好重置信号
     */
    void allPreferencesReset();

    /**
     * @brief 配置文件变化信号
     * @param oldProfile 旧配置文件
     * @param newProfile 新配置文件
     */
    void profileChanged(const QString& oldProfile, const QString& newProfile);

    /**
     * @brief 同步完成信号
     * @param success 同步是否成功
     */
    void syncCompleted(bool success);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

#endif // IPREFERENCESHANDLER_H