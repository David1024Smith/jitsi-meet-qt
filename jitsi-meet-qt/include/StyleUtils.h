#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#include <QString>
#include <QColor>
#include <QWidget>
#include <QPalette>

/**
 * @brief Utility class for styling operations
 * 
 * Provides helper functions for common styling operations,
 * color manipulation, and dynamic style generation.
 */
class StyleUtils
{
public:
    /**
     * @brief Apply rounded corners to a widget
     * @param widget Target widget
     * @param radius Corner radius in pixels
     */
    static void applyRoundedCorners(QWidget* widget, int radius = 8);

    /**
     * @brief Apply shadow effect to a widget
     * @param widget Target widget
     * @param color Shadow color
     * @param blurRadius Blur radius
     * @param offset Shadow offset
     */
    static void applyShadow(QWidget* widget, const QColor& color = QColor(0, 0, 0, 50), 
                           int blurRadius = 10, const QPoint& offset = QPoint(0, 2));

    /**
     * @brief Generate button style with custom colors
     * @param backgroundColor Button background color
     * @param textColor Button text color
     * @param hoverColor Hover state color
     * @param pressedColor Pressed state color
     * @return Generated QSS style string
     */
    static QString generateButtonStyle(const QColor& backgroundColor,
                                     const QColor& textColor,
                                     const QColor& hoverColor,
                                     const QColor& pressedColor);

    /**
     * @brief Generate input field style
     * @param backgroundColor Background color
     * @param borderColor Border color
     * @param focusColor Focus border color
     * @return Generated QSS style string
     */
    static QString generateInputStyle(const QColor& backgroundColor,
                                    const QColor& borderColor,
                                    const QColor& focusColor);

    /**
     * @brief Lighten a color by a percentage
     * @param color Original color
     * @param percentage Percentage to lighten (0-100)
     * @return Lightened color
     */
    static QColor lightenColor(const QColor& color, int percentage);

    /**
     * @brief Darken a color by a percentage
     * @param color Original color
     * @param percentage Percentage to darken (0-100)
     * @return Darkened color
     */
    static QColor darkenColor(const QColor& color, int percentage);

    /**
     * @brief Create color with alpha transparency
     * @param color Base color
     * @param alpha Alpha value (0-255)
     * @return Color with alpha
     */
    static QColor withAlpha(const QColor& color, int alpha);

    /**
     * @brief Get contrasting text color for background
     * @param backgroundColor Background color
     * @return Black or white text color for best contrast
     */
    static QColor getContrastingTextColor(const QColor& backgroundColor);

    /**
     * @brief Check if color is considered "dark"
     * @param color Color to check
     * @return True if color is dark
     */
    static bool isDarkColor(const QColor& color);

    /**
     * @brief Apply loading animation style to widget
     * @param widget Target widget
     */
    static void applyLoadingStyle(QWidget* widget);

    /**
     * @brief Apply error state style to widget
     * @param widget Target widget
     * @param isError True to apply error style, false to remove
     */
    static void applyErrorStyle(QWidget* widget, bool isError = true);

    /**
     * @brief Apply success state style to widget
     * @param widget Target widget
     * @param isSuccess True to apply success style, false to remove
     */
    static void applySuccessStyle(QWidget* widget, bool isSuccess = true);

    /**
     * @brief Get standard color palette for current theme
     * @param isDark True for dark theme colors
     * @return Color palette struct
     */
    struct ColorPalette {
        QColor primary;
        QColor primaryDark;
        QColor secondary;
        QColor background;
        QColor surface;
        QColor text;
        QColor textSecondary;
        QColor border;
        QColor error;
        QColor success;
        QColor warning;
    };

    static ColorPalette getColorPalette(bool isDark = false);

    /**
     * @brief Apply consistent spacing to layout
     * @param widget Widget containing layout
     * @param spacing Spacing value
     */
    static void applyStandardSpacing(QWidget* widget, int spacing = 16);

private:
    StyleUtils() = delete; // Static class
};

#endif // STYLEUTILS_H