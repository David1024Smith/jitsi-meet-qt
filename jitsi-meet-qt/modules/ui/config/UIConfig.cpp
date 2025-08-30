#include "UIConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QDebug>

UIConfig::UIConfig(QObject *parent)
    : QObject(parent)
    , m_darkMode(false)
    , m_windowState(Normal)
    , m_windowSize(1024, 768)
    , m_windowResizable(true)
    , m_scalingMode(AutoScaling)
    , m_scalingFactor(1.0)
    , m_highDpiEnabled(true)
    , m_animationEnabled(true)
    , m_animationDuration(250)
    , m_animationEasing("OutCubic")
{
    setupDefaults();
    connectSignals();
}

UIConfig::UIConfig(const UIConfig& other)
    : QObject(other.parent())
{
    *this = other;
}

UIConfig& UIConfig::operator=(const UIConfig& other)
{
    if (this != &other) {
        m_theme = other.m_theme;
        m_language = other.m_language;
        m_layout = other.m_layout;
        m_availableThemes = other.m_availableThemes;
        m_availableLanguages = other.m_availableLanguages;
        m_availableLayouts = other.m_availableLayouts;
        m_font = other.m_font;
        m_darkMode = other.m_darkMode;
        m_primaryColor = other.m_primaryColor;
        m_secondaryColor = other.m_secondaryColor;
        m_backgroundColor = other.m_backgroundColor;
        m_windowState = other.m_windowState;
        m_windowSize = other.m_windowSize;
        m_windowResizable = other.m_windowResizable;
        m_scalingMode = other.m_scalingMode;
        m_scalingFactor = other.m_scalingFactor;
        m_highDpiEnabled = other.m_highDpiEnabled;
        m_customStyleSheet = other.m_customStyleSheet;
        m_customProperties = other.m_customProperties;
        m_animationEnabled = other.m_animationEnabled;
        m_animationDuration = other.m_animationDuration;
        m_animationEasing = other.m_animationEasing;
        
        connectSignals();
    }
    return *this;
}

UIConfig::~UIConfig() = default;

QString UIConfig::theme() const
{
    return m_theme;
}

void UIConfig::setTheme(const QString& theme)
{
    if (m_theme != theme && validateTheme(theme)) {
        m_theme = theme;
        emit themeChanged(theme);
        emit configurationChanged();
    }
}

QString UIConfig::language() const
{
    return m_language;
}

void UIConfig::setLanguage(const QString& language)
{
    if (m_language != language && validateLanguage(language)) {
        m_language = language;
        emit languageChanged(language);
        emit configurationChanged();
    }
}

QString UIConfig::layout() const
{
    return m_layout;
}

void UIConfig::setLayout(const QString& layout)
{
    if (m_layout != layout && validateLayout(layout)) {
        m_layout = layout;
        emit layoutChanged(layout);
        emit configurationChanged();
    }
}

void UIConfig::setupDefaults()
{
    m_theme = "default";
    m_language = "en_US";
    m_layout = "main";
    
    m_availableThemes = QStringList() << "default" << "dark" << "light";
    m_availableLanguages = QStringList() << "en_US" << "zh_CN" << "es_ES" << "fr_FR";
    m_availableLayouts = QStringList() << "main" << "conference" << "settings";
    
    m_font = QApplication::font();
    m_primaryColor = QColor("#007ACC");
    m_secondaryColor = QColor("#6C757D");
    m_backgroundColor = QColor("#FFFFFF");
}

void UIConfig::connectSignals()
{
    // 连接内部信号以触发配置更改通知
    connect(this, &UIConfig::themeChanged, this, &UIConfig::configurationChanged);
    connect(this, &UIConfig::languageChanged, this, &UIConfig::configurationChanged);
    connect(this, &UIConfig::layoutChanged, this, &UIConfig::configurationChanged);
}

bool UIConfig::validateTheme(const QString& theme) const
{
    return m_availableThemes.contains(theme);
}

bool UIConfig::validateLanguage(const QString& language) const
{
    return m_availableLanguages.contains(language);
}

