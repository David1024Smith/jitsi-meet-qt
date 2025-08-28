#include "ThemeManager.h"
#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStyleHints>
#include <QDir>

const QString ThemeManager::SETTINGS_GROUP = "Theme";
const QString ThemeManager::THEME_KEY = "CurrentTheme";
const QString ThemeManager::SYSTEM_DETECTION_KEY = "SystemDetectionEnabled";

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(Theme::Light)
    , m_systemThemeDetectionEnabled(true)
    , m_settings(std::make_unique<QSettings>())
{
    loadThemeSettings();
    setupSystemThemeWatcher();
    
    // Apply the loaded theme
    applyTheme(m_currentTheme);
}

ThemeManager::~ThemeManager()
{
    saveThemeSettings();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }
    
    Theme oldTheme = m_currentTheme;
    m_currentTheme = theme;
    
    // If switching to system theme, detect current system theme
    if (theme == Theme::System) {
        Theme systemTheme = isSystemDarkMode() ? Theme::Dark : Theme::Light;
        applyTheme(systemTheme);
    } else {
        applyTheme(theme);
    }
    
    saveThemeSettings();
    emit themeChanged(m_currentTheme);
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

QString ThemeManager::currentThemeName() const
{
    return themeToString(m_currentTheme);
}

QStringList ThemeManager::availableThemes() const
{
    return {
        themeToString(Theme::Light),
        themeToString(Theme::Dark),
        themeToString(Theme::Modern),
        themeToString(Theme::System)
    };
}

QString ThemeManager::themeToString(Theme theme)
{
    switch (theme) {
        case Theme::Light:
            return "Light";
        case Theme::Dark:
            return "Dark";
        case Theme::Modern:
            return "Modern";
        case Theme::System:
            return "System";
        default:
            return "Light";
    }
}

ThemeManager::Theme ThemeManager::stringToTheme(const QString& themeString)
{
    if (themeString == "Dark") {
        return Theme::Dark;
    } else if (themeString == "Modern") {
        return Theme::Modern;
    } else if (themeString == "System") {
        return Theme::System;
    } else {
        return Theme::Light;
    }
}

bool ThemeManager::isSystemDarkMode() const
{
    // Use Qt's system theme detection
    return QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void ThemeManager::enableSystemThemeDetection(bool enabled)
{
    if (m_systemThemeDetectionEnabled == enabled) {
        return;
    }
    
    m_systemThemeDetectionEnabled = enabled;
    
    if (enabled && m_currentTheme == Theme::System) {
        // Re-apply system theme
        onSystemThemeChanged();
    }
    
    saveThemeSettings();
}

bool ThemeManager::isSystemThemeDetectionEnabled() const
{
    return m_systemThemeDetectionEnabled;
}

void ThemeManager::applyTheme(Theme theme)
{
    QString styleSheet = getStyleSheet(theme);
    if (!styleSheet.isEmpty()) {
        applyStyleSheet(styleSheet);
        qDebug() << "Applied theme:" << themeToString(theme);
    } else {
        qWarning() << "Failed to load stylesheet for theme:" << themeToString(theme);
    }
}

void ThemeManager::reloadCurrentTheme()
{
    // Clear cache for current theme
    m_styleSheetCache.remove(m_currentTheme);
    
    // Reapply theme
    if (m_currentTheme == Theme::System) {
        Theme systemTheme = isSystemDarkMode() ? Theme::Dark : Theme::Light;
        applyTheme(systemTheme);
    } else {
        applyTheme(m_currentTheme);
    }
}

QString ThemeManager::getStyleSheet(Theme theme) const
{
    // Check cache first
    if (m_styleSheetCache.contains(theme)) {
        return m_styleSheetCache[theme];
    }
    
    QString resourcePath;
    switch (theme) {
        case Theme::Light:
            resourcePath = ":/styles/default.qss";
            break;
        case Theme::Dark:
            resourcePath = ":/styles/dark.qss";
            break;
        case Theme::Modern:
            resourcePath = ":/styles/modern.qss";
            break;
        case Theme::System:
            // For system theme, use the appropriate theme based on system settings
            Theme systemTheme = isSystemDarkMode() ? Theme::Dark : Theme::Light;
            return getStyleSheet(systemTheme);
    }
    
    QString styleSheet = loadStyleSheetFromResource(resourcePath);
    
    // Cache the loaded stylesheet
    if (!styleSheet.isEmpty()) {
        m_styleSheetCache[theme] = styleSheet;
    }
    
    return styleSheet;
}

QString ThemeManager::getThemedIcon(const QString& iconName) const
{
    return getThemedIcon(iconName, m_currentTheme);
}

QString ThemeManager::getThemedIcon(const QString& iconName, Theme theme) const
{
    // For dark theme, try to find dark variant first
    if (theme == Theme::Dark || (theme == Theme::System && isSystemDarkMode())) {
        QString darkIconPath = QString(":/icons/%1-dark.svg").arg(iconName);
        if (QFile::exists(darkIconPath)) {
            return darkIconPath;
        }
    }
    
    // Fall back to regular icon
    return QString(":/icons/%1.svg").arg(iconName);
}

void ThemeManager::onSystemThemeChanged()
{
    if (!m_systemThemeDetectionEnabled || m_currentTheme != Theme::System) {
        return;
    }
    
    bool isDark = isSystemDarkMode();
    Theme systemTheme = isDark ? Theme::Dark : Theme::Light;
    applyTheme(systemTheme);
    
    emit systemThemeChanged(isDark);
}

void ThemeManager::loadThemeSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    
    QString themeString = m_settings->value(THEME_KEY, "Light").toString();
    m_currentTheme = stringToTheme(themeString);
    
    m_systemThemeDetectionEnabled = m_settings->value(SYSTEM_DETECTION_KEY, true).toBool();
    
    m_settings->endGroup();
}

void ThemeManager::saveThemeSettings()
{
    m_settings->beginGroup(SETTINGS_GROUP);
    
    m_settings->setValue(THEME_KEY, themeToString(m_currentTheme));
    m_settings->setValue(SYSTEM_DETECTION_KEY, m_systemThemeDetectionEnabled);
    
    m_settings->endGroup();
    m_settings->sync();
}

void ThemeManager::setupSystemThemeWatcher()
{
    // Connect to Qt's system theme change signal
    connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &ThemeManager::onSystemThemeChanged);
}

QString ThemeManager::loadStyleSheetFromResource(const QString& resourcePath) const
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open stylesheet resource:" << resourcePath;
        return QString();
    }
    
    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();
    
    return styleSheet;
}

void ThemeManager::applyStyleSheet(const QString& styleSheet)
{
    if (QApplication *app = qobject_cast<QApplication*>(QApplication::instance())) {
        app->setStyleSheet(styleSheet);
    }
}