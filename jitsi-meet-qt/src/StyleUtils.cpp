#include "StyleUtils.h"
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <cmath>

void StyleUtils::applyRoundedCorners(QWidget* widget, int radius)
{
    if (!widget) return;
    
    QString style = QString("border-radius: %1px;").arg(radius);
    widget->setStyleSheet(widget->styleSheet() + style);
}

void StyleUtils::applyShadow(QWidget* widget, const QColor& color, int blurRadius, const QPoint& offset)
{
    if (!widget) return;
    
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setColor(color);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    widget->setGraphicsEffect(shadow);
}

QString StyleUtils::generateButtonStyle(const QColor& backgroundColor,
                                      const QColor& textColor,
                                      const QColor& hoverColor,
                                      const QColor& pressedColor)
{
    return QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: %3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %4;"
        "}"
    ).arg(backgroundColor.name())
     .arg(textColor.name())
     .arg(hoverColor.name())
     .arg(pressedColor.name());
}

QString StyleUtils::generateInputStyle(const QColor& backgroundColor,
                                     const QColor& borderColor,
                                     const QColor& focusColor)
{
    return QString(
        "QLineEdit {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 8px;"
        "    padding: 12px 16px;"
        "    font-size: 11pt;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %3;"
        "    outline: none;"
        "}"
        "QLineEdit:hover {"
        "    border-color: %4;"
        "}"
    ).arg(backgroundColor.name())
     .arg(borderColor.name())
     .arg(focusColor.name())
     .arg(lightenColor(borderColor, 20).name());
}

QColor StyleUtils::lightenColor(const QColor& color, int percentage)
{
    if (percentage <= 0) return color;
    if (percentage >= 100) return QColor(255, 255, 255);
    
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    
    r = r + (255 - r) * percentage / 100;
    g = g + (255 - g) * percentage / 100;
    b = b + (255 - b) * percentage / 100;
    
    return QColor(qMin(255, r), qMin(255, g), qMin(255, b), color.alpha());
}

QColor StyleUtils::darkenColor(const QColor& color, int percentage)
{
    if (percentage <= 0) return color;
    if (percentage >= 100) return QColor(0, 0, 0);
    
    int r = color.red() * (100 - percentage) / 100;
    int g = color.green() * (100 - percentage) / 100;
    int b = color.blue() * (100 - percentage) / 100;
    
    return QColor(qMax(0, r), qMax(0, g), qMax(0, b), color.alpha());
}

QColor StyleUtils::withAlpha(const QColor& color, int alpha)
{
    QColor result = color;
    result.setAlpha(qBound(0, alpha, 255));
    return result;
}

QColor StyleUtils::getContrastingTextColor(const QColor& backgroundColor)
{
    // Calculate luminance using relative luminance formula
    double r = backgroundColor.redF();
    double g = backgroundColor.greenF();
    double b = backgroundColor.blueF();
    
    // Apply gamma correction
    r = (r <= 0.03928) ? r / 12.92 : std::pow((r + 0.055) / 1.055, 2.4);
    g = (g <= 0.03928) ? g / 12.92 : std::pow((g + 0.055) / 1.055, 2.4);
    b = (b <= 0.03928) ? b / 12.92 : std::pow((b + 0.055) / 1.055, 2.4);
    
    double luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;
    
    // Return white for dark backgrounds, black for light backgrounds
    return (luminance > 0.5) ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

bool StyleUtils::isDarkColor(const QColor& color)
{
    // Simple brightness calculation
    int brightness = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;
    return brightness < 128;
}

void StyleUtils::applyLoadingStyle(QWidget* widget)
{
    if (!widget) return;
    
    QString loadingStyle = 
        "QWidget {"
        "    background-color: rgba(0, 0, 0, 0.1);"
        "    border: 2px dashed #BDBDBD;"
        "    border-radius: 8px;"
        "}";
    
    widget->setStyleSheet(loadingStyle);
}

void StyleUtils::applyErrorStyle(QWidget* widget, bool isError)
{
    if (!widget) return;
    
    if (isError) {
        QString errorStyle = 
            "QWidget {"
            "    border: 2px solid #F44336;"
            "    background-color: rgba(244, 67, 54, 0.1);"
            "}";
        widget->setStyleSheet(errorStyle);
    } else {
        // Remove error style by clearing stylesheet
        widget->setStyleSheet("");
    }
}

void StyleUtils::applySuccessStyle(QWidget* widget, bool isSuccess)
{
    if (!widget) return;
    
    if (isSuccess) {
        QString successStyle = 
            "QWidget {"
            "    border: 2px solid #4CAF50;"
            "    background-color: rgba(76, 175, 80, 0.1);"
            "}";
        widget->setStyleSheet(successStyle);
    } else {
        // Remove success style by clearing stylesheet
        widget->setStyleSheet("");
    }
}

StyleUtils::ColorPalette StyleUtils::getColorPalette(bool isDark)
{
    ColorPalette palette;
    
    if (isDark) {
        // Dark theme colors
        palette.primary = QColor("#64B5F6");
        palette.primaryDark = QColor("#1976D2");
        palette.secondary = QColor("#81C784");
        palette.background = QColor("#121212");
        palette.surface = QColor("#1E1E1E");
        palette.text = QColor("#FFFFFF");
        palette.textSecondary = QColor("#BDBDBD");
        palette.border = QColor("#424242");
        palette.error = QColor("#F44336");
        palette.success = QColor("#4CAF50");
        palette.warning = QColor("#FF9800");
    } else {
        // Light theme colors
        palette.primary = QColor("#2196F3");
        palette.primaryDark = QColor("#1976D2");
        palette.secondary = QColor("#4CAF50");
        palette.background = QColor("#FAFAFA");
        palette.surface = QColor("#FFFFFF");
        palette.text = QColor("#212121");
        palette.textSecondary = QColor("#666666");
        palette.border = QColor("#E0E0E0");
        palette.error = QColor("#D32F2F");
        palette.success = QColor("#388E3C");
        palette.warning = QColor("#F57C00");
    }
    
    return palette;
}

void StyleUtils::applyStandardSpacing(QWidget* widget, int spacing)
{
    if (!widget || !widget->layout()) return;
    
    QLayout* layout = widget->layout();
    layout->setSpacing(spacing);
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    
    // Apply spacing to specific layout types
    if (auto* vbox = qobject_cast<QVBoxLayout*>(layout)) {
        vbox->setSpacing(spacing);
    } else if (auto* hbox = qobject_cast<QHBoxLayout*>(layout)) {
        hbox->setSpacing(spacing);
    } else if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
        grid->setSpacing(spacing);
    } else if (auto* form = qobject_cast<QFormLayout*>(layout)) {
        form->setSpacing(spacing);
        form->setHorizontalSpacing(spacing);
        form->setVerticalSpacing(spacing);
    }
}