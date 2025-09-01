#include "ThemeFactory.h"
#include "../themes/BaseTheme.h"
#include "../themes/DefaultTheme.h"
#include "../themes/DarkTheme.h"
#include "../themes/LightTheme.h"
#include <QDebug>
#include <QVariantMap>

ThemeFactory* ThemeFactory::s_instance = nullptr;

ThemeFactory::ThemeFactory(QObject *parent)
    : QObject(parent)
    , m_defaultThemeName("default")
    , m_cachingEnabled(true)
{
}

ThemeFactory::~ThemeFactory()
{
    clearCache();
}

std::shared_ptr<BaseTheme> ThemeFactory::createTheme(const QString& themeName)
{
    if (themeName.isEmpty()) {
        emit errorOccurred("Cannot create theme with empty name");
        return nullptr;
    }

    // 检查缓存
    if (m_cachingEnabled) {
        auto cached = getCachedTheme(themeName);
        if (cached) {
            qDebug() << "Returning cached theme:" << themeName;
            return cached;
        } else {
            // 如果缓存项已失效，清除缓存
            clearThemeCache(themeName);
        }
    }

    // 检查是否已注册
    if (!m_registeredThemes.contains(themeName)) {
        emit errorOccurred(QString("Theme not registered: %1").arg(themeName));
        return nullptr;
    }

    try {
        // 创建主题实例
        const ThemeInfo& info = m_registeredThemes[themeName];
        auto theme = info.creator();
        
        if (!theme) {
            emit errorOccurred(QString("Failed to create theme: %1").arg(themeName));
            return nullptr;
        }

        // 缓存主题
        if (m_cachingEnabled) {
            cacheTheme(themeName, theme);
        }

        emit themeCreated(themeName);
        qDebug() << "Theme created successfully:" << themeName;
        return theme;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception creating theme %1: %2").arg(themeName, e.what()));
        return nullptr;
    }
}

std::shared_ptr<BaseTheme> ThemeFactory::createDefaultTheme()
{
    return createTheme(m_defaultThemeName);
}

std::shared_ptr<BaseTheme> ThemeFactory::createThemeFromConfig(const QVariantMap& config)
{
    if (!validateThemeConfig(config)) {
        emit errorOccurred("Invalid theme configuration");
        return nullptr;
    }

    QString themeName = config.value("name").toString();
    if (themeName.isEmpty()) {
        themeName = "custom";
    }

    // 创建基础主题
    auto theme = createTheme("default");
    if (!theme) {
        return nullptr;
    }

    // 应用配置
    theme->applyConfiguration(config);
    
    qDebug() << "Theme created from config:" << themeName;
    return theme;
}

bool ThemeFactory::registerTheme(const QString& themeName, ThemeCreator creator)
{
    if (themeName.isEmpty()) {
        emit errorOccurred("Cannot register theme with empty name");
        return false;
    }

    if (!creator) {
        emit errorOccurred("Cannot register theme with null creator");
        return false;
    }

    if (m_registeredThemes.contains(themeName)) {
        qDebug() << "Theme already registered, updating:" << themeName;
    }

    ThemeInfo info;
    info.name = themeName;
    info.displayName = themeName;
    info.description = QString("Theme: %1").arg(themeName);
    info.creator = creator;

    m_registeredThemes[themeName] = info;
    emit themeRegistered(themeName);
    qDebug() << "Theme registered:" << themeName;
    return true;
}

bool ThemeFactory::unregisterTheme(const QString& themeName)
{
    if (!m_registeredThemes.contains(themeName)) {
        return false;
    }

    // 清除缓存
    clearThemeCache(themeName);
    
    // 移除注册
    m_registeredThemes.remove(themeName);
    emit themeUnregistered(themeName);
    qDebug() << "Theme unregistered:" << themeName;
    return true;
}

bool ThemeFactory::isThemeRegistered(const QString& themeName) const
{
    return m_registeredThemes.contains(themeName);
}

QStringList ThemeFactory::availableThemes() const
{
    return m_registeredThemes.keys();
}

QStringList ThemeFactory::registeredThemes() const
{
    return m_registeredThemes.keys();
}

QString ThemeFactory::defaultThemeName() const
{
    return m_defaultThemeName;
}

bool ThemeFactory::hasTheme(const QString& themeName) const
{
    return m_registeredThemes.contains(themeName);
}

QString ThemeFactory::getThemeDisplayName(const QString& themeName) const
{
    if (m_registeredThemes.contains(themeName)) {
        return m_registeredThemes[themeName].displayName;
    }
    return themeName;
}

