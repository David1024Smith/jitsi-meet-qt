#ifndef SETTINGSCONFIG_H
#define SETTINGSCONFIG_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief 设置模块配置类
 * 
 * 管理设置模块的配置参数，包括存储路径、后端选择、验证规则等。
 * 提供配置的加载、保存、验证和默认值管理功能。
 */
class SettingsConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configVersion READ configVersion WRITE setConfigVersion NOTIFY configVersionChanged)
    Q_PROPERTY(QString storageBackend READ storageBackend WRITE setStorageBackend NOTIFY storageBackendChanged)
    Q_PROPERTY(QString configPath READ configPath WRITE setConfigPath NOTIFY configPathChanged)
    Q_PROPERTY(bool encryptionEnabled READ isEncryptionEnabled WRITE setEncryptionEnabled NOTIFY encryptionEnabledChanged)
    Q_PROPERTY(bool validationEnabled READ isValidationEnabled WRITE setValidationEnabled NOTIFY validationEnabledChanged)
    Q_PROPERTY(bool autoSyncEnabled READ isAutoSyncEnabled WRITE setAutoSyncEnabled NOTIFY autoSyncEnabledChanged)
    Q_PROPERTY(int syncInterval READ syncInterval WRITE setSyncInterval NOTIFY syncIntervalChanged)

public:
    /**
     * @brief 存储后端类型
     */
    enum StorageBackendType {
        LocalFileBackend,   ///< 本地文件
        RegistryBackend,    ///< 系统注册表
        CloudBackend,       ///< 云端存储
        DatabaseBackend,    ///< 数据库存储
        MemoryBackend       ///< 内存存储
    };
    Q_ENUM(StorageBackendType)

    /**
     * @brief 配置格式类型
     */
    enum ConfigFormat {
        JsonFormat,     ///< JSON 格式
        IniFormat,      ///< INI 格式
        XmlFormat,      ///< XML 格式
        BinaryFormat    ///< 二进制格式
    };
    Q_ENUM(ConfigFormat)

    /**
     * @brief 加密算法类型
     */
    enum EncryptionType {
        NoEncryption,   ///< 无加密
        AESEncryption,  ///< AES 加密
        RSAEncryption,  ///< RSA 加密
        CustomEncryption ///< 自定义加密
    };
    Q_ENUM(EncryptionType)

    explicit SettingsConfig(QObject* parent = nullptr);
    ~SettingsConfig();

    /**
     * @brief 获取单例实例
     * @return 配置实例
     */
    static SettingsConfig* instance();

    // 基本配置属性
    QString configVersion() const;
    void setConfigVersion(const QString& version);

    QString storageBackend() const;
    void setStorageBackend(const QString& backend);
    void setStorageBackend(StorageBackendType backend);
    StorageBackendType storageBackendType() const;

    QString configPath() const;
    void setConfigPath(const QString& path);

    bool isEncryptionEnabled() const;
    void setEncryptionEnabled(bool enabled);

    bool isValidationEnabled() const;
    void setValidationEnabled(bool enabled);

    bool isAutoSyncEnabled() const;
    void setAutoSyncEnabled(bool enabled);

    int syncInterval() const;
    void setSyncInterval(int interval);

    // 高级配置
    /**
     * @brief 获取配置格式
     * @return 配置格式
     */
    ConfigFormat configFormat() const;

    /**
     * @brief 设置配置格式
     * @param format 配置格式
     */
    void setConfigFormat(ConfigFormat format);

    /**
     * @brief 获取加密类型
     * @return 加密类型
     */
    EncryptionType encryptionType() const;

    /**
     * @brief 设置加密类型
     * @param type 加密类型
     */
    void setEncryptionType(EncryptionType type);

    /**
     * @brief 获取加密密钥
     * @return 加密密钥
     */
    QString encryptionKey() const;

    /**
     * @brief 设置加密密钥
     * @param key 加密密钥
     */
    void setEncryptionKey(const QString& key);

    /**
     * @brief 获取备份目录
     * @return 备份目录路径
     */
    QString backupDirectory() const;

    /**
     * @brief 设置备份目录
     * @param directory 备份目录路径
     */
    void setBackupDirectory(const QString& directory);

    /**
     * @brief 获取最大备份数量
     * @return 最大备份数量
     */
    int maxBackupCount() const;

    /**
     * @brief 设置最大备份数量
     * @param count 最大备份数量
     */
    void setMaxBackupCount(int count);

    /**
     * @brief 获取缓存大小限制
     * @return 缓存大小（MB）
     */
    int cacheSizeLimit() const;

    /**
     * @brief 设置缓存大小限制
     * @param sizeMB 缓存大小（MB）
     */
    void setCacheSizeLimit(int sizeMB);

    /**
     * @brief 获取日志级别
     * @return 日志级别
     */
    QString logLevel() const;

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(const QString& level);

    /**
     * @brief 获取调试模式状态
     * @return 是否启用调试模式
     */
    bool isDebugMode() const;

    /**
     * @brief 设置调试模式
     * @param enabled 是否启用
     */
    void setDebugMode(bool enabled);

    // 云端配置
    /**
     * @brief 获取云端服务器URL
     * @return 服务器URL
     */
    QString cloudServerUrl() const;

    /**
     * @brief 设置云端服务器URL
     * @param url 服务器URL
     */
    void setCloudServerUrl(const QString& url);

    /**
     * @brief 获取云端认证令牌
     * @return 认证令牌
     */
    QString cloudAuthToken() const;

    /**
     * @brief 设置云端认证令牌
     * @param token 认证令牌
     */
    void setCloudAuthToken(const QString& token);

    /**
     * @brief 获取云端同步间隔
     * @return 同步间隔（秒）
     */
    int cloudSyncInterval() const;

    /**
     * @brief 设置云端同步间隔
     * @param interval 同步间隔（秒）
     */
    void setCloudSyncInterval(int interval);

    // 验证配置
    /**
     * @brief 获取验证规则文件路径
     * @return 规则文件路径
     */
    QString validationRulesPath() const;

    /**
     * @brief 设置验证规则文件路径
     * @param path 规则文件路径
     */
    void setValidationRulesPath(const QString& path);

    /**
     * @brief 获取严格验证模式状态
     * @return 是否启用严格模式
     */
    bool isStrictValidation() const;

    /**
     * @brief 设置严格验证模式
     * @param strict 是否启用严格模式
     */
    void setStrictValidation(bool strict);

    // 配置管理
    /**
     * @brief 加载配置
     * @param filePath 配置文件路径（可选）
     * @return 加载是否成功
     */
    bool loadConfiguration(const QString& filePath = QString());

    /**
     * @brief 保存配置
     * @param filePath 配置文件路径（可选）
     * @return 保存是否成功
     */
    bool saveConfiguration(const QString& filePath = QString()) const;

    /**
     * @brief 验证配置
     * @return 验证结果和错误列表
     */
    QPair<bool, QStringList> validateConfiguration() const;

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    /**
     * @brief 获取默认配置
     * @return 默认配置JSON对象
     */
    static QJsonObject defaultConfiguration();

    /**
     * @brief 导出配置到JSON
     * @return JSON对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON导入配置
     * @param json JSON对象
     * @return 导入是否成功
     */
    bool fromJson(const QJsonObject& json);

    /**
     * @brief 获取配置摘要
     * @return 配置摘要信息
     */
    QVariantMap configurationSummary() const;

    /**
     * @brief 检查配置兼容性
     * @param otherConfig 其他配置
     * @return 兼容性检查结果
     */
    bool isCompatibleWith(const SettingsConfig& otherConfig) const;

    /**
     * @brief 合并配置
     * @param otherConfig 其他配置
     * @param overwrite 是否覆盖现有值
     */
    void mergeConfiguration(const SettingsConfig& otherConfig, bool overwrite = false);

    /**
     * @brief 获取配置差异
     * @param otherConfig 其他配置
     * @return 差异列表
     */
    QStringList configurationDifferences(const SettingsConfig& otherConfig) const;

