#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "interfaces/IThemeManager.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

class BaseTheme;
class ThemeFactory;

/**
 * @brief 主题管理器实现类
 * 
 * ThemeManager是IThemeManager接口的具体实现，负责主题的
 * 加载、应用、切换和管理等功能。
 */
class ThemeManager : public QObject, public IThemeManager
{
    Q_OBJECT

public:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() override;

    // IThemeManager接口实现
    bool initialize() override;
    void shutdown() override;
    ThemeStatus status() const override;

    // 主题加载
    bool loadTheme(const QString& themeName) override;
    bool loadThemeFromFile(const QString& filePath) override;
    bool loadThemeFromConfig(const QVariantMap& config) override;
    bool unloadTheme(const QString& themeName) override;

    // 主题应用
    bool applyTheme(const QString& themeName) override;
    bool applyTheme(std::shared_ptr<BaseTheme> theme) override;
    bool reapplyCurrentTheme() override;

    // 主题查询
    QStringList availableThemes() const override;
    QStringList loadedThemes() const override;
    QString currentTheme() const override;
    std::shared_ptr<BaseTheme> getCurrentThemeObject() const override;

    // 主题信息
    QString getThemeDisplayName(const QString& themeName) const override;
    QString getThemeDescription(const QString& themeName) const override;
    QVariantMap getThemeMetadata(const QString& themeName) const override;
    bool isThemeLoaded(const QString& themeName) const override;

    // 主题自定义
    bool setThemeProperty(const QString& themeName, const QString& property, const QVariant& value) override;
    QVariant getThemeProperty(const QString& themeName, const QString& property) const override;
    bool saveThemeCustomization(const QString& themeName) override;
    bool resetThemeCustomization(const QString& themeName) override;

    // 主题验证
    bool validateTheme(const QString& themeName) const override;
    bool validateThemeFile(const QString& filePath) const override;
    QStringList getThemeValidationErrors(const QString& themeName) const override;

signals:
    void themeLoaded(const QString& themeName) override;
    void themeUnloaded(const QString& themeName) override;
    void themeApplied(const QString& themeName) override;
    void themeChanged(const QString& oldTheme, const QString& newTheme) override;
    void themePropertyChanged(const QString& themeName, const QString& property) override;
    void themeValidationFailed(const QString& themeName, const QStringList& errors) override;
    void errorOccurred(const QString& error) override;

private slots:
    void onThemeFactoryError(const QString& error);

private:
    void setupThemeFactory();
    void loadDefaultThemes();
    bool applyThemeToApplication(std::shared_ptr<BaseTheme> theme);
    void validateThemeInternal(const QString& themeName, QStringList& errors) const;

    ThemeStatus m_status;
    QString m_currentThemeName;
    std::shared_ptr<BaseTheme> m_currentTheme;
    
    std::unique_ptr<ThemeFactory> m_themeFactory;
    QMap<QString, std::shared_ptr<BaseTheme>> m_loadedThemes;
    QMap<QString, QVariantMap> m_themeCustomizations;
    QStringList m_validationErrors;
};

#endif // THEMEMANAGER_H