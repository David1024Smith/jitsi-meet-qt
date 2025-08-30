#include "BaseWidget.h"
#include "../themes/BaseTheme.h"
#include "../include/ThemeManager.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>

BaseWidget::BaseWidget(QWidget *parent)
    : QWidget(parent)
    , m_themeName("default")
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
        emit themeNameChanged(themeName);
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
        emit themeEnabledChanged(enabled);
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
    emit themeApplied();
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
    emit configurationChanged();
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
        emit styleSheetChanged();
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
    
    setStyleSheet(finalStyleSheet);
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

void BaseWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void BaseWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void BaseWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    
    if (event->type() == QEvent::StyleChange) {
        refreshTheme();
    }
}

void BaseWidget::onGlobalThemeChanged()
{
    refreshTheme();
}

void BaseWidget::setupWidget()
{
    // 设置基础属性
    setAttribute(Qt::WA_StyledBackground, true);
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