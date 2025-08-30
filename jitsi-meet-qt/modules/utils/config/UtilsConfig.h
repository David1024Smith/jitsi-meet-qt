#ifndef UTILSCONFIG_H
#define UTILSCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QSettings>
#include <QMutex>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief 工具模块统一配置管理类
 * 
 * UtilsConfig提供工具模块的统一配置管理，支持配置的加载、保存、
 * 验证和默认值管理。采用单例模式确保全局配置的一致性。
 */
class UtilsConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 配置项枚举
     */
    enum ConfigKey {
        // 模块基础配置
        ModuleVersion,
        ModuleEnabled,
        DebugMode,
        
        // 日志配置
        LogLevel,
        EnableFileLogging,
        EnableConsoleLogging,
        EnableNetworkLogging,
        LogFilePath,
        LogFileMaxSize,
        LogFileMaxCount,
        
        // 文件系统配置
        TempDirectory,
        ConfigDirectory,
        CacheDirectory,
        MaxTempFileSize,
        AutoCleanupTempFiles,
        
        // 加密配置
        DefaultEncryptionAlgorithm,
        KeySize,
        EnableSecureRandom,
        
        // 性能配置
        MaxConcurrentOperations,
        OperationTimeout,
        EnablePerformanceMonitoring,
        
        // 网络配置
        NetworkTimeout,
        MaxRetryAttempts,
        EnableNetworkCache
    };
    Q_ENUM(ConfigKey)

    /**
     * @brief 获取配置管理器单例实例
     * @return UtilsConfig实例指针
     */
    static UtilsConfig* instance();

    /**
     * @brief 析构函数
     */
    ~UtilsConfig();

    /**
     * @brief 初始化配置系统
     * @param configFilePath 配置文件路径（可选）
     * @return 初始化是否成功
     */
    bool initialize(const QString& configFilePath = QString());

    /**
     * @brief 加载配置
     * @param filePath 配置文件路径
     * @return 加载是否成功
     */
    bool loadConfiguration(const QString& filePath = QString());

    /**
     * @brief 保存配置
     * @param filePath 配置文件路径
     * @return 保存是否成功
     */
    bool saveConfiguration(const QString& filePath = QString());

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 验证配置
     * @return 配置是否有效
     */
    bool validateConfiguration() const;

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @return 配置值
     */
    QVariant getValue(ConfigKey key) const;

    /**
     * @brief 获取配置值（字符串形式）
     * @param keyName 配置键名
     * @return 配置值
     */
    QVariant getValue(const QString& keyName) const;

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void setValue(ConfigKey key, const QVariant& value);

    /**
     * @brief 设置配置值（字符串形式）
     * @param keyName 配置键名
     * @param value 配置值
     */
    void setValue(const QString& keyName, const QVariant& value);

    /**
     * @brief 获取所有配置
     * @return 配置映射
     */
    QVariantMap getAllConfiguration() const;

    /**
     * @brief 设置所有配置
     * @param config 配置映射
     */
    void setAllConfiguration(const QVariantMap& config);

    /**
     * @brief 获取配置键的字符串表示
     * @param key 配置键
     * @return 键名字符串
     */
    static QString keyToString(ConfigKey key);

    /**
     * @brief 从字符串获取配置键
     * @param keyName 键名字符串
     * @return 配置键
     */
    static ConfigKey stringToKey(const QString& keyName);

    /**
     * @brief 获取配置键的默认值
     * @param key 配置键
     * @return 默认值
     */
    static QVariant getDefaultValue(ConfigKey key);

    /**
     * @brief 获取配置文件路径
     * @return 配置文件路径
     */
    QString configFilePath() const;

    /**
     * @brief 检查配置是否已修改
     * @return 是否已修改
     */
    bool isModified() const;

    /**
     * @brief 导出配置为JSON
     * @return JSON对象
     */
    QJsonObject exportToJson() const;

    /**
     * @brief 从JSON导入配置
     * @param json JSON对象
     * @return 导入是否成功
     */
    bool importFromJson(const QJsonObject& json);

signals:
    /**
     * @brief 配置值改变信号
     * @param key 配置键
     * @param value 新值
     */
    void configurationChanged(const QString& key, const QVariant& value);

    /**
     * @brief 配置加载完成信号
     */
    void configurationLoaded();

    /**
     * @brief 配置保存完成信号
     */
    void configurationSaved();

    /**
     * @brief 配置错误信号
     * @param error 错误信息
     */
    void configurationError(const QString& error);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit UtilsConfig(QObject* parent = nullptr);

    /**
     * @brief 初始化默认配置
     */
    void initializeDefaults();

    /**
     * @brief 验证单个配置项
     * @param key 配置键
     * @param value 配置值
     * @return 是否有效
     */
    bool validateConfigItem(ConfigKey key, const QVariant& value) const;

    /**
     * @brief 创建配置目录
     * @param path 目录路径
     * @return 创建是否成功
     */
    bool createConfigDirectory(const QString& path);

private:
    static UtilsConfig* s_instance;     ///< 单例实例
    static QMutex s_mutex;              ///< 线程安全互斥锁

    QVariantMap m_configuration;        ///< 配置数据
    QVariantMap m_defaultConfiguration; ///< 默认配置
    QString m_configFilePath;           ///< 配置文件路径
    bool m_modified;                    ///< 是否已修改
    QSettings* m_settings;              ///< Qt设置对象

    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(UtilsConfig)
};

#endif // UTILSCONFIG_H