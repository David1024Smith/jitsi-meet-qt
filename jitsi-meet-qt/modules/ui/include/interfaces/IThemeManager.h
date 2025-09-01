#ifndef ITHEMEMANAGER_H
#define ITHEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief 主题管理器接口
 * 
 * IThemeManager定义了主题管理器的接口，负责主题的
 * 加载、应用、切换和管理等功能。
 */
class IThemeManager
{
public:
    /**
     * @brief 主题状态枚举
     */
    enum ThemeStatus {
        NotInitialized,  ///< 未初始化
        Initializing,    ///< 初始化中
        Ready,           ///< 就绪
        Error            ///< 错误状态
    };

    virtual ~IThemeManager() = default;

    // 基本管理
    /**
     * @brief 初始化主题管理器
     * @return 是否成功初始化
     */
    virtual bool initialize() = 0;

    /**
     * @brief 关闭主题管理器
     */
    virtual void shutdown() = 0;

    /**
     * @brief 获取当前状态
     * @return 主题管理器状态
     */
    virtual ThemeStatus status() const = 0;

    // 主题加载
    /**
     * @brief 加载主题
     * @param themeName 主题名称
     * @return 是否成功加载
     */
    virtual bool loadTheme(const QString& themeName) = 0;

    /**
     * @brief 从文件加载主题
     * @param filePath 文件路径
     * @return 是否成功加载
     */
    virtual bool loadThemeFromFile(const QString& filePath) = 0;

    /**
     * @brief 从配置加载主题
     * @param config 配置映射
     * @return 是否成功加载
     */
    virtual bool loadThemeFromConfig(const QVariantMap& config) = 0;

    /**
     * @brief 卸载主题
     * @param themeName 主题名称
     * @return 是否成功卸载
     */
    virtual bool unloadTheme(const QString& themeName) = 0;

    // 主题应用
    /**
     * @brief 应用主题
     * @param themeName 主题名称
     * @return 是否成功应用
     */
    virtual bool applyTheme(const QString& themeName) = 0;

    /**
     * @brief 应用主题
     * @param theme 主题对象
     * @return 是否成功应用
     */
    virtual bool applyTheme(std::shared_ptr<BaseTheme> theme) = 0;

    /**
     * @brief 重新应用当前主题
     * @return 是否成功应用
     */
    virtual bool reapplyCurrentTheme() = 0;

    // 主题查询
    /**
     * @brief 获取可用主题列表
     * @return 主题名称列表
     */
    virtual QStringList availableThemes() const = 0;

    /**
     * @brief 获取已加载主题列表
     * @return 主题名称列表
     */
    virtual QStringList loadedThemes() const = 0;

    /**
     * @brief 获取当前主题
     * @return 主题名称
     */
    virtual QString currentTheme() const = 0;

    /**
     * @brief 获取当前主题对象
     * @return 主题对象
     */
    virtual std::shared_ptr<BaseTheme> getCurrentThemeObject() const = 0;

    // 主题信息
    /**
     * @brief 获取主题显示名称
     * @param themeName 主题名称
     * @return 显示名称
     */
    virtual QString getThemeDisplayName(const QString& themeName) const = 0;

    /**
     * @brief 获取主题描述
     * @param themeName 主题名称
     * @return 描述
     */
    virtual QString getThemeDescription(const QString& themeName) const = 0;

    /**
     * @brief 获取主题元数据
     * @param themeName 主题名称
     * @return 元数据映射
     */
    virtual QVariantMap getThemeMetadata(const QString& themeName) const = 0;

    /**
     * @brief 检查主题是否已加载
     * @param themeName 主题名称
     * @return 是否已加载
     */
    virtual bool isThemeLoaded(const QString& themeName) const = 0;

    // 主题自定义
    /**
     * @brief 设置主题属性
     * @param themeName 主题名称
     * @param property 属性名称
     * @param value 属性值
     * @return 是否成功设置
     */
    virtual bool setThemeProperty(const QString& themeName, const QString& property, const QVariant& value) = 0;

    /**
     * @brief 获取主题属性
     * @param themeName 主题名称
     * @param property 属性名称
     * @return 属性值
     */
    virtual QVariant getThemeProperty(const QString& themeName, const QString& property) const = 0;

    /**
     * @brief 保存主题自定义
     * @param themeName 主题名称
     * @return 是否成功保存
     */
    virtual bool saveThemeCustomization(const QString& themeName) = 0;

    /**
     * @brief 重置主题自定义
     * @param themeName 主题名称
     * @return 是否成功重置
     */
    virtual bool resetThemeCustomization(const QString& themeName) = 0;

    // 主题验证
    /**
     * @brief 验证主题
     * @param themeName 主题名称
     * @return 是否有效
     */
    virtual bool validateTheme(const QString& themeName) const = 0;

    /**
     * @brief 验证主题文件
     * @param filePath 文件路径
     * @return 是否有效
     */
    virtual bool validateThemeFile(const QString& filePath) const = 0;

    /**
     * @brief 获取主题验证错误
     * @param themeName 主题名称
     * @return 错误列表
     */
    virtual QStringList getThemeValidationErrors(const QString& themeName) const = 0;

    // 信号定义
    virtual void themeLoaded(const QString& themeName) = 0;
    virtual void themeUnloaded(const QString& themeName) = 0;
    virtual void themeApplied(const QString& themeName) = 0;
    virtual void themeChanged(const QString& oldTheme, const QString& newTheme) = 0;
    virtual void themePropertyChanged(const QString& themeName, const QString& property) = 0;
    virtual void themeValidationFailed(const QString& themeName, const QStringList& errors) = 0;
    virtual void errorOccurred(const QString& error) = 0;
};

#endif // ITHEMEMANAGER_H
