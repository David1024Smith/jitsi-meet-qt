#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>
#include <QColor>
#include <QWidget>
#include <QIcon>

class QPushButton;
class QLabel;
class QLineEdit;

/**
 * @brief The StyleHelper class provides utility functions for consistent styling
 * 
 * This class offers helper methods to:
 * - Apply consistent styling to common widgets
 * - Generate themed colors and gradients
 * - Create styled icons and buttons
 * - Handle hover and focus effects
 */
class StyleHelper
{
public:
    // Color schemes
    struct ColorScheme {
        QColor primary;
        QColor primaryDark;
        QColor secondary;
        QColor background;
        QColor surface;
        QColor text;
        QColor textSecondary;
        QColor accent;
        QColor error;
        QColor success;
        QColor warning;
    };
    
    // Button styles
    enum class ButtonStyle {
        Primary,
        Secondary,
        Success,
        Warning,
        Error,
        Flat,
        Outlined
    };
    
    // Input styles
    enum class InputStyle {
        Default,
        Rounded,
        Outlined,
        Filled
    };

    // Static utility methods
    static ColorScheme getLightColorScheme();
    static ColorScheme getDarkColorScheme();
    static ColorScheme getModernColorScheme();
    
    // Widget styling
    static void styleButton(QPushButton* button, ButtonStyle style = ButtonStyle::Primary);
    static void styleLineEdit(QLineEdit* lineEdit, InputStyle style = InputStyle::Default);
    static void styleLabel(QLabel* label, const QString& role = "default");
    
    // Icon utilities
    static QIcon createThemedIcon(const QString& iconName, const QColor& color = QColor());
    static QIcon createButtonIcon(const QString& iconName, ButtonStyle style);
    
    // Color utilities
    static QString colorToString(const QColor& color);
    static QColor adjustColorBrightness(const QColor& color, int factor);
    static QColor blendColors(const QColor& color1, const QColor& color2, double ratio);
    
    // Gradient utilities
    static QString createLinearGradient(const QColor& startColor, const QColor& endColor, 
                                      const QString& direction = "to bottom");
    static QString createRadialGradient(const QColor& centerColor, const QColor& edgeColor);
    
    // Animation utilities
    static QString createTransition(const QString& property = "all", 
                                  const QString& duration = "0.2s",
                                  const QString& easing = "ease");
    
    // Shadow utilities
    static QString createBoxShadow(int offsetX, int offsetY, int blur, 
                                 const QColor& color, int spread = 0);
    
    // Border utilities
    static QString createBorder(int width, const QString& style, const QColor& color);
    static QString createBorderRadius(int radius);
    static QString createBorderRadius(int topLeft, int topRight, int bottomRight, int bottomLeft);
    
    // Layout utilities
    static void addHoverEffect(QWidget* widget, const QString& hoverStyle);
    static void addFocusEffect(QWidget* widget, const QString& focusStyle);
    static void addPressedEffect(QWidget* widget, const QString& pressedStyle);
    
    // Responsive utilities
    static int getScaledSize(int baseSize);
    static QString getScaledFont(int baseSize, const QString& weight = "normal");
    
private:
    StyleHelper() = default; // Static class
    
    static ColorScheme s_lightScheme;
    static ColorScheme s_darkScheme;
    static ColorScheme s_modernScheme;
    static bool s_schemesInitialized;
    
    static void initializeColorSchemes();
};

#endif // STYLEHELPER_H