#include "ThemeManager.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(Theme::Light)
    , m_initialized(false)
{
    s_instance = this;
}

ThemeManager::~ThemeManager()
{
    s_instance = nullptr;
}

ThemeManager* ThemeManager::instance()
{
    return s_instance;
}

bool ThemeManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing ThemeManager...";
    
    // Set default theme
    setTheme(Theme::Light);
    
    m_initialized = true;
    qDebug() << "ThemeManager initialized successfully";
    
    return true;
}

void ThemeManager::shutdown()
{
    m_initialized = false;
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }
    
    m_currentTheme = theme;
    loadTheme(theme);
    applyTheme();
    
    emit themeChanged(theme);
    qDebug() << "Theme changed to:" << static_cast<int>(theme);
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::isDarkTheme() const
{
    return m_currentTheme == Theme::Dark;
}

QColor ThemeManager::getThemeColor(const QString& colorName) const
{
    // Return default colors based on theme
    if (isDarkTheme()) {
        if (colorName == "background") return QColor(45, 45, 45);
        if (colorName == "text") return QColor(255, 255, 255);
        if (colorName == "accent") return QColor(0, 122, 255);
    } else {
        if (colorName == "background") return QColor(255, 255, 255);
        if (colorName == "text") return QColor(0, 0, 0);
        if (colorName == "accent") return QColor(0, 122, 255);
    }
    
    return QColor();
}

QString ThemeManager::getStyleSheet() const
{
    QString styleSheet;
    
    if (isDarkTheme()) {
        styleSheet = QString(
            "QWidget { background-color: %1; color: %2; }"
            "QPushButton { background-color: %3; border: 1px solid %2; padding: 5px; }"
            "QPushButton:hover { background-color: %4; }"
        ).arg(getThemeColor("background").name())
         .arg(getThemeColor("text").name())
         .arg(QColor(60, 60, 60).name())
         .arg(QColor(80, 80, 80).name());
    } else {
        styleSheet = QString(
            "QWidget { background-color: %1; color: %2; }"
            "QPushButton { background-color: %3; border: 1px solid %4; padding: 5px; }"
            "QPushButton:hover { background-color: %5; }"
        ).arg(getThemeColor("background").name())
         .arg(getThemeColor("text").name())
         .arg(QColor(240, 240, 240).name())
         .arg(QColor(200, 200, 200).name())
         .arg(QColor(220, 220, 220).name());
    }
    
    return styleSheet;
}

void ThemeManager::onSystemThemeChanged()
{
    if (m_currentTheme == Theme::Auto) {
        // Detect system theme and apply
        // This is a simplified implementation
        loadTheme(m_currentTheme);
        applyTheme();
    }
}

void ThemeManager::loadTheme(Theme theme)
{
    Q_UNUSED(theme)
    // Load theme-specific resources, colors, etc.
    qDebug() << "Loading theme:" << static_cast<int>(theme);
}

void ThemeManager::applyTheme()
{
    // Apply the theme to the application
    if (QApplication::instance()) {
        QApplication::instance()->setStyleSheet(getStyleSheet());
    }
}

QString ThemeManager::loadStyleSheetFile(const QString& fileName) const
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return file.readAll();
    }
    
    qWarning() << "Could not load stylesheet file:" << fileName;
    return QString();
}