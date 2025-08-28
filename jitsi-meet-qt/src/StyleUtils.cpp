#include "StyleUtils.h"
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QWidget>
#include <QColor>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QApplication>
#include <QScreen>
#include <QDebug>

QPropertyAnimation* StyleUtils::createFadeAnimation(QWidget* widget, int duration, double startOpacity, double endOpacity)
{
    if (!widget) return nullptr;
    
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(startOpacity);
    animation->setEndValue(endOpacity);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    return animation;
}

QPropertyAnimation* StyleUtils::createSlideAnimation(QWidget* widget, int duration, const QPoint& startPos, const QPoint& endPos)
{
    if (!widget) return nullptr;
    
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "pos", widget);
    animation->setDuration(duration);
    animation->setStartValue(startPos.isNull() ? widget->pos() : startPos);
    animation->setEndValue(endPos.isNull() ? widget->pos() : endPos);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    return animation;
}

QPropertyAnimation* StyleUtils::createScaleAnimation(QWidget* widget, int duration, double startScale, double endScale)
{
    if (!widget) return nullptr;
    
    // Note: Qt doesn't have built-in scale property for widgets
    // This is a simplified implementation that would need custom property handling
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "geometry", widget);
    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::OutBack);
    
    QRect startGeometry = widget->geometry();
    QRect endGeometry = widget->geometry();
    
    // Calculate scaled geometry
    int startWidth = static_cast<int>(startGeometry.width() * startScale);
    int startHeight = static_cast<int>(startGeometry.height() * startScale);
    int endWidth = static_cast<int>(endGeometry.width() * endScale);
    int endHeight = static_cast<int>(endGeometry.height() * endScale);
    
    startGeometry.setSize(QSize(startWidth, startHeight));
    endGeometry.setSize(QSize(endWidth, endHeight));
    
    animation->setStartValue(startGeometry);
    animation->setEndValue(endGeometry);
    
    return animation;
}

QGraphicsDropShadowEffect* StyleUtils::createDropShadow(ShadowType type)
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    
    switch (type) {
        case ShadowType::Subtle:
            shadow->setBlurRadius(8);
            shadow->setOffset(0, 2);
            shadow->setColor(QColor(0, 0, 0, 25));
            break;
            
        case ShadowType::Medium:
            shadow->setBlurRadius(12);
            shadow->setOffset(0, 4);
            shadow->setColor(QColor(0, 0, 0, 40));
            break;
            
        case ShadowType::Strong:
            shadow->setBlurRadius(20);
            shadow->setOffset(0, 8);
            shadow->setColor(QColor(0, 0, 0, 60));
            break;
            
        case ShadowType::Glow:
            shadow->setBlurRadius(15);
            shadow->setOffset(0, 0);
            shadow->setColor(QColor(33, 150, 243, 100));
            break;
    }
    
    return shadow;
}

QGraphicsOpacityEffect* StyleUtils::createOpacityEffect(double opacity)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    effect->setOpacity(opacity);
    return effect;
}

void StyleUtils::addHoverScaleEffect(QWidget* widget, double scale, int duration)
{
    if (!widget) return;
    
    // Store original geometry
    widget->setProperty("originalGeometry", widget->geometry());
    
    // Create scale up animation
    QPropertyAnimation* scaleUpAnimation = new QPropertyAnimation(widget, "geometry", widget);
    scaleUpAnimation->setDuration(duration);
    scaleUpAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Create scale down animation
    QPropertyAnimation* scaleDownAnimation = new QPropertyAnimation(widget, "geometry", widget);
    scaleDownAnimation->setDuration(duration);
    scaleDownAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Connect hover events
    widget->installEventFilter(new QObject(widget)); // Simplified event filter setup
    
    // Note: In a real implementation, you'd need proper event filter handling
}

void StyleUtils::addHoverGlowEffect(QWidget* widget, const QColor& glowColor)
{
    if (!widget) return;
    
    QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(widget);
    glowEffect->setBlurRadius(15);
    glowEffect->setOffset(0, 0);
    glowEffect->setColor(glowColor);
    
    widget->setGraphicsEffect(glowEffect);
}

void StyleUtils::addHoverFadeEffect(QWidget* widget, double hoverOpacity, int duration)
{
    if (!widget) return;
    
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(opacityEffect);
    
    QPropertyAnimation* fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", widget);
    fadeAnimation->setDuration(duration);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Store animation for later use
    widget->setProperty("fadeAnimation", QVariant::fromValue(fadeAnimation));
}

void StyleUtils::addFocusRingEffect(QWidget* widget, const QColor& ringColor)
{
    if (!widget) return;
    
    // Create focus ring effect using stylesheet
    QString focusStyle = QString(
        "QWidget:focus {"
        "    border: 2px solid %1;"
        "    border-radius: 4px;"
        "}"
    ).arg(ringColor.name());
    
    QString currentStyle = widget->styleSheet();
    widget->setStyleSheet(currentStyle + focusStyle);
}

