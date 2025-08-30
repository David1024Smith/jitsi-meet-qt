#ifndef SETTINGSMODULE_H
#define SETTINGSMODULE_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <memory>

// Forward declarations
class ISettingsManager;
class IPreferencesHandler;
class IConfigValidator;
class SettingsConfig;

/**
 * @brief 设置模块核心类
 * 
 * Settings Module 的主要入口点，负责模块的初始化、配置和生命周期管理。
 * 提供了统一的设置管理接口，集成了设置管理器、偏好处理器和配置验证器。
 */
class SettingsModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(ModuleStatus status READ status NOTIFY statusChanged)

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        NotLoaded,      ///< 未加载
        Loading,        ///< 加载中
        Loaded,         ///< 已加载
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪状态
        Error,          ///< 错误状态
        Unloading       ///< 卸载中
    };
    Q_ENUM(ModuleStatus)

    /**
     * @brief 模块配置选项
     */
    struct ModuleOptions {
        QString configPath;         ///< 配置文件路径
        QString storageBackend;     ///< 存储后端类型
        bool enableValidation;      ///< 启用验证
        bool enableEncryption;      ///< 启用加密
        bool autoSync;              ///< 自动同步
        int syncInterval;           ///< 同步间隔（秒）
        
        ModuleOptions() 
            : storageBackend("local")
            , enableValidation(true)
            , enableEncryption(false)
            , autoSync(true)
            , syncInterval(30) {}
    };

    explicit SettingsModule(QObject* parent = nullptr);
    ~SettingsModule();

    /**
     * @brief 获取模块单例实例
     * @return 模块实例
     */
    static SettingsModule* instance();

    /**
     * @brief 获取模块版本
     * @return 版本字符串
     */
    QString version() const;

    /**
     * @brief 获取模块状态
     * @return 当前状态
     */
    ModuleStatus status() const;

    /**
     * @brief 检查模块是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 初始化模块
     * @param options 模块配置选项
     * @return 初始化是否成功
     */
    bool initialize(const ModuleOptions& options = ModuleOptions());

    /**
     * @brief 卸载模块
     */
    void shutdown();

    /**
     * @brief 获取设置管理器
     * @return 设置管理器实例
     */
    ISettingsManager* settingsManager() const;

    /**
     * @brief 获取偏好处理器
     * @return 偏好处理器实例
     */
    IPreferencesHandler* preferencesHandler() const;

    /**
     * @brief 获取配置验证器
     * @return 配置验证器实例
     */
    IConfigValidator* configValidator() const;

    /**
     * @brief 获取模块配置
     * @return 模块配置实例
     */
    SettingsConfig* moduleConfig() const;

    /**
     * @brief 设置存储后端
     * @param backendType 后端类型（"local", "cloud", "registry"）
     * @param parameters 后端参数
     * @return 设置是否成功
     */
    bool setStorageBackend(const QString& backendType, const QVariantMap& parameters = QVariantMap());

    /**
     * @brief 启用/禁用功能
     * @param feature 功能名称
     * @param enabled 是否启用
     */
    void setFeatureEnabled(const QString& feature, bool enabled);

    /**
     * @brief 检查功能是否启用
     * @param feature 功能名称
     * @return 是否启用
     */
    bool isFeatureEnabled(const QString& feature) const;

    /**
     * @brief 获取模块信息
     * @return 模块信息JSON对象
     */
    QJsonObject moduleInfo() const;

    /**
     * @brief 获取模块统计信息
     * @return 统计信息JSON对象
     */
    QJsonObject moduleStatistics() const;

    /**
     * @brief 执行模块自检
     * @return 自检结果
     */
    QStringList performSelfCheck() const;

    /**
     * @brief 重置模块到默认状态
     */
    void resetToDefaults();

    /**
     * @brief 导出模块配置
     * @param filePath 导出文件路径
     * @return 导出是否成功
     */
    bool exportConfiguration(const QString& filePath) const;

    /**
     * @brief 导入模块配置
     * @param filePath 导入文件路径
     * @return 导入是否成功
     */
    bool importConfiguration(const QString& filePath);

public slots:
    /**
     * @brief 同步所有设置
     */
    void syncAll();

    /**
     * @brief 验证所有配置
     */
    void validateAll();

    /**
     * @brief 重新加载配置
     */
    void reloadConfiguration();

signals:
    /**
     * @brief 初始化状态变化信号
     * @param initialized 是否已初始化
     */
    void initializedChanged(bool initialized);

    /**
     * @brief 模块状态变化信号
     * @param status 新状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块就绪信号
     */
    void moduleReady();

    /**
     * @brief 模块错误信号
     * @param error 错误信息
     */
    void moduleError(const QString& error);

    /**
     * @brief 配置变化信号
     * @param key 配置键
     * @param value 新值
     */
    void configurationChanged(const QString& key, const QVariant& value);

    /**
     * @brief 同步完成信号
     * @param success 是否成功
     */
    void syncCompleted(bool success);

    /**
     * @brief 验证完成信号
     * @param success 是否成功
     * @param errors 错误列表
     */
    void validationCompleted(bool success, const QStringList& errors);

private slots:
    void onSettingsManagerStatusChanged();
    void onPreferencesHandlerError(const QString& error);
    void onValidationCompleted(bool success, const QStringList& errors);

private:
    void setStatus(ModuleStatus newStatus);
    bool createComponents();
    void connectSignals();
    void loadDefaultConfiguration();
    void setupValidationRules();

    class Private;
    std::unique_ptr<Private> d;
};

#endif // SETTINGSMODULE_H