bool UIConfig::validateLayout(const QString& layout) const
{
    return m_availableLayouts.contains(layout);
}

void UIConfig::loadDefaults()
{
    setupDefaults();
    emit configurationChanged();
}

UIConfig UIConfig::defaultConfig()
{
    UIConfig config;
    config.loadDefaults();
    return config;
}

bool UIConfig::operator==(const UIConfig& other) const
{
    return m_theme == other.m_theme &&
           m_language == other.m_language &&
           m_layout == other.m_layout &&
           m_font == other.m_font &&
           m_darkMode == other.m_darkMode;
}

bool UIConfig::operator!=(const UIConfig& other) const
{
    return !(*this == other);
}

QStringList UIConfig::availableThemes() const
{
    return m_availableThemes;
}

void UIConfig::setAvailableThemes(const QStringList& themes)
{
    m_availableThemes = themes;
}

QStringList UIConfig::availableLanguages() const
{
    return m_availableLanguages;
}

void UIConfig::setAvailableLanguages(const QStringList& languages)
{
    m_availableLanguages = languages;
}

QStringList UIConfig::availableLayouts() const
{
    return m_availableLayouts;
}

void UIConfig::setAvailableLayouts(const QStringList& layouts)
{
    m_availableLayouts = layouts;
}

int UIConfig::fontSize() const
{
    return m_font.pointSize();
}

void UIConfig::setFontSize(int size)
{
    if (m_font.pointSize() != size) {
        m_font.setPointSize(size);
        emit fontSizeChanged(size);
        emit fontChanged(m_font);
        emit configurationChanged();
    }
}

QString UIConfig::fontFamily() const
{
    return m_font.family();
}

void UIConfig::setFontFamily(const QString& family)
{
    if (m_font.family() != family) {
        m_font.setFamily(family);
        emit fontFamilyChanged(family);
        emit fontChanged(m_font);
        emit configurationChanged();
    }
}

QFont UIConfig::font() const
{
    return m_font;
}

void UIConfig::setFont(const QFont& font)
{
    if (m_font != font) {
        m_font = font;
        emit fontChanged(font);
        emit fontSizeChanged(font.pointSize());
        emit fontFamilyChanged(font.family());
        emit configurationChanged();
    }
}

bool UIConfig::isDarkMode() const
{
    return m_darkMode;
}

void UIConfig::setDarkMode(bool enabled)
{
    if (m_darkMode != enabled) {
        m_darkMode = enabled;
        emit darkModeChanged(enabled);
        emit configurationChanged();
    }
}

QColor UIConfig::primaryColor() const
{
    return m_primaryColor;
}

void UIConfig::setPrimaryColor(const QColor& color)
{
    if (m_primaryColor != color) {
        m_primaryColor = color;
        emit primaryColorChanged(color);
        emit configurationChanged();
    }
}

QColor UIConfig::secondaryColor() const
{
    return m_secondaryColor;
}

void UIConfig::setSecondaryColor(const QColor& color)
{
    if (m_secondaryColor != color) {
        m_secondaryColor = color;
        emit secondaryColorChanged(color);
        emit configurationChanged();
    }
}

QColor UIConfig::backgroundColor() const
{
    return m_backgroundColor;
}

void UIConfig::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged(color);
        emit configurationChanged();
    }
}

UIConfig::WindowState UIConfig::windowState() const
{
    return m_windowState;
}

void UIConfig::setWindowState(WindowState state)
{
    if (m_windowState != state) {
        m_windowState = state;
        emit windowStateChanged(state);
        emit configurationChanged();
    }
}

QSize UIConfig::windowSize() const
{
    return m_windowSize;
}

void UIConfig::setWindowSize(const QSize& size)
{
    if (m_windowSize != size) {
        m_windowSize = size;
        emit windowSizeChanged(size);
        emit configurationChanged();
    }
}

bool UIConfig::isWindowResizable() const
{
    return m_windowResizable;
}

void UIConfig::setWindowResizable(bool resizable)
{
    if (m_windowResizable != resizable) {
        m_windowResizable = resizable;
        emit configurationChanged();
    }
}