void StyleUtils::addFocusGlowEffect(QWidget* widget, const QColor& glowColor)
{
    if (!widget) return;
    
    QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(widget);
    glowEffect->setBlurRadius(10);
    glowEffect->setOffset(0, 0);
    glowEffect->setColor(glowColor);
    
    // Apply effect on focus (simplified implementation)
    widget->setGraphicsEffect(glowEffect);
}

void StyleUtils::addPressScaleEffect(QWidget* widget, double scale, int duration)
{
    if (!widget) return;
    
    QPropertyAnimation* pressAnimation = new QPropertyAnimation(widget, "geometry", widget);
    pressAnimation->setDuration(duration);
    pressAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Store for later use
    widget->setProperty("pressAnimation", QVariant::fromValue(pressAnimation));
}

void StyleUtils::addRippleEffect(QWidget* widget, const QColor& rippleColor)
{
    if (!widget) return;
    
    // Ripple effect would require custom painting implementation
    // This is a placeholder for the concept
    Q_UNUSED(rippleColor)
    
    qDebug() << "Ripple effect added to widget:" << widget->objectName();
}

QPropertyAnimation* StyleUtils::createSpinAnimation(QWidget* widget, int duration)
{
    if (!widget) return nullptr;
    
    // Note: Qt widgets don't have built-in rotation property
    // This would require custom implementation with QGraphicsView or custom painting
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "rotation", widget);
    animation->setDuration(duration);
    animation->setStartValue(0);
    animation->setEndValue(360);
    animation->setLoopCount(-1); // Infinite loop
    animation->setEasingCurve(QEasingCurve::Linear);
    
    return animation;
}

QPropertyAnimation* StyleUtils::createPulseAnimation(QWidget* widget, int duration)
{
    if (!widget) return nullptr;
    
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(duration);
    animation->setStartValue(0.5);
    animation->setEndValue(1.0);
    animation->setLoopCount(-1);
    animation->setEasingCurve(QEasingCurve::InOutSine);
    
    return animation;
}

QPropertyAnimation* StyleUtils::createBreatheAnimation(QWidget* widget, int duration)
{
    if (!widget) return nullptr;
    
    QSequentialAnimationGroup* breatheGroup = new QSequentialAnimationGroup(widget);
    
    // Inhale
    QPropertyAnimation* inhale = createScaleAnimation(widget, duration / 2, 1.0, 1.05);
    breatheGroup->addAnimation(inhale);
    
    // Exhale
    QPropertyAnimation* exhale = createScaleAnimation(widget, duration / 2, 1.05, 1.0);
    breatheGroup->addAnimation(exhale);
    
    breatheGroup->setLoopCount(-1);
    
    return nullptr; // Return the group animation (would need different return type)
}

