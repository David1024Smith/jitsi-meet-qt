#include "StyleHelper.h"
#include "ThemeManager.h"
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

void StyleHelper::applyConferenceButtonStyle(QPushButton* button, const QString& iconPath, bool isToggleable)
{
    if (!button) return;

    // Set icon if provided
    if (!iconPath.isEmpty()) {
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(24, 24));
    }

    // Apply base styling
    button->setCheckable(isToggleable);
    button->setObjectName("ConferenceButton");
    
    // Apply theme-specific styling
    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    QString styleSheet;
    
    if (currentTheme == ThemeManager::DarkTheme) {
        styleSheet = QString(
            "QPushButton#ConferenceButton {"
            "    background-color: #2A2A2A;"
            "    border: 2px solid #424242;"
            "    border-radius: 24px;"
            "    color: #FFFFFF;"
            "    min-width: 48px;"
            "    min-height: 48px;"
            "    max-width: 48px;"
            "    max-height: 48px;"
            "}"
            "QPushButton#ConferenceButton:hover {"
            "    background-color: #333333;"
            "    border-color: #64B5F6;"
            "}"
            "QPushButton#ConferenceButton:checked {"
            "    background-color: rgba(100, 181, 246, 0.2);"
            "    border-color: #64B5F6;"
            "    color: #64B5F6;"
            "}"
        );
    } else if (currentTheme == ThemeManager::ModernTheme) {
        styleSheet = QString(
            "QPushButton#ConferenceButton {"
            "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
            "                                stop: 0 #FFFFFF, stop: 1 #F5F5F5);"
            "    border: 2px solid #E0E0E0;"
            "    border-radius: 28px;"
            "    color: #212121;"
            "    min-width: 56px;"
            "    min-height: 56px;"
            "    max-width: 56px;"
            "    max-height: 56px;"
            "}"
            "QPushButton#ConferenceButton:hover {"
            "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
            "                                stop: 0 #E3F2FD, stop: 1 #BBDEFB);"
            "    border-color: #2196F3;"
            "}"
            "QPushButton#ConferenceButton:checked {"
            "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
            "                                stop: 0 #E3F2FD, stop: 1 #BBDEFB);"
            "    border-color: #2196F3;"
            "    color: #2196F3;"
            "}"
        );
    } else {
        // Light theme
        styleSheet = QString(
            "QPushButton#ConferenceButton {"
            "    background-color: #F5F5F5;"
            "    border: 2px solid #E0E0E0;"
            "    border-radius: 24px;"
            "    color: #212121;"
            "    min-width: 48px;"
            "    min-height: 48px;"
            "    max-width: 48px;"
            "    max-height: 48px;"
            "}"
            "QPushButton#ConferenceButton:hover {"
            "    background-color: #E3F2FD;"
            "    border-color: #2196F3;"
            "}"
            "QPushButton#ConferenceButton:checked {"
            "    background-color: #E3F2FD;"
            "    border-color: #2196F3;"
            "    color: #2196F3;"
            "}"
        );
    }
    
    button->setStyleSheet(styleSheet);
}

void StyleHelper::applyVideoWidgetStyle(QWidget* widget, bool isMainVideo)
{
    if (!widget) return;

    widget->setObjectName(isMainVideo ? "MainVideoWidget" : "VideoWidget");
    
    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    QString styleSheet;
    
    if (isMainVideo) {
        if (currentTheme == ThemeManager::DarkTheme) {
            styleSheet = "QWidget#MainVideoWidget { background-color: #000000; border: 2px solid #424242; border-radius: 12px; }";
        } else if (currentTheme == ThemeManager::ModernTheme) {
            styleSheet = "QWidget#MainVideoWidget { background-color: #212121; border: 3px solid #424242; border-radius: 16px; }";
        } else {
            styleSheet = "QWidget#MainVideoWidget { background-color: #212121; border: 2px solid #424242; border-radius: 12px; }";
        }
    } else {
        if (currentTheme == ThemeManager::DarkTheme) {
            styleSheet = "QWidget#VideoWidget { background-color: #424242; border: 2px solid #616161; border-radius: 8px; }"
                        "QWidget#VideoWidget:hover { border-color: #64B5F6; }";
        } else if (currentTheme == ThemeManager::ModernTheme) {
            styleSheet = "QWidget#VideoWidget { background-color: #424242; border: 3px solid #616161; border-radius: 12px; }"
                        "QWidget#VideoWidget:hover { border-color: #2196F3; }";
        } else {
            styleSheet = "QWidget#VideoWidget { background-color: #424242; border: 2px solid #616161; border-radius: 8px; }"
                        "QWidget#VideoWidget:hover { border-color: #2196F3; }";
        }
    }
    
    widget->setStyleSheet(styleSheet);
}

