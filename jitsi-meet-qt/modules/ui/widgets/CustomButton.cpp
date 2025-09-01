#include "CustomButton.h"
#include "../themes/BaseTheme.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

CustomButton::CustomButton(QWidget *parent)
    : QPushButton(parent)
    , m_buttonStyle(DefaultStyle)
    , m_buttonSize(MediumSize)
    , m_iconVisible(true)
    , m_hovered(false)
    , m_pressed(false)
{
    setupButton();
}

CustomButton::CustomButton(const QString& text, QWidget *parent)
    : QPushButton(text, parent)
    , m_buttonStyle(DefaultStyle)
    , m_buttonSize(MediumSize)
    , m_iconVisible(true)
    , m_hovered(false)
    , m_pressed(false)
{
    setupButton();
}

CustomButton::CustomButton(const QIcon& icon, const QString& text, QWidget *parent)
    : QPushButton(icon, text, parent)
    , m_buttonStyle(DefaultStyle)
    , m_buttonSize(MediumSize)
    , m_iconVisible(true)
    , m_currentIcon(icon)
    , m_hovered(false)
    , m_pressed(false)
{
    setupButton();
}

CustomButton::~CustomButton() = default;

CustomButton::ButtonStyle CustomButton::buttonStyle() const
{
    return m_buttonStyle;
}

void CustomButton::setButtonStyle(ButtonStyle style)
{
    if (m_buttonStyle != style) {
        m_buttonStyle = style;
        updateButtonStyle();
        emit buttonStyleChanged(style);
    }
}

CustomButton::ButtonSize CustomButton::buttonSize() const
{
    return m_buttonSize;
}

void CustomButton::setButtonSize(ButtonSize size)
{
    if (m_buttonSize != size) {
        m_buttonSize = size;
        updateButtonSize();
        emit buttonSizeChanged(size);
    }
}

bool CustomButton::isIconVisible() const
{
    return m_iconVisible;
}

void CustomButton::setIconVisible(bool visible)
{
    if (m_iconVisible != visible) {
        m_iconVisible = visible;
        updateIcon();
        emit iconVisibleChanged(visible);
    }
}

QString CustomButton::iconName() const
{
    return m_iconName;
}

void CustomButton::setIconName(const QString& iconName)
{
    if (m_iconName != iconName) {
        m_iconName = iconName;
        updateIcon();
        emit iconNameChanged(iconName);
    }
}

void CustomButton::setIconFromTheme(const QString& iconName)
{
    setIconName(iconName);
    QIcon themeIcon = QIcon::fromTheme(iconName);
    if (!themeIcon.isNull()) {
        m_currentIcon = themeIcon;
        setIcon(m_currentIcon);
    }
}

void CustomButton::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        return;
    }

    onThemeChanged(theme);
    updateThemeColors();
    updateThemeFonts();
    updateThemeSizes();
    updateButtonStyle();
}

QVariantMap CustomButton::getConfiguration() const
{
    QVariantMap config = getDefaultConfiguration();
    config["buttonStyle"] = static_cast<int>(m_buttonStyle);
    config["buttonSize"] = static_cast<int>(m_buttonSize);
    config["iconVisible"] = m_iconVisible;
    config["iconName"] = m_iconName;
    config["text"] = text();
    return config;
}

void CustomButton::setConfiguration(const QVariantMap& config)
{
    if (!validateConfiguration(config)) {
        qWarning() << "Invalid configuration for CustomButton";
        return;
    }

    if (config.contains("buttonStyle")) {
        setButtonStyle(static_cast<ButtonStyle>(config["buttonStyle"].toInt()));
    }
    if (config.contains("buttonSize")) {
        setButtonSize(static_cast<ButtonSize>(config["buttonSize"].toInt()));
    }
    if (config.contains("iconVisible")) {
        setIconVisible(config["iconVisible"].toBool());
    }
    if (config.contains("iconName")) {
        setIconName(config["iconName"].toString());
    }
    if (config.contains("text")) {
        setText(config["text"].toString());
    }
}

QString CustomButton::componentName() const
{
    return "CustomButton";
}

void CustomButton::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    Q_UNUSED(theme)
    // 主题变化处理
}

QString CustomButton::getDefaultStyleSheet() const
{
    return generateStyleSheet();
}

void CustomButton::updateThemeColors()
{
    updateButtonStyle();
}

void CustomButton::updateThemeFonts()
{
    // 修复多继承歧义问题，明确指定QPushButton的setFont方法
    QPushButton::setFont(getFontForButtonSize(m_buttonSize));
}

void CustomButton::updateThemeSizes()
{
    QSize size = getSizeForButtonSize(m_buttonSize);
    // 修复多继承歧义问题，明确指定QPushButton的setMinimumSize和setMaximumSize方法
    QPushButton::setMinimumSize(size);
    QPushButton::setMaximumSize(size.width() * 3, size.height());
}

QVariantMap CustomButton::getDefaultConfiguration() const
{
    QVariantMap config;
    config["buttonStyle"] = static_cast<int>(DefaultStyle);
    config["buttonSize"] = static_cast<int>(MediumSize);
    config["iconVisible"] = true;
    config["iconName"] = QString();
    config["text"] = QString();
    return config;
}