public slots:
    /**
     * @brief 重新加载配置
     */
    void reloadConfiguration();

    /**
     * @brief 应用配置更改
     */
    void applyChanges();

signals:
    /**
     * @brief 配置版本变化信号
     * @param version 新版本
     */
    void configVersionChanged(const QString& version);

    /**
     * @brief 存储后端变化信号
     * @param backend 新后端
     */
    void storageBackendChanged(const QString& backend);

    /**
     * @brief 配置路径变化信号
     * @param path 新路径
     */
    void configPathChanged(const QString& path);

    /**
     * @brief 加密启用状态变化信号
     * @param enabled 是否启用
     */
    void encryptionEnabledChanged(bool enabled);

    /**
     * @brief 验证启用状态变化信号
     * @param enabled 是否启用
     */
    void validationEnabledChanged(bool enabled);

    /**
     * @brief 自动同步启用状态变化信号
     * @param enabled 是否启用
     */
    void autoSyncEnabledChanged(bool enabled);

    /**
     * @brief 同步间隔变化信号
     * @param interval 新间隔
     */
    void syncIntervalChanged(int interval);

    /**
     * @brief 配置加载完成信号
     * @param success 是否成功
     */
    void configurationLoaded(bool success);

    /**
     * @brief 配置保存完成信号
     * @param success 是否成功
     */
    void configurationSaved(bool success);

    /**
     * @brief 配置验证完成信号
     * @param success 是否成功
     * @param errors 错误列表
     */
    void configurationValidated(bool success, const QStringList& errors);

    /**
     * @brief 配置重置信号
     */
    void configurationReset();

    /**
     * @brief 配置错误信号
     * @param error 错误信息
     */
    void configurationError(const QString& error);

private:
    void initializeDefaults();
    QString getDefaultConfigPath() const;
    bool createConfigDirectory() const;
    QJsonObject loadJsonFromFile(const QString& filePath) const;
    bool saveJsonToFile(const QJsonObject& json, const QString& filePath) const;
    QString backendTypeToString(StorageBackendType type) const;
    StorageBackendType stringToBackendType(const QString& str) const;
    QString formatToString(ConfigFormat format) const;
    ConfigFormat stringToFormat(const QString& str) const;
    QString encryptionTypeToString(EncryptionType type) const;
    EncryptionType stringToEncryptionType(const QString& str) const;

    class Private;
    std::unique_ptr<Private> d;
};

#endif // SETTINGSCONFIG_H