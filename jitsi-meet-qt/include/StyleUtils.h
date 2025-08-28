#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#include <QString>
#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsEffect>

class QGraphicsDropShadowEffect;
class QGraphicsOpacityEffect;

/**
 * @brief The StyleUtils class provides additional styling utilities and effects
 * 
 * This class offers advanced styling capabilities including:
 * - Animation effects for widgets
 * - Shadow and glow effects
 * - Hover and focus animations
 * - Custom property animations
 */
class StyleUtils
{
public:
    // Animation types
    enum class AnimationType {
        FadeIn,
        FadeOut,
        SlideIn,
        SlideOut,
        ScaleIn,
        ScaleOut,
        Bounce,
        Shake
    };
    
    // Shadow types
    enum class ShadowType {
        Subtle,
        Medium,
        Strong,
        Glow
    };
    
    // Animation utilities
    static QPropertyAnimation* createFadeAnimation(QWidget* widget, int duration = 300, 
                                                 double startOpacity = 0.0, double endOpacity = 1.0);
    static QPropertyAnimation* createSlideAnimation(QWidget* widget, int duration = 300,
                                                  const QPoint& startPos = QPoint(), const QPoint& endPos = QPoint());
    static QPropertyAnimation* createScaleAnimation(QWidget* widget, int duration = 300,
                                                  double startScale = 0.8, double endScale = 1.0);
    
    // Effect utilities
    static QGraphicsDropShadowEffect* createDropShadow(ShadowType type = ShadowType::Medium);
    static QGraphicsOpacityEffect* createOpacityEffect(double opacity = 1.0);
    
    // Hover effects
    static void addHoverScaleEffect(QWidget* widget, double scale = 1.05, int duration = 200);
    static void addHoverGlowEffect(QWidget* widget, const QColor& glowColor = QColor(33, 150, 243, 100));
    static void addHoverFadeEffect(QWidget* widget, double hoverOpacity = 0.8, int duration = 200);
    
    // Focus effects
    static void addFocusRingEffect(QWidget* widget, const QColor& ringColor = QColor(33, 150, 243));
    static void addFocusGlowEffect(QWidget* widget, const QColor& glowColor = QColor(33, 150, 243, 150));
    
    // Press effects
    static void addPressScaleEffect(QWidget* widget, double scale = 0.95, int duration = 100);
    static void addRippleEffect(QWidget* widget, const QColor& rippleColor = QColor(255, 255, 255, 100));
    
    // Loading animations
    static QPropertyAnimation* createSpinAnimation(QWidget* widget, int duration = 1000);
    static QPropertyAnimation* createPulseAnimation(QWidget* widget, int duration = 1000);
    static QPropertyAnimation* createBreatheAnimation(QWidget* widget, int duration = 2000);
    
    // Transition effects
    static void animateWidgetTransition(QWidget* fromWidget, QWidget* toWidget, 
                                      AnimationType type = AnimationType::FadeIn, int duration = 300);
    static void animateLayoutChange(QWidget* container, int duration = 300);
    
    // Utility functions
    static void setWidgetOpacity(QWidget* widget, double opacity);
    static void setWidgetScale(QWidget* widget, double scale);
    static void setWidgetRotation(QWidget* widget, double angle);
    
    // Style string generators
    static QString generateHoverStyle(const QString& baseStyle, const QString& hoverModifications);
    static QString generateFocusStyle(const QString& baseStyle, const QString& focusModifications);
    static QString generatePressedStyle(const QString& baseStyle, const QString& pressedModifications);
    
    // Color manipulation
    static QString lightenColor(const QString& color, int percentage);
    static QString darkenColor(const QString& color, int percentage);
    static QString adjustColorAlpha(const QString& color, int alpha);
    
    // Responsive design utilities
    static int getResponsiveSize(int baseSize, const QString& breakpoint = "medium");
    static QString getResponsiveMargin(int baseMargin, const QString& breakpoint = "medium");
    static QString getResponsivePadding(int basePadding, const QString& breakpoint = "medium");
    
private:
    StyleUtils() = default; // Static class
    
    // Helper methods
    static void setupHoverAnimation(QWidget* widget, QPropertyAnimation* animation);
    static void setupFocusAnimation(QWidget* widget, QPropertyAnimation* animation);
    static QPropertyAnimation* createPropertyAnimation(QWidget* widget, const QByteArray& property,
                                                     const QVariant& startValue, const QVariant& endValue,
                                                     int duration);
};

#endif // STYLEUTILS_H