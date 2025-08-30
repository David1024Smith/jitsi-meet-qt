#include "MockWidget.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>

MockWidget::MockWidget(QWidget* parent)
    : BaseWidget(parent)
    , m_mockComponentName("MockWidget")
    , m_mockConfigurationResult(true)
    , m_mockThemeApplicationSuccessful(true)
    , m_applyThemeCallCount(0)
    , m_setConfigurationCallCount(0)
    , m_lastAppliedTheme(nullptr)
    , m_mockSizeHint(100, 50)
    , m_mockMinimumSize(50, 25)
    , m_mockMaximumSize(200, 100)
    , m_paintEventCount(0)
    , m_resizeEventCount(0)
{
    // Set default mock configuration
    m_mockConfiguration["componentName"] = m_mockComponentName;
    m_mockConfiguration["themeName"] = "default";
    m_mockConfiguration["themeEnabled"] = true;
    m_mockConfiguration["visible"] = true;
    m_mockConfiguration["enabled"] = true;
}

MockWidget::~MockWidget()
{
}

QString MockWidget::componentName() const
{
    return m_mockComponentName;
}

QVariantMap MockWidget::getConfiguration() const
{
    return m_mockConfiguration;
}

bool MockWidget::setConfiguration(const QVariantMap& config)
{
    m_setConfigurationCallCount++;
    m_lastConfiguration = config;
    
    emit mockConfigurationSet(config);
    
    if (m_mockConfigurationResult) {
        // Merge configuration
        for (auto it = config.begin(); it != config.end(); ++it) {
            m_mockConfiguration[it.key()] = it.value();
        }
        
        // Apply specific configuration values
        if (config.contains("componentName")) {
            m_mockComponentName = config["componentName"].toString();
        }
        if (config.contains("themeName")) {
            setThemeName(config["themeName"].toString());
        }
        if (config.contains("themeEnabled")) {
            setThemeEnabled(config["themeEnabled"].toBool());
        }
        if (config.contains("visible")) {
            setVisible(config["visible"].toBool());
        }
        if (config.contains("enabled")) {
            setEnabled(config["enabled"].toBool());
        }
    }
    
    return m_mockConfigurationResult;
}

void MockWidget::applyTheme(BaseTheme* theme)
{
    m_applyThemeCallCount++;
    m_lastAppliedTheme = theme;
    
    emit mockThemeApplied(theme);
    
    if (m_mockThemeApplicationSuccessful && theme) {
        // Simulate theme application
        setThemeName(theme->name());
        
        // Apply theme colors to palette
        QPalette palette = this->palette();
        palette.setColor(QPalette::Window, theme->backgroundColor());
        palette.setColor(QPalette::WindowText, theme->textColor());
        palette.setColor(QPalette::Base, theme->backgroundColor());
        palette.setColor(QPalette::Text, theme->textColor());
        palette.setColor(QPalette::Button, theme->primaryColor());
        palette.setColor(QPalette::ButtonText, theme->textColor());
        setPalette(palette);
        
        // Apply stylesheet if available
        if (!theme->styleSheet().isEmpty()) {
            setStyleSheet(theme->styleSheet());
        }
        
        update(); // Trigger repaint
    }
}

void MockWidget::setMockComponentName(const QString& name)
{
    m_mockComponentName = name;
    m_mockConfiguration["componentName"] = name;
}

void MockWidget::setMockConfiguration(const QVariantMap& config)
{
    m_mockConfiguration = config;
}

void MockWidget::setMockConfigurationResult(bool result)
{
    m_mockConfigurationResult = result;
}

void MockWidget::setMockThemeApplication(bool successful)
{
    m_mockThemeApplicationSuccessful = successful;
}

void MockWidget::resetCallCounts()
{
    m_applyThemeCallCount = 0;
    m_setConfigurationCallCount = 0;
    m_lastAppliedTheme = nullptr;
    m_lastConfiguration.clear();
    m_paintEventCount = 0;
    m_resizeEventCount = 0;
}

void MockWidget::setMockSizeHint(const QSize& size)
{
    m_mockSizeHint = size;
    updateGeometry();
}

void MockWidget::setMockMinimumSize(const QSize& size)
{
    m_mockMinimumSize = size;
    setMinimumSize(size);
}

void MockWidget::setMockMaximumSize(const QSize& size)
{
    m_mockMaximumSize = size;
    setMaximumSize(size);
}

QSize MockWidget::sizeHint() const
{
    return m_mockSizeHint;
}

QSize MockWidget::minimumSizeHint() const
{
    return m_mockMinimumSize;
}

void MockWidget::paintEvent(QPaintEvent* event)
{
    m_paintEventCount++;
    
    // Simple mock painting
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());
    
    // Draw component name for identification
    painter.setPen(palette().windowText().color());
    painter.drawText(rect(), Qt::AlignCenter, m_mockComponentName);
    
    BaseWidget::paintEvent(event);
}

void MockWidget::resizeEvent(QResizeEvent* event)
{
    m_resizeEventCount++;
    BaseWidget::resizeEvent(event);
}

#include "MockWidget.moc"