void StyleUtils::animateWidgetTransition(QWidget* fromWidget, QWidget* toWidget, AnimationType type, int duration)
{
    if (!fromWidget || !toWidget) return;
    
    QParallelAnimationGroup* transitionGroup = new QParallelAnimationGroup();
    
    switch (type) {
        case AnimationType::FadeIn:
            {
                QPropertyAnimation* fadeOut = createFadeAnimation(fromWidget, duration, 1.0, 0.0);
                QPropertyAnimation* fadeIn = createFadeAnimation(toWidget, duration, 0.0, 1.0);
                transitionGroup->addAnimation(fadeOut);
                transitionGroup->addAnimation(fadeIn);
            }
            break;
            
        case AnimationType::SlideIn:
            {
                QPoint fromPos = fromWidget->pos();
                QPoint toPos = toWidget->pos();
                QPoint offScreenLeft = QPoint(-toWidget->width(), toPos.y());
                
                QPropertyAnimation* slideOut = createSlideAnimation(fromWidget, duration, fromPos, QPoint(fromWidget->width(), fromPos.y()));
                QPropertyAnimation* slideIn = createSlideAnimation(toWidget, duration, offScreenLeft, toPos);
                transitionGroup->addAnimation(slideOut);
                transitionGroup->addAnimation(slideIn);
            }
            break;
            
        case AnimationType::ScaleIn:
            {
                QPropertyAnimation* scaleOut = createScaleAnimation(fromWidget, duration, 1.0, 0.8);
                QPropertyAnimation* fadeOut = createFadeAnimation(fromWidget, duration, 1.0, 0.0);
                QPropertyAnimation* scaleIn = createScaleAnimation(toWidget, duration, 0.8, 1.0);
                QPropertyAnimation* fadeIn = createFadeAnimation(toWidget, duration, 0.0, 1.0);
                
                transitionGroup->addAnimation(scaleOut);
                transitionGroup->addAnimation(fadeOut);
                transitionGroup->addAnimation(scaleIn);
                transitionGroup->addAnimation(fadeIn);
            }
            break;
            
        default:
            // Default to fade transition
            animateWidgetTransition(fromWidget, toWidget, AnimationType::FadeIn, duration);
            return;
    }
    
    // Connect completion signal
    QObject::connect(transitionGroup, &QAbstractAnimation::finished, [fromWidget, toWidget]() {
        fromWidget->hide();
        toWidget->show();
    });
    
    transitionGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void StyleUtils::animateLayoutChange(QWidget* container, int duration)
{
    if (!container) return;
    
    // Store current positions of child widgets
    QList<QWidget*> children = container->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QHash<QWidget*, QRect> oldGeometries;
    
    for (QWidget* child : children) {
        oldGeometries[child] = child->geometry();
    }
    
    // Force layout update
    container->updateGeometry();
    QApplication::processEvents();
    
    // Animate to new positions
    QParallelAnimationGroup* layoutAnimation = new QParallelAnimationGroup(container);
    
    for (QWidget* child : children) {
        QRect oldGeometry = oldGeometries[child];
        QRect newGeometry = child->geometry();
        
        if (oldGeometry != newGeometry) {
            child->setGeometry(oldGeometry);
            QPropertyAnimation* moveAnimation = createSlideAnimation(child, duration, oldGeometry.topLeft(), newGeometry.topLeft());
            layoutAnimation->addAnimation(moveAnimation);
        }
    }
    
    layoutAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StyleUtils::setWidgetOpacity(QWidget* widget, double opacity)
{
    if (!widget) return;
    
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    effect->setOpacity(opacity);
}

void StyleUtils::setWidgetScale(QWidget* widget, double scale)
{
    if (!widget) return;
    
    // Simplified scale implementation using geometry
    QRect geometry = widget->geometry();
    QSize newSize = geometry.size() * scale;
    QPoint center = geometry.center();
    
    geometry.setSize(newSize);
    geometry.moveCenter(center);
    widget->setGeometry(geometry);
}

void StyleUtils::setWidgetRotation(QWidget* widget, double angle)
{
    if (!widget) return;
    
    // Qt widgets don't support rotation directly
    // This would require QGraphicsView or custom painting
    Q_UNUSED(angle)
    qDebug() << "Widget rotation not implemented for standard widgets";
}

QString StyleUtils::generateHoverStyle(const QString& baseStyle, const QString& hoverModifications)
{
    return baseStyle + "\n" + "QWidget:hover { " + hoverModifications + " }";
}

QString StyleUtils::generateFocusStyle(const QString& baseStyle, const QString& focusModifications)
{
    return baseStyle + "\n" + "QWidget:focus { " + focusModifications + " }";
}

QString StyleUtils::generatePressedStyle(const QString& baseStyle, const QString& pressedModifications)
{
    return baseStyle + "\n" + "QWidget:pressed { " + pressedModifications + " }";
}

QString StyleUtils::lightenColor(const QString& color, int percentage)
{
    QColor qcolor(color);
    int factor = 255 * percentage / 100;
    
    int r = qMin(255, qcolor.red() + factor);
    int g = qMin(255, qcolor.green() + factor);
    int b = qMin(255, qcolor.blue() + factor);
    
    return QColor(r, g, b, qcolor.alpha()).name();
}

QString StyleUtils::darkenColor(const QString& color, int percentage)
{
    QColor qcolor(color);
    int factor = 255 * percentage / 100;
    
    int r = qMax(0, qcolor.red() - factor);
    int g = qMax(0, qcolor.green() - factor);
    int b = qMax(0, qcolor.blue() - factor);
    
    return QColor(r, g, b, qcolor.alpha()).name();
}

QString StyleUtils::adjustColorAlpha(const QString& color, int alpha)
{
    QColor qcolor(color);
    qcolor.setAlpha(alpha);
    
    return QString("rgba(%1, %2, %3, %4)")
           .arg(qcolor.red())
           .arg(qcolor.green())
           .arg(qcolor.blue())
           .arg(alpha);
}

int StyleUtils::getResponsiveSize(int baseSize, const QString& breakpoint)
{
    // Simple responsive scaling based on screen size
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) return baseSize;
    
    QSize screenSize = screen->size();
    double scaleFactor = 1.0;
    
    if (breakpoint == "small" && screenSize.width() < 768) {
        scaleFactor = 0.8;
    } else if (breakpoint == "medium" && screenSize.width() < 1024) {
        scaleFactor = 0.9;
    } else if (breakpoint == "large" && screenSize.width() >= 1024) {
        scaleFactor = 1.1;
    }
    
    return static_cast<int>(baseSize * scaleFactor);
}

QString StyleUtils::getResponsiveMargin(int baseMargin, const QString& breakpoint)
{
    int responsiveMargin = getResponsiveSize(baseMargin, breakpoint);
    return QString("margin: %1px;").arg(responsiveMargin);
}

QString StyleUtils::getResponsivePadding(int basePadding, const QString& breakpoint)
{
    int responsivePadding = getResponsiveSize(basePadding, breakpoint);
    return QString("padding: %1px;").arg(responsivePadding);
}