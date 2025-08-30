#include "BaseLayout.h"
#include "themes/BaseTheme.h"
#include <QDebug>

BaseLayout::BaseLayout(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_applied(false)
    , m_enabled(true)
    , m_visible(true)
    , m_responsive(false)
{
    setupLayout();
}

BaseLayout::~BaseLayout() = default;

QString BaseLayout::layoutVersion() const
{
    return "1.0.0";
}

bool BaseLayout::isInitialized() const
{
    return m_initialized;
}

bool BaseLayout::isApplied() const
{
    return m_applied;
}

void BaseLayout::resetConfiguration()
{
    m_configuration = getDefaultConfiguration();
    onConfigurationChanged(m_configuration);
    emit configurationChanged();
}

bool BaseLayout::validateConfiguration(const QVariantMap& config) const
{
    Q_UNUSED(config)
    return true; // Base implementation accepts all configurations
}

void BaseLayout::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        onThemeChanged(theme);
        emit themeChanged();
    }
}

void BaseLayout::refreshTheme()
{
    if (m_currentTheme) {
        onThemeChanged(m_currentTheme);
        emit themeChanged();
    }
}

std::shared_ptr<BaseTheme> BaseLayout::currentTheme() const
{
    return m_currentTheme;
}

bool BaseLayout::isEnabled() const
{
    return m_enabled;
}

void BaseLayout::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(enabled);
    }
}

bool BaseLayout::isVisible() const
{
    return m_visible;
}

void BaseLayout::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        emit visibleChanged(visible);
    }
}

bool BaseLayout::validate() const
{
    return validateConfiguration(m_configuration);
}

QStringList BaseLayout::validationErrors() const
{
    QStringList errors;
    if (!validate()) {
        errors << "Configuration validation failed";
    }
    return errors;
}

void BaseLayout::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    Q_UNUSED(theme)
    updateThemeColors();
    updateThemeFonts();
    updateThemeSizes();
}

void BaseLayout::updateThemeColors()
{
    // Base implementation - override in derived classes
}

void BaseLayout::updateThemeFonts()
{
    // Base implementation - override in derived classes
}

void BaseLayout::updateThemeSizes()
{
    // Base implementation - override in derived classes
}

void BaseLayout::onConfigurationChanged(const QVariantMap& config)
{
    m_configuration = config;
    updateLayout();
}

QVariantMap BaseLayout::getDefaultConfiguration() const
{
    QVariantMap config;
    config["enabled"] = true;
    config["visible"] = true;
    config["responsive"] = false;
    return config;
}

void BaseLayout::updateLayout()
{
    updateGeometry();
    updateSpacing();
    updateMargins();
}

void BaseLayout::updateGeometry()
{
    // Base implementation - override in derived classes
}

void BaseLayout::updateSpacing()
{
    // Base implementation - override in derived classes
}

void BaseLayout::updateMargins()
{
    // Base implementation - override in derived classes
}

void BaseLayout::setInitialized(bool initialized)
{
    m_initialized = initialized;
}

void BaseLayout::setApplied(bool applied)
{
    if (m_applied != applied) {
        m_applied = applied;
        if (applied) {
            emit layoutApplied();
        } else {
            emit layoutCleanedUp();
        }
    }
}

void BaseLayout::setupLayout()
{
    m_configuration = getDefaultConfiguration();
    connectThemeSignals();
}

void BaseLayout::connectThemeSignals()
{
    // Connect theme-related signals if needed
    // This can be overridden in derived classes
}