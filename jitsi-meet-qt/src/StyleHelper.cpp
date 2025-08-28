#include "StyleHelper.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QDebug>

// Static member initialization
StyleHelper::ColorScheme StyleHelper::s_lightScheme;
StyleHelper::ColorScheme StyleHelper::s_darkScheme;
StyleHelper::ColorScheme StyleHelper::s_modernScheme;
bool StyleHelper::s_schemesInitialized = false;

StyleHelper::ColorScheme StyleHelper::getLightColorScheme()
{
    if (!s_schemesInitialized) {
        initializeColorSchemes();
    }
    return s_lightScheme;
}

StyleHelper::ColorScheme StyleHelper::getDarkColorScheme()
{
    if (!s_schemesInitialized) {
        initializeColorSchemes();
    }
    return s_darkScheme;
}

StyleHelper::ColorScheme StyleHelper::getModernColorScheme()
{
    if (!s_schemesInitialized) {
        initializeColorSchemes();
    }
    return s_modernScheme;
}

void StyleHelper::styleButton(QPushButton* button, ButtonStyle style)
{
    if (!button) return;
    
    QString styleSheet;
    ColorScheme scheme = getLightColorScheme(); // Default to light, should be determined by theme manager
    
    switch (style) {
        case ButtonStyle::Primary:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: %1;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "    min-width: 100px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "}"
                "QPushButton:pressed {"
                "    background-color: %3;"
                "}"
                "QPushButton:disabled {"
                "    background-color: #BDBDBD;"
                "    color: #757575;"
                "}"
            ).arg(colorToString(scheme.primary))
             .arg(colorToString(scheme.primaryDark))
             .arg(colorToString(adjustColorBrightness(scheme.primaryDark, -20)));
            break;
            
        case ButtonStyle::Secondary:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: transparent;"
                "    color: %1;"
                "    border: 2px solid %1;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "    min-width: 100px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "}"
                "QPushButton:pressed {"
                "    background-color: %3;"
                "}"
            ).arg(colorToString(scheme.primary))
             .arg(colorToString(QColor(scheme.primary.red(), scheme.primary.green(), scheme.primary.blue(), 25)))
             .arg(colorToString(QColor(scheme.primary.red(), scheme.primary.green(), scheme.primary.blue(), 50)));
            break;
            
        case ButtonStyle::Success:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: %1;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "    min-width: 100px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "}"
            ).arg(colorToString(scheme.success))
             .arg(colorToString(adjustColorBrightness(scheme.success, -20)));
            break;
            
        case ButtonStyle::Error:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: %1;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "    min-width: 100px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "}"
            ).arg(colorToString(scheme.error))
             .arg(colorToString(adjustColorBrightness(scheme.error, -20)));
            break;
            
        case ButtonStyle::Flat:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: transparent;"
                "    color: %1;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "}"
            ).arg(colorToString(scheme.text))
             .arg(colorToString(scheme.surface));
            break;
            
        case ButtonStyle::Outlined:
            styleSheet = QString(
                "QPushButton {"
                "    background-color: transparent;"
                "    color: %1;"
                "    border: 1px solid %2;"
                "    border-radius: 8px;"
                "    padding: 12px 24px;"
                "    font-weight: 500;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "    border-color: %1;"
                "}"
            ).arg(colorToString(scheme.text))
             .arg(colorToString(scheme.surface));
            break;
    }
    
    button->setStyleSheet(styleSheet);
}

void StyleHelper::styleLineEdit(QLineEdit* lineEdit, InputStyle style)
{
    if (!lineEdit) return;
    
    QString styleSheet;
    ColorScheme scheme = getLightColorScheme();
    
    switch (style) {
        case InputStyle::Default:
            styleSheet = QString(
                "QLineEdit {"
                "    background-color: %1;"
                "    border: 2px solid %2;"
                "    border-radius: 8px;"
                "    padding: 12px 16px;"
                "    font-size: 11pt;"
                "    color: %3;"
                "    selection-background-color: %4;"
                "}"
                "QLineEdit:focus {"
                "    border-color: %4;"
                "    outline: none;"
                "}"
                "QLineEdit:hover {"
                "    border-color: %5;"
                "}"
            ).arg(colorToString(scheme.surface))
             .arg(colorToString(QColor("#E0E0E0")))
             .arg(colorToString(scheme.text))
             .arg(colorToString(scheme.primary))
             .arg(colorToString(QColor("#BDBDBD")));
            break;
            
        case InputStyle::Rounded:
            styleSheet = QString(
                "QLineEdit {"
                "    background-color: %1;"
                "    border: 2px solid %2;"
                "    border-radius: 20px;"
                "    padding: 12px 20px;"
                "    font-size: 11pt;"
                "    color: %3;"
                "}"
                "QLineEdit:focus {"
                "    border-color: %4;"
                "}"
            ).arg(colorToString(scheme.surface))
             .arg(colorToString(QColor("#E0E0E0")))
             .arg(colorToString(scheme.text))
             .arg(colorToString(scheme.primary));
            break;
            
        case InputStyle::Outlined:
            styleSheet = QString(
                "QLineEdit {"
                "    background-color: transparent;"
                "    border: 2px solid %1;"
                "    border-radius: 8px;"
                "    padding: 12px 16px;"
                "    font-size: 11pt;"
                "    color: %2;"
                "}"
                "QLineEdit:focus {"
                "    border-color: %3;"
                "}"
            ).arg(colorToString(QColor("#E0E0E0")))
             .arg(colorToString(scheme.text))
             .arg(colorToString(scheme.primary));
            break;
            
        case InputStyle::Filled:
            styleSheet = QString(
                "QLineEdit {"
                "    background-color: %1;"
                "    border: none;"
                "    border-bottom: 2px solid %2;"
                "    border-radius: 4px 4px 0 0;"
                "    padding: 16px 12px 8px 12px;"
                "    font-size: 11pt;"
                "    color: %3;"
                "}"
                "QLineEdit:focus {"
                "    border-bottom-color: %4;"
                "}"
            ).arg(colorToString(scheme.surface))
             .arg(colorToString(QColor("#E0E0E0")))
             .arg(colorToString(scheme.text))
             .arg(colorToString(scheme.primary));
            break;
    }
    
    lineEdit->setStyleSheet(styleSheet);
}

