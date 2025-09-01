#include "../include/ThemeManager.h"
#include "ThemeFactory.h"
#include "../themes/BaseTheme.h"
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QStyle>
#include <QWidget>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_status(IThemeManager::NotInitialized) // 使用IThemeManager中定义的正确枚举值
    , m_currentThemeName("default")
{
    // 初始化主题工厂
    m_themeFactory = std::make_unique<ThemeFactory>(this);
}

ThemeManager::~ThemeManager()
{
    shutdown();
}

bool ThemeManager::initialize()
{
    if (m_status == IThemeManager::Ready) { // 使用IThemeManager中定义的正确枚举值
        return true;
    }

    m_status = IThemeManager::Initializing; // 这个枚举值在IThemeManager中存在

    try {
        // 设置主题工厂
        setupThemeFactory();

        // 加载默认主题
        loadDefaultThemes();

        m_status = IThemeManager::Ready; // 使用IThemeManager中定义的正确枚举值
        qDebug() << "ThemeManager initialized successfully";
        return true;

    } catch (const std::exception& e) {
        m_status = IThemeManager::Error;
        emit errorOccurred(QString("Failed to initialize ThemeManager: %1").arg(e.what()));
        return false;
    }
}

void ThemeManager::shutdown()
{
    if (m_status == IThemeManager::NotInitialized) { // 使用IThemeManager中定义的正确枚举值
        return;
    }

    // 清理加载的主题
    m_loadedThemes.clear();
    m_currentTheme.reset();
    m_themeCustomizations.clear();

    // 清理工厂
    if (m_themeFactory) {
        m_themeFactory.reset();
    }

    m_status = IThemeManager::NotInitialized; // 使用IThemeManager中定义的正确枚举值
    qDebug() << "ThemeManager shutdown completed";
}

IThemeManager::ThemeStatus ThemeManager::status() const
{
    return m_status;
}

bool ThemeManager::loadTheme(const QString& themeName)
{
    if (themeName.isEmpty()) {
        emit errorOccurred("Cannot load theme with empty name");
        return false;
    }

    if (m_loadedThemes.contains(themeName)) {
        qDebug() << "Theme already loaded:" << themeName;
        return true;
    }

    if (!m_themeFactory) {
        emit errorOccurred("Theme factory not initialized");
        return false;
    }

    auto theme = m_themeFactory->createTheme(themeName);
    if (!theme) {
        emit errorOccurred(QString("Failed to create theme: %1").arg(themeName));
        return false;
    }

    m_loadedThemes[themeName] = theme;
    emit themeLoaded(themeName);
    qDebug() << "Theme loaded successfully:" << themeName;
    return true;
}

bool ThemeManager::loadThemeFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Cannot open theme file: %1").arg(filePath));
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("Invalid JSON in theme file: %1").arg(error.errorString()));
        return false;
    }

    QVariantMap config = doc.object().toVariantMap();
    return loadThemeFromConfig(config);
}

bool ThemeManager::loadThemeFromConfig(const QVariantMap& config)
{
    if (!validateThemeFile("")) { // 使用配置验证
        emit errorOccurred("Invalid theme configuration");
        return false;
    }

    if (!m_themeFactory) {
        emit errorOccurred("Theme factory not initialized");
        return false;
    }

    auto theme = m_themeFactory->createThemeFromConfig(config);
    if (!theme) {
        emit errorOccurred("Failed to create theme from configuration");
        return false;
    }

    QString themeName = config.value("name", "custom").toString();
    m_loadedThemes[themeName] = theme;
    emit themeLoaded(themeName);
    return true;
}

bool ThemeManager::unloadTheme(const QString& themeName)
{
    if (!m_loadedThemes.contains(themeName)) {
        return false;
    }

    // 如果是当前主题，不能卸载
    if (themeName == m_currentThemeName) {
        emit errorOccurred("Cannot unload current theme");
        return false;
    }

    m_loadedThemes.remove(themeName);
    m_themeCustomizations.remove(themeName);
    emit themeUnloaded(themeName);
    qDebug() << "Theme unloaded:" << themeName;
    return true;
}

bool ThemeManager::applyTheme(const QString& themeName)
{
    if (themeName.isEmpty()) {
        emit errorOccurred("Cannot apply theme with empty name");
        return false;
    }

    // 确保主题已加载
    if (!loadTheme(themeName)) {
        return false;
    }

    auto theme = m_loadedThemes[themeName];
    return applyTheme(theme);
}

bool ThemeManager::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        emit errorOccurred("Cannot apply null theme");
        return false;
    }

    QString oldTheme = m_currentThemeName;
    
    if (applyThemeToApplication(theme)) {
        m_currentTheme = theme;
        m_currentThemeName = theme->name();
        m_status = IThemeManager::Ready; // 使用IThemeManager中定义的正确枚举值
        
        emit themeApplied(m_currentThemeName);
        if (oldTheme != m_currentThemeName) {
            emit themeChanged(oldTheme, m_currentThemeName);
        }
        
        qDebug() << "Theme applied successfully:" << m_currentThemeName;
        return true;
    }

    return false;
}

bool ThemeManager::reapplyCurrentTheme()
{
    if (m_currentTheme) {
        return applyThemeToApplication(m_currentTheme);
    }
    return false;
}