UIConfig::ScalingMode UIConfig::scalingMode() const
{
    return m_scalingMode;
}

void UIConfig::setScalingMode(ScalingMode mode)
{
    if (m_scalingMode != mode) {
        m_scalingMode = mode;
        emit scalingModeChanged(mode);
        emit configurationChanged();
    }
}

qreal UIConfig::scalingFactor() const
{
    return m_scalingFactor;
}

void UIConfig::setScalingFactor(qreal factor)
{
    if (qAbs(m_scalingFactor - factor) > 0.01) {
        m_scalingFactor = factor;
        emit scalingFactorChanged(factor);
        emit configurationChanged();
    }
}

bool UIConfig::isHighDpiEnabled() const
{
    return m_highDpiEnabled;
}

void UIConfig::setHighDpiEnabled(bool enabled)
{
    if (m_highDpiEnabled != enabled) {
        m_highDpiEnabled = enabled;
        emit configurationChanged();
    }
}

QString UIConfig::customStyleSheet() const
{
    return m_customStyleSheet;
}

void UIConfig::setCustomStyleSheet(const QString& styleSheet)
{
    if (m_customStyleSheet != styleSheet) {
        m_customStyleSheet = styleSheet;
        emit customStyleSheetChanged(styleSheet);
        emit configurationChanged();
    }
}

QVariantMap UIConfig::customProperties() const
{
    return m_customProperties;
}

void UIConfig::setCustomProperties(const QVariantMap& properties)
{
    m_customProperties = properties;
    emit configurationChanged();
}

void UIConfig::setCustomProperty(const QString& key, const QVariant& value)
{
    if (m_customProperties.value(key) != value) {
        m_customProperties[key] = value;
        emit customPropertyChanged(key, value);
        emit configurationChanged();
    }
}

QVariant UIConfig::customProperty(const QString& key, const QVariant& defaultValue) const
{
    return m_customProperties.value(key, defaultValue);
}

bool UIConfig::isAnimationEnabled() const
{
    return m_animationEnabled;
}

void UIConfig::setAnimationEnabled(bool enabled)
{
    if (m_animationEnabled != enabled) {
        m_animationEnabled = enabled;
        emit animationEnabledChanged(enabled);
        emit configurationChanged();
    }
}

int UIConfig::animationDuration() const
{
    return m_animationDuration;
}

void UIConfig::setAnimationDuration(int duration)
{
    if (m_animationDuration != duration) {
        m_animationDuration = duration;
        emit configurationChanged();
    }
}

QString UIConfig::animationEasing() const
{
    return m_animationEasing;
}

void UIConfig::setAnimationEasing(const QString& easing)
{
    if (m_animationEasing != easing) {
        m_animationEasing = easing;
        emit configurationChanged();
    }
}

QVariantMap UIConfig::toVariantMap() const
{
    QVariantMap map;
    
    // 主题和外观
    map["theme"] = m_theme;
    map["language"] = m_language;
    map["layout"] = m_layout;
    map["availableThemes"] = m_availableThemes;
    map["availableLanguages"] = m_availableLanguages;
    map["availableLayouts"] = m_availableLayouts;
    
    // 字体和颜色
    map["font"] = m_font.toString();
    map["darkMode"] = m_darkMode;
    map["primaryColor"] = m_primaryColor.name();
    map["secondaryColor"] = m_secondaryColor.name();
    map["backgroundColor"] = m_backgroundColor.name();
    
    // 窗口设置
    map["windowState"] = static_cast<int>(m_windowState);
    map["windowSize"] = QVariantList() << m_windowSize.width() << m_windowSize.height();
    map["windowResizable"] = m_windowResizable;
    
    // 缩放设置
    map["scalingMode"] = static_cast<int>(m_scalingMode);
    map["scalingFactor"] = m_scalingFactor;
    map["highDpiEnabled"] = m_highDpiEnabled;
    
    // 自定义设置
    map["customStyleSheet"] = m_customStyleSheet;
    map["customProperties"] = m_customProperties;
    
    // 动画设置
    map["animationEnabled"] = m_animationEnabled;
    map["animationDuration"] = m_animationDuration;
    map["animationEasing"] = m_animationEasing;
    
    return map;
}

