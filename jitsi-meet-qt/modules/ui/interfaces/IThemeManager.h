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
 * IThemeManager定义了主题管理的标准接口，包括主题加载、
 * 应用、切换和自定义等功能。
 */
class IThemeManager
{
public:
    enum ThemeStatus {
        NotLoaded,
        Loading,
        Loaded,
        Applied,
        Error
    };

    virtual ~IThemeManager() = default;

    // 生命周期管理
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual ThemeStatus status() const = 0;

    // 主题加载
    virtual bool loadTheme(const QString& themeName) = 0;
    virtual bool loadThemeFromFile(const QString& filePath) = 0;
    virtual bool loadThemeFromConfig(const QVariantMap& config) = 0;
    virtual bool unloadTheme(const QString& themeName) = 0;

    // 主题应用
    virtual bool applyTheme(const QString& themeName) = 0;
    virtual bool applyTheme(std::shared_ptr<BaseTheme> theme) = 0;
    virtual bool reapplyCurrentTheme() = 0;

    // 主题查询
    virtual QStringList availableThemes() const = 0;
    virtual QStringList loadedThemes() const = 0;
    virtual QString currentTheme() const = 0;
    virtual std::shared_ptr<BaseTheme> getCurrentThemeObject() const = 0;

    // 主题信息
    virtual QString getThemeDisplayName(const QString& themeName) const = 0;
    virtual QString getThemeDescription(const QString& themeName) const = 0;
    virtual QVariantMap getThemeMetadata(const QString& themeName) const = 0;
    virtual bool isThemeLoaded(const QString& themeName) const = 0;

    // 主题自定义
    virtual bool setThemeProperty(const QString& themeName, const QString& property, const QVariant& value) = 0;
    virtual QVariant getThemeProperty(const QString& themeName, const QString& property) const = 0;
    virtual bool saveThemeCustomization(const QString& themeName) = 0;
    virtual bool resetThemeCustomization(const QString& themeName) = 0;

    // 主题验证
    virtual bool validateTheme(const QString& themeName) const = 0;
    virtual bool validateThemeFile(const QString& filePath) const = 0;
    virtual QStringList getThemeValidationErrors(const QString& themeName) const = 0;

    // 信号接口 (需要在实现类中定义为signals)
    virtual void themeLoaded(const QString& themeName) = 0;
    virtual void themeUnloaded(const QString& themeName) = 0;
    virtual void themeApplied(const QString& themeName) = 0;
    virtual void themeChanged(const QString& oldTheme, const QString& newTheme) = 0;
    virtual void themePropertyChanged(const QString& themeName, const QString& property) = 0;
    virtual void themeValidationFailed(const QString& themeName, const QStringList& errors) = 0;
    virtual void errorOccurred(const QString& error) = 0;
};

Q_DECLARE_INTERFACE(IThemeManager, "org.jitsi.ThemeManager/1.0")

#endif // ITHEMEMANAGER_H