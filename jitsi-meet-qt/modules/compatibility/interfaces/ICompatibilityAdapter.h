#ifndef ICOMPATIBILITYADAPTER_H
#define ICOMPATIBILITYADAPTER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

/**
 * @brief 兼容性适配器接口
 * 
 * 定义了兼容性适配器的标准接口，用于在新旧代码之间提供兼容性层。
 */
class ICompatibilityAdapter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 适配器状态枚举
     */
    enum AdapterStatus {
        NotInitialized,     ///< 未初始化
        Initializing,       ///< 初始化中
        Ready,              ///< 就绪
        Active,             ///< 活跃
        Error,              ///< 错误
        Disabled            ///< 禁用
    };
    Q_ENUM(AdapterStatus)

    /**
     * @brief 兼容性级别枚举
     */
    enum CompatibilityLevel {
        FullCompatibility,      ///< 完全兼容
        PartialCompatibility,   ///< 部分兼容
        LimitedCompatibility,   ///< 有限兼容
        NoCompatibility         ///< 不兼容
    };
    Q_ENUM(CompatibilityLevel)

    explicit ICompatibilityAdapter(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ICompatibilityAdapter() = default;

    /**
     * @brief 初始化适配器
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 获取适配器状态
     * @return 当前状态
     */
    virtual AdapterStatus status() const = 0;

    /**
     * @brief 获取适配器名称
     * @return 适配器名称
     */
    virtual QString adapterName() const = 0;

    /**
     * @brief 获取目标模块名称
     * @return 目标模块名称
     */
    virtual QString targetModule() const = 0;

    /**
     * @brief 检查兼容性级别
     * @return 兼容性级别
     */
    virtual CompatibilityLevel checkCompatibility() = 0;

    /**
     * @brief 启用适配器
     * @return 是否启用成功
     */
    virtual bool enable() = 0;

    /**
     * @brief 禁用适配器
     */
    virtual void disable() = 0;

    /**
     * @brief 获取适配器配置
     * @return 配置映射
     */
    virtual QVariantMap getConfiguration() const = 0;

    /**
     * @brief 设置适配器配置
     * @param config 配置映射
     * @return 是否设置成功
     */
    virtual bool setConfiguration(const QVariantMap& config) = 0;

    /**
     * @brief 验证适配器功能
     * @return 验证结果列表
     */
    virtual QStringList validateFunctionality() = 0;

signals:
    /**
     * @brief 状态变化信号
     * @param status 新状态
     */
    void statusChanged(AdapterStatus status);

    /**
     * @brief 兼容性检查完成信号
     * @param level 兼容性级别
     */
    void compatibilityChecked(CompatibilityLevel level);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 验证完成信号
     * @param results 验证结果
     */
    void validationCompleted(const QStringList& results);
};

#endif // ICOMPATIBILITYADAPTER_H