void UIConfig::fromVariantMap(const QVariantMap& map)
{
    // 主题和外观
    if (map.contains("theme")) {
        setTheme(map["theme"].toString());
    }
    if (map.contains("language")) {
        setLanguage(map["language"].toString());
    }
    if (map.contains("layout")) {
        setLayout(map["layout"].toString());
    }
    if (map.contains("availableThemes")) {
        setAvailableThemes(map["availableThemes"].toStringList());
    }
    if (map.contains("availableLanguages")) {
        setAvailableLanguages(map["availableLanguages"].toStringList());
    }
    if (map.contains("availableLayouts")) {
        setAvailableLayouts(map["availableLayouts"].toStringList());
    }
    
    // 字体和颜色
    if (map.contains("font")) {
        QFont font;
        font.fromString(map["font"].toString());
        setFont(font);
    }
    if (map.contains("darkMode")) {
        setDarkMode(map["darkMode"].toBool());
    }
    if (map.contains("primaryColor")) {
        setPrimaryColor(QColor(map["primaryColor"].toString()));
    }
    if (map.contains("secondaryColor")) {
        setSecondaryColor(QColor(map["secondaryColor"].toString()));
    }
    if (map.contains("backgroundColor")) {
        setBackgroundColor(QColor(map["backgroundColor"].toString()));
    }
    
    // 窗口设置
    if (map.contains("windowState")) {
        setWindowState(static_cast<WindowState>(map["windowState"].toInt()));
    }
    if (map.contains("windowSize")) {
        QVariantList sizeList = map["windowSize"].toList();
        if (sizeList.size() >= 2) {
            setWindowSize(QSize(sizeList[0].toInt(), sizeList[1].toInt()));
        }
    }
    if (map.contains("windowResizable")) {
        setWindowResizable(map["windowResizable"].toBool());
    }
    
    // 缩放设置
    if (map.contains("scalingMode")) {
        setScalingMode(static_cast<ScalingMode>(map["scalingMode"].toInt()));
    }
    if (map.contains("scalingFactor")) {
        setScalingFactor(map["scalingFactor"].toReal());
    }
    if (map.contains("highDpiEnabled")) {
        setHighDpiEnabled(map["highDpiEnabled"].toBool());
    }
    
    // 自定义设置
    if (map.contains("customStyleSheet")) {
        setCustomStyleSheet(map["customStyleSheet"].toString());
    }
    if (map.contains("customProperties")) {
        setCustomProperties(map["customProperties"].toMap());
    }
    
    // 动画设置
    if (map.contains("animationEnabled")) {
        setAnimationEnabled(map["animationEnabled"].toBool());
    }
    if (map.contains("animationDuration")) {
        setAnimationDuration(map["animationDuration"].toInt());
    }
    if (map.contains("animationEasing")) {
        setAnimationEasing(map["animationEasing"].toString());
    }
}

QByteArray UIConfig::toJson() const
{
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    return doc.toJson();
}

bool UIConfig::fromJson(const QByteArray& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << error.errorString();
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool UIConfig::validate() const
{
    return validationErrors().isEmpty();
}

QStringList UIConfig::validationErrors() const
{
    QStringList errors;
    
    if (!validateTheme(m_theme)) {
        errors << QString("Invalid theme: %1").arg(m_theme);
    }
    
    if (!validateLanguage(m_language)) {
        errors << QString("Invalid language: %1").arg(m_language);
    }
    
    if (!validateLayout(m_layout)) {
        errors << QString("Invalid layout: %1").arg(m_layout);
    }
    
    if (m_scalingFactor <= 0.0 || m_scalingFactor > 5.0) {
        errors << QString("Invalid scaling factor: %1").arg(m_scalingFactor);
    }
    
    if (m_animationDuration < 0 || m_animationDuration > 5000) {
        errors << QString("Invalid animation duration: %1").arg(m_animationDuration);
    }
    
    return errors;
}

bool UIConfig::isValid() const
{
    return validate();
}

void UIConfig::resetToDefaults()
{
    loadDefaults();
}