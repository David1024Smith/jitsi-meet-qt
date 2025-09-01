#include "StyleUtils.h"
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <QStringList>
#include <QRegularExpression>

QColor StyleUtils::adjustBrightness(const QColor& color, qreal factor)
{
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    
    l = qBound(0, static_cast<int>(l + factor * 255), 255);
    
    return QColor::fromHsl(h, s, l, a);
}

QColor StyleUtils::adjustSaturation(const QColor& color, qreal factor)
{
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    
    s = qBound(0, static_cast<int>(s + factor * 255), 255);
    
    return QColor::fromHsl(h, s, l, a);
}

QColor StyleUtils::blendColors(const QColor& color1, const QColor& color2, qreal ratio)
{
    ratio = qBound(0.0, ratio, 1.0);
    
    int r = static_cast<int>(color1.red() * (1.0 - ratio) + color2.red() * ratio);
    int g = static_cast<int>(color1.green() * (1.0 - ratio) + color2.green() * ratio);
    int b = static_cast<int>(color1.blue() * (1.0 - ratio) + color2.blue() * ratio);
    int a = static_cast<int>(color1.alpha() * (1.0 - ratio) + color2.alpha() * ratio);
    
    return QColor(r, g, b, a);
}

qreal StyleUtils::calculateContrast(const QColor& color1, const QColor& color2)
{
    qreal lum1 = getColorLuminance(color1);
    qreal lum2 = getColorLuminance(color2);
    
    qreal lighter = qMax(lum1, lum2);
    qreal darker = qMin(lum1, lum2);
    
    return (lighter + 0.05) / (darker + 0.05);
}

QColor StyleUtils::getTextColorForBackground(const QColor& backgroundColor)
{
    return isDarkColor(backgroundColor) ? Qt::white : Qt::black;
}

QString StyleUtils::colorToHex(const QColor& color, bool includeAlpha)
{
    if (includeAlpha) {
        return QString("#%1%2%3%4")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'))
            .arg(color.alpha(), 2, 16, QChar('0'));
    } else {
        return QString("#%1%2%3")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'));
    }
}

QColor StyleUtils::hexToColor(const QString& hexString)
{
    QString hex = hexString;
    if (hex.startsWith('#')) {
        hex = hex.mid(1);
    }
    
    if (hex.length() == 6) {
        bool ok;
        int rgb = hex.toInt(&ok, 16);
        if (ok) {
            return QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
        }
    } else if (hex.length() == 8) {
        bool ok;
        uint rgba = hex.toUInt(&ok, 16);
        if (ok) {
            return QColor((rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF, rgba & 0xFF);
        }
    }
    
    return QColor();
}

QString StyleUtils::createGradientStyle(const QColor& startColor, const QColor& endColor, const QString& direction)
{
    return QString("qlineargradient(%1, stop:0 %2, stop:1 %3)")
        .arg(direction)
        .arg(colorToHex(startColor))
        .arg(colorToHex(endColor));
}

QString StyleUtils::createShadowStyle(const QColor& color, int offsetX, int offsetY, int blurRadius, int spreadRadius)
{
    Q_UNUSED(spreadRadius) // CSS box-shadow equivalent not directly available in Qt
    return QString("/* Shadow effect: color=%1, offset=(%2,%3), blur=%4 */")
        .arg(colorToHex(color))
        .arg(offsetX)
        .arg(offsetY)
        .arg(blurRadius);
}

QString StyleUtils::createBorderStyle(int width, const QString& style, const QColor& color, int radius)
{
    QString borderStyle = QString("border: %1px %2 %3;").arg(width).arg(style).arg(colorToHex(color));
    if (radius > 0) {
        borderStyle += QString(" border-radius: %1px;").arg(radius);
    }
    return borderStyle;
}

QString StyleUtils::createFontStyle(const QFont& font)
{
    QString style = QString("font-family: %1; font-size: %2pt;")
        .arg(font.family())
        .arg(font.pointSize());
    
    if (font.bold()) {
        style += " font-weight: bold;";
    }
    if (font.italic()) {
        style += " font-style: italic;";
    }
    if (font.underline()) {
        style += " text-decoration: underline;";
    }
    
    return style;
}

QString StyleUtils::createMarginsStyle(const QMargins& margins)
{
    return QString("margin: %1px %2px %3px %4px;")
        .arg(margins.top())
        .arg(margins.right())
        .arg(margins.bottom())
        .arg(margins.left());
}

QString StyleUtils::createPaddingStyle(const QMargins& padding)
{
    return QString("padding: %1px %2px %3px %4px;")
        .arg(padding.top())
        .arg(padding.right())
        .arg(padding.bottom())
        .arg(padding.left());
}

QString StyleUtils::createSizeStyle(const QSize& size)
{
    return QString("width: %1px; height: %2px;")
        .arg(size.width())
        .arg(size.height());
}

QString StyleUtils::mergeStyleSheets(const QStringList& styles)
{
    return styles.join(" ");
}

QVariantMap StyleUtils::parseStyleSheet(const QString& styleSheet)
{
    QVariantMap result;
    
    // Simple CSS parser - splits by semicolon and extracts property:value pairs
    QStringList declarations = styleSheet.split(';', Qt::SkipEmptyParts);
    
    for (const QString& declaration : declarations) {
        QStringList parts = declaration.split(':', Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString property = parts[0].trimmed();
            QString value = parts[1].trimmed();
            result[property] = value;
        }
    }
    
    return result;
}

qreal StyleUtils::getColorLuminance(const QColor& color)
{
    // Calculate relative luminance according to WCAG 2.0
    auto sRGBToLinear = [](qreal c) {
        if (c <= 0.03928) {
            return c / 12.92;
        } else {
            return qPow((c + 0.055) / 1.055, 2.4);
        }
    };
    
    qreal r = sRGBToLinear(color.redF());
    qreal g = sRGBToLinear(color.greenF());
    qreal b = sRGBToLinear(color.blueF());
    
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

bool StyleUtils::isDarkColor(const QColor& color)
{
    return getColorLuminance(color) < 0.5;
}

bool StyleUtils::isLightColor(const QColor& color)
{
    return !isDarkColor(color);
}

QColor StyleUtils::getIconColor(const QColor& backgroundColor, bool isDarkTheme)
{
    if (isDarkTheme) {
        return isDarkColor(backgroundColor) ? Qt::white : Qt::black;
    } else {
        return isDarkColor(backgroundColor) ? Qt::white : Qt::black;
    }
}

QColor StyleUtils::createDisabledColor(const QColor& color)
{
    return QColor(color.red(), color.green(), color.blue(), 128); // 50% opacity
}