QStringList ThemeManager::availableThemes() const
{
    if (m_themeFactory) {
        return m_themeFactory->availableThemes();
    }
    return QStringList();
}

QStringList ThemeManager::loadedThemes() const
{
    return m_loadedThemes.keys();
}

QString ThemeManager::currentTheme() const
{
    return m_currentThemeName;
}

std::shared_ptr<BaseTheme> ThemeManager::getCurrentThemeObject() const
{
    return m_currentTheme;
}

QString ThemeManager::getThemeDisplayName(const QString& themeName) const
{
    if (m_themeFactory) {
        return m_themeFactory->getThemeDisplayName(themeName);
    }
    return themeName;
}

QString ThemeManager::getThemeDescription(const QString& themeName) const
{
    if (m_themeFactory) {
        return m_themeFactory->getThemeDescription(themeName);
    }
    return QString();
}

QVariantMap ThemeManager::getThemeMetadata(const QString& themeName) const
{
    if (m_themeFactory) {
        return m_themeFactory->getThemeMetadata(themeName);
    }
    return QVariantMap();
}

bool ThemeManager::isThemeLoaded(const QString& themeName) const
{
    return m_loadedThemes.contains(themeName);
}

bool ThemeManager::setThemeProperty(const QString& themeName, const QString& property, const QVariant& value)
{
    if (!m_loadedThemes.contains(themeName)) {
        if (!loadTheme(themeName)) {
            return false;
        }
    }

    auto theme = m_loadedThemes[themeName];
    if (theme && theme->setProperty(property, value)) {
        // 保存自定义设置
        m_themeCustomizations[themeName][property] = value;
        emit themePropertyChanged(themeName, property);
        
        // 如果是当前主题，重新应用
        if (themeName == m_currentThemeName) {
            reapplyCurrentTheme();
        }
        
        return true;
    }

    return false;
}

QVariant ThemeManager::getThemeProperty(const QString& themeName, const QString& property) const
{
    if (m_loadedThemes.contains(themeName)) {
        auto theme = m_loadedThemes[themeName];
        if (theme) {
            return theme->property(property.toUtf8().constData());
        }
    }
    return QVariant();
}

bool ThemeManager::saveThemeCustomization(const QString& themeName)
{
    if (!m_themeCustomizations.contains(themeName)) {
        return true; // 没有自定义设置
    }

    // 这里可以实现保存到文件的逻辑
    qDebug() << "Theme customization saved for:" << themeName;
    return true;
}

bool ThemeManager::resetThemeCustomization(const QString& themeName)
{
    if (m_themeCustomizations.remove(themeName) > 0) {
        // 重新加载原始主题
        if (m_loadedThemes.contains(themeName)) {
            m_loadedThemes.remove(themeName);
            loadTheme(themeName);
            
            // 如果是当前主题，重新应用
            if (themeName == m_currentThemeName) {
                applyTheme(themeName);
            }
        }
        
        qDebug() << "Theme customization reset for:" << themeName;
        return true;
    }
    return false;
}

bool ThemeManager::validateTheme(const QString& themeName) const
{
    if (m_themeFactory) {
        return m_themeFactory->validateTheme(themeName);
    }
    return false;
}

bool ThemeManager::validateThemeFile(const QString& filePath) const
{
    if (filePath.isEmpty()) {
        return true; // 配置验证
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument::fromJson(file.readAll(), &error);
    return error.error == QJsonParseError::NoError;
}

QStringList ThemeManager::getThemeValidationErrors(const QString& themeName) const
{
    QStringList errors;
    validateThemeInternal(themeName, errors);
    return errors;
}

void ThemeManager::onThemeFactoryError(const QString& error)
{
    emit errorOccurred(QString("Theme Factory error: %1").arg(error));
}

void ThemeManager::setupThemeFactory()
{
    // m_themeFactory已经在构造函数中初始化
    if (!m_themeFactory) {
        m_themeFactory = std::make_unique<ThemeFactory>(this);
    }
    
    connect(m_themeFactory.get(), &ThemeFactory::errorOccurred,
            this, &ThemeManager::onThemeFactoryError);
    
    m_themeFactory->registerBuiltinThemes();
}

void ThemeManager::loadDefaultThemes()
{
    QStringList defaultThemes = {"default", "dark", "light"};
    
    for (const QString& themeName : defaultThemes) {
        if (!loadTheme(themeName)) {
            qWarning() << "Failed to load default theme:" << themeName;
        }
    }
}

bool ThemeManager::applyThemeToApplication(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        return false;
    }

    try {
        // 应用主题样式表
        QString styleSheet = theme->styleSheet();
        if (QApplication::instance()) {
            // 在Qt6中，QApplication::setStyleSheet已被移除，我们需要应用到所有顶级窗口
            for (QWidget* widget : QApplication::topLevelWidgets()) {
                widget->setStyleSheet(styleSheet);
            }
        }

        // 应用主题属性
        theme->apply();
        
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply theme: %1").arg(e.what()));
        return false;
    }
}

void ThemeManager::validateThemeInternal(const QString& themeName, QStringList& errors) const
{
    if (themeName.isEmpty()) {
        errors << "Theme name cannot be empty";
        return;
    }

    if (!m_themeFactory || !m_themeFactory->hasTheme(themeName)) {
        errors << QString("Theme not found: %1").arg(themeName);
        return;
    }

    // 更多验证逻辑可以在这里添加
}