#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief 主题管理器
 * 
 * 负责管理应用程序的主题和样式
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        Light,
        Dark,
        Auto
    };
    Q_ENUM(Theme)

    enum ThemeStatus {
        NotInitialized,
        Initializing,
        Ready,
        Error,
        ShuttingDown
    };
    Q_ENUM(ThemeStatus)

    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager();

    /**
     * @brief 初始化主题管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 关闭主题管理器
     */
    void shutdown();

    /**
     * @brief 获取当前状态
     * @return 主题管理器状态
     */
    ThemeStatus status() const;

    /**
     * @brief 设置主题
     * @param theme 主题类型
     */
    void setTheme(Theme theme);

    /**
     * @brief 获取当前主题
     * @return 当前主题
     */
    Theme currentTheme() const;

    /**
     * @brief 应用主题样式
     */
    void applyTheme();

    /**
     * @brief 应用指定主题
     * @param themeName 主题名称
     * @return 应用是否成功
     */
    bool applyTheme(const QString& themeName);

    /**
     * @brief 应用主题对象
     * @param theme 主题对象
     * @return 应用是否成功
     */
    bool applyTheme(std::shared_ptr<BaseTheme> theme);

    /**
     * @brief 获取可用主题列表
     * @return 主题名称列表
     */
    QStringList availableThemes() const;

    /**
     * @brief 加载主题
     * @param themeName 主题名称
     * @return 加载是否成功
     */
    bool loadTheme(const QString& themeName);

    /**
     * @brief 从文件加载主题
     * @param filePath 文件路径
     * @return 加载是否成功
     */
    bool loadThemeFromFile(const QString& filePath);

    /**
     * @brief 从配置加载主题
     * @param config 配置映射
     * @return 加载是否成功
     */
    bool loadThemeFromConfig(const QVariantMap& config);

    /**
     * @brief 卸载主题
     * @param themeName 主题名称
     * @return 卸载是否成功
     */
    bool unloadTheme(const QString& themeName);

    /**
     * @brief 重新应用当前主题
     * @return 应用是否成功
     */
    bool reapplyCurrentTheme();

    /**
     * @brief 获取已加载主题列表
     * @return 主题名称列表
     */
    QStringList loadedThemes() const;

    /**
     * @brief 获取当前主题名称
     * @return 当前主题名称
     */
    QString currentThemeName() const;

    /**
     * @brief 获取当前主题对象
     * @return 当前主题对象
     */
    std::shared_ptr<BaseTheme> getCurrentThemeObject() const;

    /**
     * @brief 获取主题显示名称
     * @param themeName 主题名称
     * @return 显示名称
     */
    QString getThemeDisplayName(const QString& themeName) const;

    /**
     * @brief 获取主题描述
     * @param themeName 主题名称
     * @return 描述
     */
    QString getThemeDescription(const QString& themeName) const;

    /**
     * @brief 获取主题元数据
     * @param themeName 主题名称
     * @return 元数据映射
     */
    QVariantMap getThemeMetadata(const QString& themeName) const;

    /**
     * @brief 检查主题是否已加载
     * @param themeName 主题名称
     * @return 是否已加载
     */
    bool isThemeLoaded(const QString& themeName) const;

    /**
     * @brief 设置主题属性
     * @param themeName 主题名称
     * @param property 属性名称
     * @param value 属性值
     * @return 设置是否成功
     */
    bool setThemeProperty(const QString& themeName, const QString& property, const QVariant& value);

    /**
     * @brief 获取主题属性
     * @param themeName 主题名称
     * @param property 属性名称
     * @return 属性值
     */
    QVariant getThemeProperty(const QString& themeName, const QString& property) const;

    /**
     * @brief 保存主题自定义
     * @param themeName 主题名称
     * @return 保存是否成功
     */
    bool saveThemeCustomization(const QString& themeName);

    /**
     * @brief 重置主题自定义
     * @param themeName 主题名称
     * @return 重置是否成功
     */
    bool resetThemeCustomization(const QString& themeName);

    /**
     * @brief 验证主题
     * @param themeName 主题名称
     * @return 是否有效
     */
    bool validateTheme(const QString& themeName) const;

    /**
     * @brief 验证主题文件
     * @param filePath 文件路径
     * @return 是否有效
     */
    bool validateThemeFile(const QString& filePath) const;

    /**
     * @brief 获取主题验证错误
     * @param themeName 主题名称
     * @return 错误列表
     */
    QStringList getThemeValidationErrors(const QString& themeName) const;

signals:
    /**
     * @brief 主题变化信号
     * @param theme 新主题
     */
    void themeChanged(Theme theme);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    void onThemeFactoryError(const QString& error);

private:
    void setupThemeFactory();
    void loadDefaultThemes();
    bool applyThemeToApplication(std::shared_ptr<BaseTheme> theme);
    void validateThemeInternal(const QString& themeName, QStringList& errors) const;
    
    void loadLightTheme();
    void loadDarkTheme();

    Theme m_currentTheme;
    ThemeStatus m_status;
    QString m_currentThemeName;
};

#endif // THEMEMANAGER_H