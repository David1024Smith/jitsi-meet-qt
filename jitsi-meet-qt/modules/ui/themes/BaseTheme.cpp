#include "BaseTheme.h"
#include <QJsonDocument>
#include <QJsonObject>

// 构造函数已在头文件中定义为内联函数
// BaseTheme::BaseTheme(QObject *parent)
//     : QObject(parent)
// {
// }

// 析构函数已在头文件中定义为内联函数
// BaseTheme::~BaseTheme() = default;

QString BaseTheme::version() const
{
    return "1.0.0";
}

QString BaseTheme::author() const
{
    return "Jitsi Meet Qt Team";
}

QVariantMap BaseTheme::toVariantMap() const
{
    QVariantMap map;
    map["name"] = name();
    map["displayName"] = displayName();
    map["description"] = description();
    map["version"] = version();
    map["author"] = author();
    
    // Colors
    QVariantMap colors;
    colors["primary"] = primaryColor().name();
    colors["secondary"] = secondaryColor().name();
    colors["background"] = backgroundColor().name();
    colors["text"] = textColor().name();
    colors["accent"] = accentColor().name();
    colors["border"] = borderColor().name();
    colors["hover"] = hoverColor().name();
    colors["pressed"] = pressedColor().name();
    colors["disabled"] = disabledColor().name();
    colors["error"] = errorColor().name();
    colors["warning"] = warningColor().name();
    colors["success"] = successColor().name();
    map["colors"] = colors;
    
    // Fonts
    QVariantMap fonts;
    fonts["default"] = defaultFont().toString();
    fonts["title"] = titleFont().toString();
    fonts["header"] = headerFont().toString();
    fonts["button"] = buttonFont().toString();
    fonts["menu"] = menuFont().toString();
    fonts["tooltip"] = tooltipFont().toString();
    map["fonts"] = fonts;
    
    // Sizes
    QVariantMap sizes;
    sizes["borderRadius"] = borderRadius();
    sizes["borderWidth"] = borderWidth();
    sizes["spacing"] = spacing();
    sizes["margin"] = margin();
    sizes["padding"] = padding();
    sizes["iconSize"] = iconSize();
    sizes["buttonHeight"] = buttonHeight();
    sizes["toolbarHeight"] = toolbarHeight();
    map["sizes"] = sizes;
    
    // Custom properties
    if (!m_customProperties.isEmpty()) {
        map["customProperties"] = m_customProperties;
    }
    
    return map;
}

void BaseTheme::fromVariantMap(const QVariantMap& map)
{
    // Load custom properties
    if (map.contains("customProperties")) {
        m_customProperties = map["customProperties"].toMap();
    }
    
    // Emit property changed signals for all custom properties
    for (auto it = m_customProperties.begin(); it != m_customProperties.end(); ++it) {
        emit propertyChanged(it.key(), it.value());
    }
}

// 删除重复定义的函数，因为在前面已经定义过了

void BaseTheme::setName(const QString& name)
{
    // 基类实现为空，子类可以重写
    Q_UNUSED(name)
}

void BaseTheme::setDisplayName(const QString& displayName)
{
    // 基类实现为空，子类可以重写
    Q_UNUSED(displayName)
}