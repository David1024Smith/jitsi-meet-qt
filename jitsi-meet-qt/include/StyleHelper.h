#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QFont>
#include <QMap>
#include <QVariantMap>

/**
 * @brief 样式助手类
 * 
 * 该类提供样式相关的辅助功能，包括颜色、字体、图标和样式表的管理。
 */
class StyleHelper : public QObject
{
    Q_OBJECT

public:
    enum ColorRole {
        Primary,
        Secondary,
        Success,
        Danger,
        Warning,
        Info,
        Light,
        Dark,
        Background,
        Surface,
        Text,
        TextSecondary,
        Border,
        Disabled,
        Highlight,
        Link
    };
    Q_ENUM(ColorRole)

    enum FontRole {
        Default,
        Title,
        Subtitle,
        Heading1,
        Heading2,
        Heading3,
        Small,
        Monospace,
        Button
    };
    Q_ENUM(FontRole)

    enum StyleTheme {
        LightTheme,
        DarkTheme,
        SystemTheme,
        CustomTheme
    };
    Q_ENUM(StyleTheme)

    explicit StyleHelper(QObject *parent = nullptr);
    ~StyleHelper();

    /**
     * @brief 获取单例实例
     */
    static StyleHelper* instance();

    /**
     * @brief 初始化样式助手
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭样式助手
     */
    void shutdown();

    /**
     * @brief 设置主题
     * @param theme 主题
     */
    void setTheme(StyleTheme theme);

    /**
     * @brief 获取当前主题
     * @return 当前主题
     */
    StyleTheme currentTheme() const;

    /**
     * @brief 获取颜色
     * @param role 颜色角色
     * @return 颜色
     */
    QColor getColor(ColorRole role) const;

    /**
     * @brief 设置颜色
     * @param role 颜色角色
     * @param color 颜色
     */
    void setColor(ColorRole role, const QColor& color);

    /**
     * @brief 获取字体
     * @param role 字体角色
     * @return 字体
     */
    QFont getFont(FontRole role) const;

    /**
     * @brief 设置字体
     * @param role 字体角色
     * @param font 字体
     */
    void setFont(FontRole role, const QFont& font);

    /**
     * @brief 获取图标路径
     * @param iconName 图标名称
     * @return 图标路径
     */
    QString getIconPath(const QString& iconName) const;

    /**
     * @brief 获取样式表
     * @param widgetType 组件类型
     * @return 样式表
     */
    QString getStyleSheet(const QString& widgetType) const;

    /**
     * @brief 获取完整样式表
     * @return 完整样式表
     */
    QString getFullStyleSheet() const;

    /**
     * @brief 加载样式表文件
     * @param filePath 文件路径
     * @return 是否成功加载
     */
    bool loadStyleSheetFile(const QString& filePath);

    /**
     * @brief 保存样式表文件
     * @param filePath 文件路径
     * @return 是否成功保存
     */
    bool saveStyleSheetFile(const QString& filePath);

    /**
     * @brief 重置为默认样式
     */
    void resetToDefaultStyle();

    /**
     * @brief 应用样式到应用程序
     */
    void applyStyleToApplication();

    /**
     * @brief 获取颜色名称
     * @param role 颜色角色
     * @return 颜色名称
     */
    static QString colorRoleName(ColorRole role);

    /**
     * @brief 获取字体角色名称
     * @param role 字体角色
     * @return 字体角色名称
     */
    static QString fontRoleName(FontRole role);

    /**
     * @brief 获取主题名称
     * @param theme 主题
     * @return 主题名称
     */
    static QString themeName(StyleTheme theme);

    /**
     * @brief 检测系统主题
     * @return 系统主题
     */
    static StyleTheme detectSystemTheme();

    /**
     * @brief 获取样式配置
     * @return 样式配置
     */
    QVariantMap getStyleConfig() const;

    /**
     * @brief 设置样式配置
     * @param config 样式配置
     */
    void setStyleConfig(const QVariantMap& config);

signals:
    /**
     * @brief 主题变更信号
     * @param theme 主题
     */
    void themeChanged(StyleTheme theme);

    /**
     * @brief 颜色变更信号
     * @param role 颜色角色
     * @param color 颜色
     */
    void colorChanged(ColorRole role, const QColor& color);

    /**
     * @brief 字体变更信号
     * @param role 字体角色
     * @param font 字体
     */
    void fontChanged(FontRole role, const QFont& font);

    /**
     * @brief 样式表变更信号
     */
    void styleSheetChanged();

private:
    void loadDefaultColors();
    void loadDefaultFonts();
    void loadDefaultStyleSheets();
    void updateStyleSheets();
    void generateFullStyleSheet();
    QString colorToStyleSheet(const QColor& color) const;
    QString fontToStyleSheet(const QFont& font) const;
    void connectSignals();

    static StyleHelper* s_instance;
    StyleTheme m_currentTheme;
    QMap<ColorRole, QColor> m_colors;
    QMap<FontRole, QFont> m_fonts;
    QMap<QString, QString> m_styleSheets;
    QMap<QString, QString> m_iconPaths;
    QString m_fullStyleSheet;
    bool m_initialized;
};

#endif // STYLEHELPER_H