void StyleHelper::styleLabel(QLabel* label, const QString& role)
{
    if (!label) return;
    
    ColorScheme scheme = getLightColorScheme();
    QString styleSheet;
    
    if (role == "title") {
        styleSheet = QString(
            "QLabel {"
            "    font-size: 24pt;"
            "    font-weight: bold;"
            "    color: %1;"
            "    margin: 20px 0;"
            "}"
        ).arg(colorToString(scheme.primary));
    } else if (role == "subtitle") {
        styleSheet = QString(
            "QLabel {"
            "    font-size: 12pt;"
            "    color: %1;"
            "    margin-bottom: 30px;"
            "}"
        ).arg(colorToString(scheme.textSecondary));
    } else if (role == "error") {
        styleSheet = QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: 500;"
            "    background-color: %2;"
            "    border: 1px solid %3;"
            "    border-radius: 4px;"
            "    padding: 8px 12px;"
            "}"
        ).arg(colorToString(scheme.error))
         .arg(colorToString(QColor("#FFEBEE")))
         .arg(colorToString(QColor("#FFCDD2")));
    } else if (role == "success") {
        styleSheet = QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: 500;"
            "    background-color: %2;"
            "    border: 1px solid %3;"
            "    border-radius: 4px;"
            "    padding: 8px 12px;"
            "}"
        ).arg(colorToString(scheme.success))
         .arg(colorToString(QColor("#E8F5E8")))
         .arg(colorToString(QColor("#C8E6C9")));
    } else {
        // Default label style
        styleSheet = QString(
            "QLabel {"
            "    color: %1;"
            "}"
        ).arg(colorToString(scheme.text));
    }
    
    label->setStyleSheet(styleSheet);
}

QIcon StyleHelper::createThemedIcon(const QString& iconName, const QColor& color)
{
    // This is a simplified implementation
    // In a real application, you might want to load SVG icons and recolor them
    QString iconPath = QString(":/icons/%1.svg").arg(iconName);
    return QIcon(iconPath);
}

QIcon StyleHelper::createButtonIcon(const QString& iconName, ButtonStyle style)
{
    QColor iconColor;
    ColorScheme scheme = getLightColorScheme();
    
    switch (style) {
        case ButtonStyle::Primary:
        case ButtonStyle::Success:
        case ButtonStyle::Error:
            iconColor = Qt::white;
            break;
        default:
            iconColor = scheme.text;
            break;
    }
    
    return createThemedIcon(iconName, iconColor);
}

QString StyleHelper::colorToString(const QColor& color)
{
    if (color.alpha() == 255) {
        return color.name();
    } else {
        return QString("rgba(%1, %2, %3, %4)")
               .arg(color.red())
               .arg(color.green())
               .arg(color.blue())
               .arg(color.alpha() / 255.0, 0, 'f', 2);
    }
}

QColor StyleHelper::adjustColorBrightness(const QColor& color, int factor)
{
    int r = qBound(0, color.red() + factor, 255);
    int g = qBound(0, color.green() + factor, 255);
    int b = qBound(0, color.blue() + factor, 255);
    return QColor(r, g, b, color.alpha());
}

QColor StyleHelper::blendColors(const QColor& color1, const QColor& color2, double ratio)
{
    ratio = qBound(0.0, ratio, 1.0);
    
    int r = static_cast<int>(color1.red() * (1.0 - ratio) + color2.red() * ratio);
    int g = static_cast<int>(color1.green() * (1.0 - ratio) + color2.green() * ratio);
    int b = static_cast<int>(color1.blue() * (1.0 - ratio) + color2.blue() * ratio);
    int a = static_cast<int>(color1.alpha() * (1.0 - ratio) + color2.alpha() * ratio);
    
    return QColor(r, g, b, a);
}

