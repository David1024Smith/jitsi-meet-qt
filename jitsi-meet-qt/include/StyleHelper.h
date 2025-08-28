#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>
#include <QColor>
#include <QWidget>
#include <QPushButton>
#include <QLabel>

/**
 * @brief Helper class for applying consistent styling across the application
 */
class StyleHelper
{
public:
    /**
     * @brief Apply conference control button styling
     * @param button Button to style
     * @param iconPath Path to the button icon
     * @param isToggleable Whether the button can be toggled
     */
    static void applyConferenceButtonStyle(QPushButton* button, const QString& iconPath, bool isToggleable = false);

    /**
     * @brief Apply video widget styling
     * @param widget Video widget to style
     * @param isMainVideo Whether this is the main video widget
     */
    static void applyVideoWidgetStyle(QWidget* widget, bool isMainVideo = false);

    /**
     * @brief Apply panel styling (chat, participants, etc.)
     * @param widget Panel widget to style
     */
    static void applyPanelStyle(QWidget* widget);

    /**
     * @brief Apply card styling for list items
     * @param widget Widget to style as a card
     */
    static void applyCardStyle(QWidget* widget);

    /**
     * @brief Apply status label styling
     * @param label Label to style
     * @param status Status type (success, error, warning, info)
     */
    static void applyStatusLabelStyle(QLabel* label, const QString& status);

    /**
     * @brief Get theme-appropriate color
     * @param colorName Color name (primary, secondary, success, error, etc.)
     * @return QColor for the current theme
     */
    static QColor getThemeColor(const QString& colorName);

    /**
     * @brief Generate gradient stylesheet
     * @param startColor Starting color
     * @param endColor Ending color
     * @param direction Gradient direction (horizontal, vertical, diagonal)
     * @return CSS gradient string
     */
    static QString generateGradient(const QColor& startColor, const QColor& endColor, const QString& direction = "vertical");

    /**
     * @brief Apply hover effect to widget
     * @param widget Widget to apply hover effect to
     * @param hoverColor Color to use on hover
     */
    static void applyHoverEffect(QWidget* widget, const QColor& hoverColor);

    /**
     * @brief Apply shadow effect
     * @param widget Widget to apply shadow to
     * @param shadowColor Shadow color
     * @param blurRadius Blur radius
     * @param offset Shadow offset
     */
    static void applyShadowEffect(QWidget* widget, const QColor& shadowColor, int blurRadius = 10, int offset = 2);

    /**
     * @brief Apply rounded corners to widget
     * @param widget Widget to apply rounded corners to
     * @param radius Corner radius
     */
    static void applyRoundedCorners(QWidget* widget, int radius = 8);

    /**
     * @brief Get icon path for current theme
     * @param iconName Base icon name
     * @return Full path to themed icon
     */
    static QString getThemedIcon(const QString& iconName);

private:
    StyleHelper() = default;
};

#endif // STYLEHELPER_H