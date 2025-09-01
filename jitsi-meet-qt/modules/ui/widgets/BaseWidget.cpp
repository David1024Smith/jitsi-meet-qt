#include "BaseWidget.h"
#include "../themes/BaseTheme.h"
#include <QDebug>

BaseWidget::BaseWidget()
    : m_themeName("default")
    , m_themeEnabled(true)
{
    setupWidget();
    connectSignals();
    applyDefaultConfiguration();
}

BaseWidget::~BaseWidget() = default;

QString BaseWidget::themeName() const
{
    return m_themeName;
}

void BaseWidget::setThemeName(const QString& themeName)
{
    if (m_themeName != themeName) {
        m_themeName = themeName;
        // emit themeNameChanged(themeName); // 信号需要在具体的 QObject 子类中发出
        refreshTheme();
    }
}

bool BaseWidget::isThemeEnabled() const
{
    return m_themeEnabled;
}

void BaseWidget::setThemeEnabled(bool enabled)
{
    if (m_themeEnabled != enabled) {
        m_themeEnabled = enabled;
        // emit themeEnabledChanged(enabled); // 信号需要在具体的 QObject 子类中发出
        refreshTheme();
    }
}

void BaseWidget::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme || !m_themeEnabled) {
        return;
    }

    m_currentTheme = theme;
    onThemeChanged(theme);
    updateThemeColors();
    updateThemeFonts();
    updateThemeSizes();
    applyCustomStyle();
    // emit themeApplied(); // 信号需要在具体的 QObject 子类中发出
}

void BaseWidget::refreshTheme()
{
    if (!m_themeEnabled) {
        return;
    }

    // 从ThemeManager获取当前主题
    // 这里需要实现主题管理器的集成
    // auto themeManager = ThemeManager::instance();
    // if (themeManager) {
    //     auto theme = themeManager->getTheme(m_themeName);
    //     applyTheme(theme);
    // }
}

QVariantMap BaseWidget::getConfiguration() const
{
    QVariantMap config = getDefaultConfiguration();
    config["themeName"] = m_themeName;
    config["themeEnabled"] = m_themeEnabled;
    config["customStyleSheet"] = m_customStyleSheet;
    
    // 合并自定义配置
    for (auto it = m_configuration.begin(); it != m_configuration.end(); ++it) {
        config[it.key()] = it.value();
    }
    
    return config;
}

void BaseWidget::setConfiguration(const QVariantMap& config)
{
    if (!validateConfiguration(config)) {
        qWarning() << "Invalid configuration for" << componentName();
        return;
    }

    m_configuration = config;
    
    // 应用基础配置
    if (config.contains("themeName")) {
        setThemeName(config["themeName"].toString());
    }
    if (config.contains("themeEnabled")) {
        setThemeEnabled(config["themeEnabled"].toBool());
    }
    if (config.contains("customStyleSheet")) {
        setCustomStyleSheet(config["customStyleSheet"].toString());
    }

    onConfigurationChanged(config);
    // emit configurationChanged(); // 信号需要在具体的 QObject 子类中发出
}

void BaseWidget::resetConfiguration()
{
    setConfiguration(getDefaultConfiguration());
}

void BaseWidget::setCustomStyleSheet(const QString& styleSheet)
{
    if (m_customStyleSheet != styleSheet) {
        m_customStyleSheet = styleSheet;
        applyCustomStyle();
        // emit styleSheetChanged(); // 信号需要在具体的 QObject 子类中发出
    }
}

QString BaseWidget::customStyleSheet() const
{
    return m_customStyleSheet;
}

void BaseWidget::applyCustomStyle()
{
    QString finalStyleSheet = getDefaultStyleSheet();
    
    if (!m_customStyleSheet.isEmpty()) {
        finalStyleSheet += "\n" + m_customStyleSheet;
    }
    
    // setStyleSheet(finalStyleSheet); // 这需要在具体的 QWidget 子类中调用
}

bool BaseWidget::isConfigured() const
{
    return !m_configuration.isEmpty();
}

bool BaseWidget::validate() const
{
    return validateConfiguration(m_configuration);
}

QStringList BaseWidget::validationErrors() const
{
    QStringList errors;
    
    if (m_themeName.isEmpty()) {
        errors << "Theme name cannot be empty";
    }
    
    if (!validateConfiguration(m_configuration)) {
        errors << "Invalid configuration";
    }
    
    return errors;
}

QString BaseWidget::componentName() const
{
    return "BaseWidget";
}

QString BaseWidget::componentVersion() const
{
    return "1.0.0";
}

QString BaseWidget::componentDescription() const
{
    return "Base widget class providing theme support and configuration management";
}

void BaseWidget::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    Q_UNUSED(theme)
    // 子类可以重写此方法来处理主题变化
}

QString BaseWidget::getDefaultStyleSheet() const
{
    return QString();
}

void BaseWidget::updateThemeColors()
{
    if (!m_currentTheme) {
        return;
    }
    
    // 子类可以重写此方法来更新颜色
}

void BaseWidget::updateThemeFonts()
{
    if (!m_currentTheme) {
        return;
    }
    
    // 子类可以重写此方法来更新字体
}

void BaseWidget::updateThemeSizes()
{
    if (!m_currentTheme) {
        return;
    }
    
    // 子类可以重写此方法来更新尺寸
}

void BaseWidget::onConfigurationChanged(const QVariantMap& config)
{
    Q_UNUSED(config)
    // 子类可以重写此方法来处理配置变化
}

QVariantMap BaseWidget::getDefaultConfiguration() const
{
    QVariantMap config;
    config["themeName"] = "default";
    config["themeEnabled"] = true;
    config["customStyleSheet"] = QString();
    return config;
}

bool BaseWidget::validateConfiguration(const QVariantMap& config) const
{
    // 基础验证
    if (config.contains("themeName") && config["themeName"].toString().isEmpty()) {
        return false;
    }
    
    return true;
}

// 这些函数已移除，因为 BaseWidget 不再继承 QWidget
// void BaseWidget::paintEvent(QPaintEvent *event) - 需要在具体的 QWidget 子类中实现
// void BaseWidget::resizeEvent(QResizeEvent *event) - 需要在具体的 QWidget 子类中实现
// void BaseWidget::changeEvent(QEvent *event) - 需要在具体的 QWidget 子类中实现
// void BaseWidget::onGlobalThemeChanged() - 需要在具体的 QObject 子类中实现

void BaseWidget::setupWidget()
{
    // 基础设置 - 具体的 QWidget 子类需要调用 setAttribute 等函数
}

void BaseWidget::connectSignals()
{
    // 连接全局主题变化信号
    // 这里需要连接到ThemeManager的信号
}

void BaseWidget::applyDefaultConfiguration()
{
    setConfiguration(getDefaultConfiguration());
}