bool CustomButton::validateConfiguration(const QVariantMap& config) const
{
    if (config.contains("buttonStyle")) {
        int style = config["buttonStyle"].toInt();
        if (style < DefaultStyle || style > OutlineStyle) {
            return false;
        }
    }
    
    if (config.contains("buttonSize")) {
        int size = config["buttonSize"].toInt();
        if (size < SmallSize || size > ExtraLargeSize) {
            return false;
        }
    }
    
    return true;
}

void CustomButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
}

void CustomButton::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    updateButtonStyle();
    QPushButton::enterEvent(event);
}

void CustomButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateButtonStyle();
    QPushButton::leaveEvent(event);
}

void CustomButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        updateButtonStyle();
    }
    QPushButton::mousePressEvent(event);
}

void CustomButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = false;
        updateButtonStyle();
    }
    QPushButton::mouseReleaseEvent(event);
}

void CustomButton::onClicked()
{
    // 处理点击事件
}

void CustomButton::onPressed()
{
    // 处理按下事件
}

void CustomButton::onReleased()
{
    // 处理释放事件
}

void CustomButton::setupButton()
{
    // 修复信号槽连接 - 明确指定 QObject::connect
    QObject::connect(this, QOverload<bool>::of(&QPushButton::clicked), [this](bool) { onClicked(); });
    QObject::connect(this, &QPushButton::pressed, [this]() { onPressed(); });
    QObject::connect(this, &QPushButton::released, [this]() { onReleased(); });
    
    // 设置初始样式
    updateButtonStyle();
    updateButtonSize();
    updateIcon();
}

void CustomButton::updateButtonStyle()
{
    // 修复多继承歧义问题，明确指定QPushButton的setStyleSheet方法
    QPushButton::setStyleSheet(generateStyleSheet());
}

void CustomButton::updateButtonSize()
{
    QSize size = getSizeForButtonSize(m_buttonSize);
    // 修复多继承歧义问题，明确指定QPushButton的方法
    QPushButton::setMinimumSize(size);
    QPushButton::setFont(getFontForButtonSize(m_buttonSize));
}

void CustomButton::updateIcon()
{
    if (m_iconVisible && !m_iconName.isEmpty()) {
        if (m_currentIcon.isNull()) {
            m_currentIcon = QIcon::fromTheme(m_iconName);
        }
        setIcon(m_currentIcon);
    } else {
        setIcon(QIcon());
    }
}

QString CustomButton::generateStyleSheet() const
{
    QString baseStyle = getStyleForButtonStyle(m_buttonStyle);
    
    // 添加状态样式
    if (m_pressed) {
        baseStyle += ":pressed { background-color: rgba(0, 0, 0, 0.1); }";
    }
    if (m_hovered) {
        baseStyle += ":hover { background-color: rgba(255, 255, 255, 0.1); }";
    }
    
    return baseStyle;
}

QString CustomButton::getStyleForButtonStyle(ButtonStyle style) const
{
    switch (style) {
        case PrimaryStyle:
            return "QPushButton { background-color: #007ACC; color: white; border: none; border-radius: 4px; padding: 8px 16px; }";
        case SecondaryStyle:
            return "QPushButton { background-color: #6C757D; color: white; border: none; border-radius: 4px; padding: 8px 16px; }";
        case SuccessStyle:
            return "QPushButton { background-color: #28A745; color: white; border: none; border-radius: 4px; padding: 8px 16px; }";
        case WarningStyle:
            return "QPushButton { background-color: #FFC107; color: black; border: none; border-radius: 4px; padding: 8px 16px; }";
        case DangerStyle:
            return "QPushButton { background-color: #DC3545; color: white; border: none; border-radius: 4px; padding: 8px 16px; }";
        case InfoStyle:
            return "QPushButton { background-color: #17A2B8; color: white; border: none; border-radius: 4px; padding: 8px 16px; }";
        case LinkStyle:
            return "QPushButton { background-color: transparent; color: #007ACC; border: none; text-decoration: underline; }";
        case OutlineStyle:
            return "QPushButton { background-color: transparent; color: #007ACC; border: 2px solid #007ACC; border-radius: 4px; padding: 6px 14px; }";
        default:
            return "QPushButton { background-color: #F8F9FA; color: #212529; border: 1px solid #DEE2E6; border-radius: 4px; padding: 8px 16px; }";
    }
}

QSize CustomButton::getSizeForButtonSize(ButtonSize size) const
{
    switch (size) {
        case SmallSize:
            return QSize(80, 24);
        case LargeSize:
            return QSize(120, 40);
        case ExtraLargeSize:
            return QSize(140, 48);
        default: // MediumSize
            return QSize(100, 32);
    }
}

QFont CustomButton::getFontForButtonSize(ButtonSize size) const
{
    QFont font = QApplication::font();
    
    switch (size) {
        case SmallSize:
            font.setPointSize(font.pointSize() - 1);
            break;
        case LargeSize:
            font.setPointSize(font.pointSize() + 1);
            break;
        case ExtraLargeSize:
            font.setPointSize(font.pointSize() + 2);
            font.setBold(true);
            break;
        default: // MediumSize
            break;
    }
    
    return font;
}