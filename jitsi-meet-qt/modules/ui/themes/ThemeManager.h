#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>

/**
 * @brief 主题管理器
 * 
 * 负责管理应用程序的主题，包括暗色和亮色主题的切换。
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        Auto
    };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();

    /**
     * @brief 获取单例实例
     */
    static ThemeManager* instance();

    /**
     * @brief 初始化主题管理器
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * @brief 关闭主题管理器
     */
    void shutdown();

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
     * @brief 检查是否为暗色主题
     * @return 是否为暗色主题
     */
    bool isDarkTheme() const;

    /**
     * @brief 获取主题颜色
     * @param colorName 颜色名称
     * @return 颜色值
     */
    QColor getThemeColor(const QString& colorName) const;

    /**
     * @brief 获取主题样式表
     * @return 样式表字符串
     */
    QString getStyleSheet() const;

signals:
    /**
     * @brief 主题更改信号
     * @param theme 新主题
     */
    void themeChanged(Theme theme);

private slots:
    void onSystemThemeChanged();

private:
    void loadTheme(Theme theme);
    void applyTheme();
    QString loadStyleSheetFile(const QString& fileName) const;

    static ThemeManager* s_instance;
    Theme m_currentTheme;
    bool m_initialized;
};

#endif // THEMEMANAGER_H