QString ThemeFactory::getThemeDescription(const QString& themeName) const
{
    if (m_registeredThemes.contains(themeName)) {
        return m_registeredThemes[themeName].description;
    }
    return QString();
}

QVariantMap ThemeFactory::getThemeMetadata(const QString& themeName) const
{
    if (m_registeredThemes.contains(themeName)) {
        return m_registeredThemes[themeName].metadata;
    }
    return QVariantMap();
}

void ThemeFactory::enableCaching(bool enabled)
{
    m_cachingEnabled = enabled;
    if (!enabled) {
        clearCache();
    }
}

bool ThemeFactory::isCachingEnabled() const
{
    return m_cachingEnabled;
}

void ThemeFactory::clearCache()
{
    m_themeCache.clear();
    emit cacheCleared();
    qDebug() << "Theme cache cleared";
}

void ThemeFactory::clearThemeCache(const QString& themeName)
{
    m_themeCache.remove(themeName);
}

bool ThemeFactory::validateTheme(const QString& themeName) const
{
    if (themeName.isEmpty()) {
        return false;
    }

    if (!m_registeredThemes.contains(themeName)) {
        return false;
    }

    // 尝试创建主题实例进行验证
    try {
        const ThemeInfo& info = m_registeredThemes[themeName];
        auto theme = info.creator();
        return theme != nullptr;
    } catch (...) {
        return false;
    }
}

bool ThemeFactory::validateThemeConfig(const QVariantMap& config) const
{
    // 基本验证
    if (config.isEmpty()) {
        return false;
    }

    // 检查必需字段
    QStringList requiredFields = {"name", "colors", "fonts"};
    for (const QString& field : requiredFields) {
        if (!config.contains(field)) {
            qDebug() << "Missing required field:" << field;
            return false;
        }
    }

    return true;
}

ThemeFactory* ThemeFactory::instance()
{
    if (!s_instance) {
        s_instance = new ThemeFactory();
    }
    return s_instance;
}

void ThemeFactory::registerBuiltinThemes()
{
    registerDefaultThemes();
    setupBuiltinThemes();
}

void ThemeFactory::onThemeDestroyed()
{
    // 处理主题对象销毁
    qDebug() << "Theme object destroyed";
}

void ThemeFactory::registerDefaultThemes()
{
    // 注册默认主题
    registerTheme("default", []() -> std::shared_ptr<BaseTheme> {
        return std::make_shared<DefaultTheme>();
    });

    // 注册暗色主题
    registerTheme("dark", []() -> std::shared_ptr<BaseTheme> {
        return std::make_shared<DarkTheme>();
    });

    // 注册亮色主题
    registerTheme("light", []() -> std::shared_ptr<BaseTheme> {
        return std::make_shared<LightTheme>();
    });
}

void ThemeFactory::setupBuiltinThemes()
{
    // 设置主题元数据
    if (m_registeredThemes.contains("default")) {
        m_registeredThemes["default"].displayName = "Default Theme";
        m_registeredThemes["default"].description = "Standard application theme";
        m_registeredThemes["default"].metadata["version"] = "1.0";
        m_registeredThemes["default"].metadata["author"] = "Jitsi Team";
    }

    if (m_registeredThemes.contains("dark")) {
        m_registeredThemes["dark"].displayName = "Dark Theme";
        m_registeredThemes["dark"].description = "Dark color scheme for low-light environments";
        m_registeredThemes["dark"].metadata["version"] = "1.0";
        m_registeredThemes["dark"].metadata["author"] = "Jitsi Team";
    }

    if (m_registeredThemes.contains("light")) {
        m_registeredThemes["light"].displayName = "Light Theme";
        m_registeredThemes["light"].description = "Light color scheme for bright environments";
        m_registeredThemes["light"].metadata["version"] = "1.0";
        m_registeredThemes["light"].metadata["author"] = "Jitsi Team";
    }
}

std::shared_ptr<BaseTheme> ThemeFactory::getCachedTheme(const QString& themeName) const
{
    auto it = m_themeCache.find(themeName);
    if (it != m_themeCache.end()) {
        auto theme = it->lock();
        if (theme) {
            return theme;
        } else {
            // 弱引用已失效，需要清除缓存项，但由于const限制，不能直接修改
            // 这里返回nullptr，让调用者处理缓存清理
        }
    }
    return nullptr;
}

void ThemeFactory::cacheTheme(const QString& themeName, std::shared_ptr<BaseTheme> theme)
{
    if (theme) {
        m_themeCache[themeName] = theme;
    }
}