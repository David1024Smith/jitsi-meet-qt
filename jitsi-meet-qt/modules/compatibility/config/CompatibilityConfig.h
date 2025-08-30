#ifndef COMPATIBILITYCONFIG_H
#define COMPATIBILITYCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QMutex>

/**
 * @brief 兼容性配置管理器
 * 
 * 管理兼容性适配器系统的配置选项。
 */
class CompatibilityConfig : public QObject
{
    Q_OBJECT

public:
    explicit CompatibilityConfig(QObject *parent = nullptr);
    ~CompatibilityConfig();

    bool loadConfiguration();
    bool saveConfiguration();
    
    // 全局配置
    bool isValidationEnabled() const;
    void setValidationEnabled(bool enabled);
    
    bool isPerformanceCheckEnabled() const;
    void setPerformanceCheckEnabled(bool enabled);
    
    bool isAutoRollbackEnabled() const;
    void setAutoRollbackEnabled(bool enabled);
    
    int getCheckpointRetentionDays() const;
    void setCheckpointRetentionDays(int days);
    
    int getMaxRollbackAttempts() const;
    void setMaxRollbackAttempts(int attempts);
    
    // 适配器特定配置
    QVariantMap getAdapterConfig(const QString& adapterName) const;
    void setAdapterConfig(const QString& adapterName, const QVariantMap& config);
    
    // 验证器配置
    QVariantMap getValidatorConfig() const;
    void setValidatorConfig(const QVariantMap& config);
    
    // 回滚管理器配置
    QVariantMap getRollbackConfig() const;
    void setRollbackConfig(const QVariantMap& config);
    
    // 配置文件路径
    QString getConfigFilePath() const;
    void setConfigFilePath(const QString& filePath);
    
    // 重置为默认配置
    void resetToDefaults();
    
    // 验证配置
    bool validateConfiguration() const;

signals:
    void configurationChanged();
    void configurationLoaded();
    void configurationSaved();

private:
    void setupDefaultConfiguration();
    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath);

    QString m_configFilePath;
    QVariantMap m_configuration;
    mutable QMutex m_mutex;
};

#endif // COMPATIBILITYCONFIG_H