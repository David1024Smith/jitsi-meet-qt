#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>
#include <QStyleHints>

#ifdef Q_OS_WIN
#include <QSettings>
#include <QTimer>
#endif

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(LightTheme)
#ifdef Q_OS_WIN
    , m_systemThemeWatcher(nullptr)
#endif
{
    s_instance = this;
    setupSystemThemeMonitoring();
}

ThemeManager::~ThemeManager()
{
    s_instance = nullptr;
}

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager(qApp);
    }
    return s_instance;
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }

    Theme actualTheme = theme;
    
    // Resolve system theme to actual theme
    if (theme == SystemTheme) {
        actualTheme = detectSystemTheme();
    }

    m_currentTheme = theme; // Store the requested theme (including SystemTheme)
    applyStyleSheet(actualTheme);
    updateIconTheme();

    emit themeChanged(theme);
    
    qDebug() << "Theme changed to:" << themeToString(theme) 
             << "(actual:" << themeToString(actualTheme) << ")";
}

void ThemeManager::loadThemeFromConfig()
{
    QSettings settings;
    QString themeStr = settings.value("appearance/theme", "light").toString();
    Theme theme = themeFromString(themeStr);
    setTheme(theme);
}

void ThemeManager::saveThemeToConfig()
{
    QSettings settings;
    settings.setValue("appearance/theme", themeToString(m_currentTheme));
}

bool ThemeManager::systemSupportsDarkMode() const
{
#ifdef Q_OS_WIN
    return true; // Windows 10+ supports dark mode detection
#elif defined(Q_OS_MACOS)
    return true; // macOS supports dark mode detection
#else
    return false; // Linux support varies
#endif
}

ThemeManager::Theme ThemeManager::detectSystemTheme() const
{
    if (!systemSupportsDarkMode()) {
        return LightTheme;
    }

#ifdef Q_OS_WIN
    // Check Windows registry for dark mode setting
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      QSettings::NativeFormat);
    bool appsUseLightTheme = registry.value("AppsUseLightTheme", true).toBool();
    return appsUseLightTheme ? LightTheme : DarkTheme;
#elif defined(Q_OS_MACOS)
    // Use Qt's style hints for macOS
    return (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) 
           ? DarkTheme : LightTheme;
#else
    // For Linux, try to detect from Qt style hints
    return (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) 
           ? DarkTheme : LightTheme;
#endif
}

QString ThemeManager::themeToString(Theme theme)
{
    switch (theme) {
        case LightTheme: return "light";
        case DarkTheme: return "dark";
        case ModernTheme: return "modern";
        case SystemTheme: return "system";
        default: return "light";
    }
}

ThemeManager::Theme ThemeManager::themeFromString(const QString& themeStr)
{
    QString lower = themeStr.toLower();
    if (lower == "dark") return DarkTheme;
    if (lower == "modern") return ModernTheme;
    if (lower == "system") return SystemTheme;
    return LightTheme; // Default to light
}

void ThemeManager::onSystemThemeChanged()
{
    if (m_currentTheme == SystemTheme) {
        // Re-apply system theme to pick up changes
        Theme actualTheme = detectSystemTheme();
        applyStyleSheet(actualTheme);
        updateIconTheme();
        emit themeChanged(SystemTheme);
        
        qDebug() << "System theme changed, applied:" << themeToString(actualTheme);
    }
}

void ThemeManager::applyStyleSheet(Theme theme)
{
    QString stylesheetPath;
    
    switch (theme) {
        case DarkTheme:
            stylesheetPath = ":/styles/dark.qss";
            break;
        case ModernTheme:
            stylesheetPath = ":/styles/modern.qss";
            break;
        case LightTheme:
        default:
            stylesheetPath = ":/styles/default.qss";
            break;
    }

    QString stylesheet = loadStyleSheet(stylesheetPath);
    if (!stylesheet.isEmpty()) {
        qApp->setStyleSheet(stylesheet);
        qDebug() << "Applied stylesheet:" << stylesheetPath;
    } else {
        qWarning() << "Failed to load stylesheet:" << stylesheetPath;
    }
}

QString ThemeManager::loadStyleSheet(const QString& stylesheetPath)
{
    QFile file(stylesheetPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open stylesheet file:" << stylesheetPath;
        return QString();
    }

    QTextStream stream(&file);
    QString stylesheet = stream.readAll();
    file.close();

    return stylesheet;
}

void ThemeManager::setupSystemThemeMonitoring()
{
    if (!systemSupportsDarkMode()) {
        return;
    }

#ifdef Q_OS_WIN
    // Monitor Windows registry changes for theme updates
    // This is a simplified approach - in a real implementation,
    // you might want to use WinAPI to monitor registry changes
    QTimer* themeCheckTimer = new QTimer(this);
    connect(themeCheckTimer, &QTimer::timeout, this, &ThemeManager::onSystemThemeChanged);
    themeCheckTimer->start(5000); // Check every 5 seconds
#elif defined(Q_OS_MACOS)
    // Connect to Qt's color scheme change signal (Qt 6.5+)
    connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &ThemeManager::onSystemThemeChanged);
#endif
}

void ThemeManager::updateIconTheme()
{
    // This method can be extended to update icon themes
    // based on the current theme (light/dark icons)
    
    // For now, we'll just log the theme change
    Theme actualTheme = (m_currentTheme == SystemTheme) ? detectSystemTheme() : m_currentTheme;
    
    if (actualTheme == DarkTheme) {
        qDebug() << "Updated to dark icon theme";
        // Here you could switch to dark variants of icons
    } else if (actualTheme == ModernTheme) {
        qDebug() << "Updated to modern icon theme";
        // Here you could switch to modern variants of icons
    } else {
        qDebug() << "Updated to light icon theme";
        // Here you could switch to light variants of icons
    }
}