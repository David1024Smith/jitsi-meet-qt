#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QApplication>

/**
 * @brief Manages application themes and styling
 * 
 * The ThemeManager class handles loading and switching between different
 * visual themes (light/dark) for the Jitsi Meet Qt application.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        LightTheme,
        DarkTheme,
        SystemTheme  // Follow system preference
    };

    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();

    /**
     * @brief Get the singleton instance
     * @return ThemeManager instance
     */
    static ThemeManager* instance();

    /**
     * @brief Get current theme
     * @return Current theme enum
     */
    Theme currentTheme() const;

    /**
     * @brief Set the application theme
     * @param theme Theme to apply
     */
    void setTheme(Theme theme);

    /**
     * @brief Load theme from configuration
     */
    void loadThemeFromConfig();

    /**
     * @brief Save current theme to configuration
     */
    void saveThemeToConfig();

    /**
     * @brief Check if system supports dark mode detection
     * @return True if system dark mode can be detected
     */
    bool systemSupportsDarkMode() const;

    /**
     * @brief Detect system theme preference
     * @return Detected system theme
     */
    Theme detectSystemTheme() const;

    /**
     * @brief Get theme name as string
     * @param theme Theme enum
     * @return Theme name
     */
    static QString themeToString(Theme theme);

    /**
     * @brief Parse theme from string
     * @param themeStr Theme string
     * @return Theme enum
     */
    static Theme themeFromString(const QString& themeStr);

signals:
    /**
     * @brief Emitted when theme changes
     * @param theme New theme
     */
    void themeChanged(Theme theme);

private slots:
    /**
     * @brief Handle system theme changes (Windows/macOS)
     */
    void onSystemThemeChanged();

private:
    /**
     * @brief Apply stylesheet to application
     * @param theme Theme to apply
     */
    void applyStyleSheet(Theme theme);

    /**
     * @brief Load stylesheet from resources
     * @param stylesheetPath Path to QSS file
     * @return Stylesheet content
     */
    QString loadStyleSheet(const QString& stylesheetPath);

    /**
     * @brief Setup system theme monitoring
     */
    void setupSystemThemeMonitoring();

    /**
     * @brief Update icon theme based on current theme
     */
    void updateIconTheme();

    Theme m_currentTheme;
    static ThemeManager* s_instance;

#ifdef Q_OS_WIN
    void* m_systemThemeWatcher; // Windows theme change watcher
#endif
};

#endif // THEMEMANAGER_H