QString StyleHelper::createLinearGradient(const QColor& startColor, const QColor& endColor, const QString& direction)
{
    return QString("qlineargradient(x1: 0, y1: 0, x2: %1, y2: %2, stop: 0 %3, stop: 1 %4)")
           .arg(direction.contains("right") ? "1" : "0")
           .arg(direction.contains("bottom") ? "1" : "0")
           .arg(colorToString(startColor))
           .arg(colorToString(endColor));
}

QString StyleHelper::createRadialGradient(const QColor& centerColor, const QColor& edgeColor)
{
    return QString("qradialgradient(cx: 0.5, cy: 0.5, radius: 1, stop: 0 %1, stop: 1 %2)")
           .arg(colorToString(centerColor))
           .arg(colorToString(edgeColor));
}

QString StyleHelper::createTransition(const QString& property, const QString& duration, const QString& easing)
{
    // Note: Qt StyleSheets don't support CSS transitions, this is for documentation/future use
    return QString("transition: %1 %2 %3;").arg(property, duration, easing);
}

QString StyleHelper::createBoxShadow(int offsetX, int offsetY, int blur, const QColor& color, int spread)
{
    // Note: Qt StyleSheets have limited shadow support, this is for documentation
    Q_UNUSED(offsetX)
    Q_UNUSED(offsetY)
    Q_UNUSED(blur)
    Q_UNUSED(color)
    Q_UNUSED(spread)
    return QString(); // Qt doesn't support box-shadow in stylesheets
}

QString StyleHelper::createBorder(int width, const QString& style, const QColor& color)
{
    return QString("border: %1px %2 %3;")
           .arg(width)
           .arg(style)
           .arg(colorToString(color));
}

QString StyleHelper::createBorderRadius(int radius)
{
    return QString("border-radius: %1px;").arg(radius);
}

QString StyleHelper::createBorderRadius(int topLeft, int topRight, int bottomRight, int bottomLeft)
{
    return QString("border-radius: %1px %2px %3px %4px;")
           .arg(topLeft)
           .arg(topRight)
           .arg(bottomRight)
           .arg(bottomLeft);
}

int StyleHelper::getScaledSize(int baseSize)
{
    // Get system DPI scaling factor
    qreal dpr = QApplication::primaryScreen()->devicePixelRatio();
    return static_cast<int>(baseSize * dpr);
}

QString StyleHelper::getScaledFont(int baseSize, const QString& weight)
{
    int scaledSize = getScaledSize(baseSize);
    return QString("font-size: %1pt; font-weight: %2;").arg(scaledSize).arg(weight);
}

void StyleHelper::initializeColorSchemes()
{
    // Light color scheme
    s_lightScheme.primary = QColor("#2196F3");
    s_lightScheme.primaryDark = QColor("#1976D2");
    s_lightScheme.secondary = QColor("#FF9800");
    s_lightScheme.background = QColor("#FAFAFA");
    s_lightScheme.surface = QColor("#FFFFFF");
    s_lightScheme.text = QColor("#212121");
    s_lightScheme.textSecondary = QColor("#666666");
    s_lightScheme.accent = QColor("#FF5722");
    s_lightScheme.error = QColor("#F44336");
    s_lightScheme.success = QColor("#4CAF50");
    s_lightScheme.warning = QColor("#FF9800");
    
    // Dark color scheme
    s_darkScheme.primary = QColor("#64B5F6");
    s_darkScheme.primaryDark = QColor("#1976D2");
    s_darkScheme.secondary = QColor("#FFB74D");
    s_darkScheme.background = QColor("#121212");
    s_darkScheme.surface = QColor("#1E1E1E");
    s_darkScheme.text = QColor("#FFFFFF");
    s_darkScheme.textSecondary = QColor("#BDBDBD");
    s_darkScheme.accent = QColor("#FF7043");
    s_darkScheme.error = QColor("#F44336");
    s_darkScheme.success = QColor("#4CAF50");
    s_darkScheme.warning = QColor("#FF9800");
    
    // Modern color scheme (gradient-based)
    s_modernScheme.primary = QColor("#2196F3");
    s_modernScheme.primaryDark = QColor("#1565C0");
    s_modernScheme.secondary = QColor("#9C27B0");
    s_modernScheme.background = QColor("#F8F9FA");
    s_modernScheme.surface = QColor("#FFFFFF");
    s_modernScheme.text = QColor("#212121");
    s_modernScheme.textSecondary = QColor("#6C757D");
    s_modernScheme.accent = QColor("#E91E63");
    s_modernScheme.error = QColor("#DC3545");
    s_modernScheme.success = QColor("#28A745");
    s_modernScheme.warning = QColor("#FFC107");
    
    s_schemesInitialized = true;
}