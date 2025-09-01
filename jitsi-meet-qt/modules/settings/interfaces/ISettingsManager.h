#ifndef ISETTINGSMANAGER_H
#define ISETTINGSMANAGER_H

#include <QObject>
#include <QVariant>
#include <QStringList>

/**
 * @brief 设置管理器接口
 * 
 * 定义了设置管理的核心接口，包括设置的读取、写入、验证和同步功能。
 * 支持分层的设置键值对管理和多种数据类型。
 */
class ISettingsManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 设置管理器状态枚举
     */
    enum ManagerStatus {
        Uninitialized,  ///< 未初始化
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪状态
        Syncing,        ///< 同步中
        Error           ///< 错误状态
    };
    Q_ENUM(ManagerStatus)

    /**
     * @brief 设置作用域枚举
     */
    enum SettingsScope {
        UserScope,      ///< 用户级设置
        SystemScope,    ///< 系统级设置
        ApplicationScope ///< 应用程序级设置
    };
    Q_ENUM(SettingsScope)

    explicit ISettingsManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ISettingsManager() = default;

    /**
     * @brief 初始化设置管理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取管理器状态
     * @return 当前状态
     */
    virtual ManagerStatus status() const = 0;

    /**
     * @brief 设置配置值
     * @param key 配置键（支持分层，如 "audio/volume"）
     * @param value 配置值
     * @param scope 设置作用域
     */
    virtual void setValue(const QString& key, const QVariant& value, SettingsScope scope = UserScope) = 0;

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @param scope 设置作用域
     * @return 配置值
     */
    virtual QVariant value(const QString& key, const QVariant& defaultValue = QVariant(), SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 检查是否包含指定键
     * @param key 配置键
     * @param scope 设置作用域
     * @return 是否包含该键
     */
    virtual bool contains(const QString& key, SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 移除配置项
     * @param key 配置键
     * @param scope 设置作用域
     */
    virtual void remove(const QString& key, SettingsScope scope = UserScope) = 0;

    /**
     * @brief 获取所有键列表
     * @param scope 设置作用域
     * @return 键列表
     */
    virtual QStringList allKeys(SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 获取指定组的键列表
     * @param group 组名
     * @param scope 设置作用域
     * @return 键列表
     */
    virtual QStringList childKeys(const QString& group, SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 获取子组列表
     * @param group 父组名
     * @param scope 设置作用域
     * @return 子组列表
     */
    virtual QStringList childGroups(const QString& group, SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 同步设置到存储后端
     * @return 同步是否成功
     */
    virtual bool sync() = 0;

    /**
     * @brief 验证当前配置
     * @return 验证是否通过
     */
    virtual bool validate() const = 0;

    /**
     * @brief 重置所有设置为默认值
     * @param scope 设置作用域
     */
    virtual void reset(SettingsScope scope = UserScope) = 0;

    /**
     * @brief 重置指定组的设置
     * @param group 组名
     * @param scope 设置作用域
     */
    virtual void resetGroup(const QString& group, SettingsScope scope = UserScope) = 0;

    /**
     * @brief 导出设置到文件
     * @param filePath 文件路径
     * @param scope 设置作用域
     * @return 导出是否成功
     */
    virtual bool exportSettings(const QString& filePath, SettingsScope scope = UserScope) const = 0;

    /**
     * @brief 从文件导入设置
     * @param filePath 文件路径
     * @param scope 设置作用域
     * @return 导入是否成功
     */
    virtual bool importSettings(const QString& filePath, SettingsScope scope = UserScope) = 0;

signals:
    /**
     * @brief 状态变化信号
     * @param status 新状态
     */
    void statusChanged(ManagerStatus status);

    /**
     * @brief 设置值变化信号
     * @param key 配置键
     * @param value 新值
     * @param scope 设置作用域
     */
    void valueChanged(const QString& key, const QVariant& value, SettingsScope scope);

    /**
     * @brief 同步完成信号
     * @param success 同步是否成功
     */
    void syncCompleted(bool success);

    /**
     * @brief 验证完成信号
     * @param success 验证是否通过
     * @param errors 错误列表
     */
    void validationCompleted(bool success, const QStringList& errors);

    /**
     * @brief 设置重置信号
     * @param scope 重置的作用域
     */
    void settingsReset(SettingsScope scope);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

#endif // ISETTINGSMANAGER_H