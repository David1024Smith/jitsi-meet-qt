#ifndef REPLACEMENTCONFIG_H
#define REPLACEMENTCONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QHash>

class QJsonObject;

/**
 * @brief 渐进式替换配置管理器
 * 
 * 管理替换策略、模块配置和安全设置
 */
class ReplacementConfig : public QObject
{
    Q_OBJECT

public:
    explicit ReplacementConfig(QObject *parent = nullptr);
    ~ReplacementConfig();

    // 配置文件操作
    bool loadConfiguration(const QString& configPath);
    bool saveConfiguration(const QString& configPath) const;

    // 策略配置
    QVariantMap getStrategyConfiguration(const QString& strategyName) const;
    void setStrategyConfiguration(const QString& strategyName, const QVariantMap& config);

    // 模块配置
    QVariantMap getModuleConfiguration(const QString& moduleName) const;
    void setModuleConfiguration(const QString& moduleName, const QVariantMap& config);

    // 安全配置
    QVariantMap getSafetyConfiguration() const;
    void setSafetyConfiguration(const QVariantMap& config);

    // 查询方法
    QStringList getAvailableStrategies() const;
    QStringList getConfiguredModules() const;
    bool isLoaded() const;

private:
    void loadDefaultConfiguration();
    void loadStrategies(const QJsonObject& strategies);
    void loadModuleConfigurations(const QJsonObject& modules);
    void loadSafetyConfiguration(const QJsonObject& safety);
    
    void saveStrategies(QJsonObject& strategies) const;
    void saveModuleConfigurations(QJsonObject& modules) const;
    void saveSafetyConfiguration(QJsonObject& safety) const;

    bool m_loaded;
    QHash<QString, QVariantMap> m_strategyConfigurations;
    QHash<QString, QVariantMap> m_moduleConfigurations;
    QVariantMap m_safetyConfiguration;
};

#endif // REPLACEMENTCONFIG_H