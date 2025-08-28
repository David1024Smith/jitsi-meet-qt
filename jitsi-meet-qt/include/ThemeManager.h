#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <memory>

class QSettings;

/**
 * @brief The ThemeManager class handles application theming and style management
 * 
 * This class provides functionality to:
 * - Load and apply different themes (light, dark, modern)
 * - Manage theme persistence across application sessions
 * - Provide theme switching capabilities
 * - Handle system theme detection
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        Modern,
        System  // Follow system theme
    };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();

    // Theme management
    void setTheme(Theme theme);
    Theme currentTheme() const;
    QString currentThemeName() const;
    
    // Available themes
    QStringList availableThemes() const;
    static QString themeToString(Theme theme);
    static Theme stringToTheme(const QString& themeString);
    
    // System theme detection
    bool isSystemDarkMode() const;
    void enableSystemThemeDetection(bool enabled);
    bool isSystemThemeDetectionEnabled() const;
    
    // Style management
    void applyTheme(Theme theme);
    void reloadCurrentTheme();
    QString getStyleSheet(Theme theme) const;
    
    // Icon management
    QString getThemedIcon(const QString& iconName) const;
    QString getThemedIcon(const QString& iconName, Theme theme) const;

signals:
    void themeChanged(Theme newTheme);
    void systemThemeChanged(bool isDark);

private slots:
    void onSystemThemeChanged();

private:
    void loadThemeSettings();
    void saveThemeSettings();
    void setupSystemThemeWatcher();
    QString loadStyleSheetFromResource(const QString& resourcePath) const;
    void applyStyleSheet(const QString& styleSheet);
    
    Theme m_currentTheme;
    bool m_systemThemeDetectionEnabled;
    std::unique_ptr<QSettings> m_settings;
    
    // Style sheet cache
    mutable QHash<Theme, QString> m_styleSheetCache;
    
    static const QString SETTINGS_GROUP;
    static const QString THEME_KEY;
    static const QString SYSTEM_DETECTION_KEY;
};

#endif // THEMEMANAGER_H