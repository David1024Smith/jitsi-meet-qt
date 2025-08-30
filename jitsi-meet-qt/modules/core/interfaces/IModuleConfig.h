#ifndef IMODULECONFIG_H
#define IMODULECONFIG_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QJsonObject>

/**
 * @brief 模块配置接口
 * 
 * 定义模块配置的标准接口，所有模块配置类都应该实现此接口
 */
class IModuleConfig : public QObject
{
    Q_OBJECT

public:
    enum ConfigScope {
        Global,     // 全局配置
        User,       // 用户配置
        Session,    // 会话配置
        Runtime     // 运行时配置
    };
    Q_ENUM(ConfigScope)

    virtual ~IModuleConfig() = default;

    // 基本配置信息
    virtual QString moduleName() const = 0;
    virtual QString moduleVersion() const = 0;
    virtual QString moduleDescription() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    // 配置数据操作
    virtual QVariantMap toVariantMap() const = 0;
    virtual void fromVariantMap(const QVariantMap& map) = 0;
    virtual QJsonObject toJsonObject() const = 0;
    virtual void fromJsonObject(const QJsonObject& json) = 0;

    // 配置验证
    virtual bool validate() const = 0;
    virtual QStringList getValidationErrors() const = 0;
    virtual bool hasRequiredFields() const = 0;

    // 配置持久化
    virtual bool save() = 0;
    virtual bool load() = 0;
    virtual bool reset() = 0;
    virtual bool backup() = 0;
    virtual bool restore() = 0;

    // 配置作用域
    virtual ConfigScope getScope() const = 0;
    virtual void setScope(ConfigScope scope) = 0;

    // 配置变更通知
    virtual void notifyConfigChanged() = 0;

signals:
    void configChanged(const QString& key, const QVariant& value);
    void configLoaded();
    void configSaved();
    void configReset();
    void validationFailed(const QStringList& errors);
};

#endif // IMODULECONFIG_H