void StyleHelper::applyPanelStyle(QWidget* widget)
{
    if (!widget) return;

    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    QString styleSheet;
    
    if (currentTheme == ThemeManager::DarkTheme) {
        styleSheet = "QWidget { background-color: #1E1E1E; border: 1px solid #424242; border-radius: 8px; }";
    } else if (currentTheme == ThemeManager::ModernTheme) {
        styleSheet = "QWidget { background-color: white; border: 1px solid #E0E0E0; border-radius: 12px; margin: 8px; }";
    } else {
        styleSheet = "QWidget { background-color: #FFFFFF; border: 1px solid #E0E0E0; border-radius: 8px; }";
    }
    
    widget->setStyleSheet(styleSheet);
}

void StyleHelper::applyCardStyle(QWidget* widget)
{
    if (!widget) return;

    applyPanelStyle(widget);
    applyShadowEffect(widget, QColor(0, 0, 0, 30), 8, 2);
}

void StyleHelper::applyStatusLabelStyle(QLabel* label, const QString& status)
{
    if (!label) return;

    QString styleSheet;
    
    if (status == "success") {
        styleSheet = "QLabel { color: #388E3C; font-weight: 500; background-color: #E8F5E8; "
                    "border: 1px solid #C8E6C9; border-radius: 4px; padding: 8px 12px; }";
    } else if (status == "error") {
        styleSheet = "QLabel { color: #D32F2F; font-weight: 500; background-color: #FFEBEE; "
                    "border: 1px solid #FFCDD2; border-radius: 4px; padding: 8px 12px; }";
    } else if (status == "warning") {
        styleSheet = "QLabel { color: #F57C00; font-weight: 500; background-color: #FFF3E0; "
                    "border: 1px solid #FFCC02; border-radius: 4px; padding: 8px 12px; }";
    } else if (status == "info") {
        styleSheet = "QLabel { color: #1976D2; font-weight: 500; background-color: #E3F2FD; "
                    "border: 1px solid #BBDEFB; border-radius: 4px; padding: 8px 12px; }";
    }
    
    label->setStyleSheet(styleSheet);
}

QColor StyleHelper::getThemeColor(const QString& colorName)
{
    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    
    if (colorName == "primary") {
        return QColor("#2196F3");
    } else if (colorName == "secondary") {
        return currentTheme == ThemeManager::DarkTheme ? QColor("#64B5F6") : QColor("#1976D2");
    } else if (colorName == "success") {
        return QColor("#4CAF50");
    } else if (colorName == "error") {
        return QColor("#F44336");
    } else if (colorName == "warning") {
        return QColor("#FF9800");
    } else if (colorName == "background") {
        if (currentTheme == ThemeManager::DarkTheme) {
            return QColor("#121212");
        } else {
            return QColor("#FAFAFA");
        }
    } else if (colorName == "surface") {
        if (currentTheme == ThemeManager::DarkTheme) {
            return QColor("#1E1E1E");
        } else {
            return QColor("#FFFFFF");
        }
    } else if (colorName == "text") {
        if (currentTheme == ThemeManager::DarkTheme) {
            return QColor("#FFFFFF");
        } else {
            return QColor("#212121");
        }
    }
    
    return QColor("#000000"); // Default fallback
}

QString StyleHelper::generateGradient(const QColor& startColor, const QColor& endColor, const QString& direction)
{
    QString gradientDirection;
    if (direction == "horizontal") {
        gradientDirection = "x1: 0, y1: 0, x2: 1, y2: 0";
    } else if (direction == "diagonal") {
        gradientDirection = "x1: 0, y1: 0, x2: 1, y2: 1";
    } else {
        gradientDirection = "x1: 0, y1: 0, x2: 0, y2: 1"; // vertical
    }
    
    return QString("qlineargradient(%1, stop: 0 %2, stop: 1 %3)")
           .arg(gradientDirection)
           .arg(startColor.name())
           .arg(endColor.name());
}

void StyleHelper::applyHoverEffect(QWidget* widget, const QColor& hoverColor)
{
    if (!widget) return;
    
    // This would typically be implemented with event filters or custom widgets
    // For now, we'll just set a property that can be used in stylesheets
    widget->setProperty("hoverColor", hoverColor);
}

void StyleHelper::applyShadowEffect(QWidget* widget, const QColor& shadowColor, int blurRadius, int offset)
{
    if (!widget) return;

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(widget);
    shadowEffect->setColor(shadowColor);
    shadowEffect->setBlurRadius(blurRadius);
    shadowEffect->setOffset(offset, offset);
    widget->setGraphicsEffect(shadowEffect);
}

void StyleHelper::applyRoundedCorners(QWidget* widget, int radius)
{
    if (!widget) return;
    
    QString styleSheet = QString("QWidget { border-radius: %1px; }").arg(radius);
    widget->setStyleSheet(widget->styleSheet() + styleSheet);
}

QString StyleHelper::getThemedIcon(const QString& iconName)
{
    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    
    // For dark theme, we might want to use different icon variants
    if (currentTheme == ThemeManager::DarkTheme && iconName == "dropdown") {
        return ":/icons/dropdown-dark.svg";
    }
    
    // Default to standard icon path
    return QString(":/icons/%1.svg").arg(iconName);
}