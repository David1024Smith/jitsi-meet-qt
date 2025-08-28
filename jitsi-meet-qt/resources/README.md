# Jitsi Meet Qt Resources and Theming System

This document describes the comprehensive resource and theming system implemented for the Jitsi Meet Qt application.

## Overview

The resource system provides:
- **Comprehensive theming** with light, dark, and modern themes
- **Scalable vector icons** for all UI elements
- **Responsive styling** that adapts to different screen sizes
- **Animation and effect utilities** for enhanced user experience
- **Consistent design language** across all components

## Directory Structure

```
resources/
├── icons/           # SVG icons for all UI elements
├── images/          # Background images and graphics
├── styles/          # QSS stylesheets for different themes
└── resources.qrc    # Qt resource file
```

## Theming System

### Available Themes

1. **Light Theme** (`default.qss`)
   - Clean, bright interface
   - Material Design inspired colors
   - High contrast for accessibility

2. **Dark Theme** (`dark.qss`)
   - Dark background with light text
   - Reduced eye strain in low-light conditions
   - Modern dark UI patterns

3. **Modern Theme** (`modern.qss`)
   - Gradient-based design
   - Enhanced visual effects
   - Glass-morphism elements

### Theme Management

The `ThemeManager` class provides:

```cpp
// Set theme
themeManager->setTheme(ThemeManager::Theme::Dark);

// Get current theme
ThemeManager::Theme current = themeManager->currentTheme();

// Enable system theme detection
themeManager->enableSystemThemeDetection(true);

// Get themed icons
QString iconPath = themeManager->getThemedIcon("microphone");
```

### Style Helper Utilities

The `StyleHelper` class provides consistent styling:

```cpp
// Style buttons with different variants
StyleHelper::styleButton(button, StyleHelper::ButtonStyle::Primary);
StyleHelper::styleButton(button, StyleHelper::ButtonStyle::Secondary);
StyleHelper::styleButton(button, StyleHelper::ButtonStyle::Success);

// Style input fields
StyleHelper::styleLineEdit(lineEdit, StyleHelper::InputStyle::Rounded);

// Style labels with semantic roles
StyleHelper::styleLabel(label, "title");
StyleHelper::styleLabel(errorLabel, "error");
```

## Icon System

### Icon Categories

1. **Application Icons**
   - `app.svg` - Main application icon
   - `logo.svg` - Jitsi Meet logo

2. **Navigation Icons**
   - `settings.svg` - Settings/preferences
   - `about.svg` - About/information
   - `back.svg` - Back/return navigation
   - `recent.svg` - Recent items

3. **Conference Control Icons**
   - `microphone.svg` / `microphone-off.svg` - Audio controls
   - `camera.svg` / `camera-off.svg` - Video controls
   - `screen-share.svg` - Screen sharing
   - `chat.svg` - Chat/messaging
   - `participants.svg` - Participant list
   - `phone-hangup.svg` - Leave meeting

4. **UI Control Icons**
   - `fullscreen.svg` / `fullscreen-exit.svg` - Fullscreen toggle
   - `dropdown.svg` / `dropdown-dark.svg` - Dropdown indicators
   - `refresh.svg` - Refresh/reload
   - `close.svg` - Close/dismiss

5. **Status Icons**
   - `warning.svg` - Warning messages
   - `error.svg` - Error states
   - `success.svg` - Success confirmations
   - `loading.svg` - Loading indicators

### Icon Usage

Icons are referenced using the Qt resource system:

```cpp
// Load icon from resources
QIcon icon(":/icons/microphone.svg");

// Set button icon
button->setIcon(QIcon(":/icons/camera.svg"));

// Get themed icon (automatically selects dark variant if needed)
QString iconPath = themeManager->getThemedIcon("microphone");
```

## Animation and Effects

The `StyleUtils` class provides animation utilities:

```cpp
// Fade animations
QPropertyAnimation* fadeIn = StyleUtils::createFadeAnimation(widget, 300, 0.0, 1.0);

// Scale animations
QPropertyAnimation* scaleUp = StyleUtils::createScaleAnimation(widget, 200, 0.8, 1.0);

// Hover effects
StyleUtils::addHoverScaleEffect(button, 1.05, 200);
StyleUtils::addHoverGlowEffect(widget, QColor(33, 150, 243, 100));

// Loading animations
QPropertyAnimation* spin = StyleUtils::createSpinAnimation(loadingIcon, 1000);
QPropertyAnimation* pulse = StyleUtils::createPulseAnimation(indicator, 2000);
```

## Responsive Design

The system includes responsive utilities:

```cpp
// Get scaled sizes based on screen DPI
int scaledSize = StyleHelper::getScaledSize(16);

// Responsive breakpoints
int responsiveMargin = StyleUtils::getResponsiveSize(12, "medium");
QString responsivePadding = StyleUtils::getResponsivePadding(8, "large");
```

## Color System

### Color Schemes

Each theme defines a comprehensive color scheme:

```cpp
StyleHelper::ColorScheme scheme = StyleHelper::getLightColorScheme();
// Access colors: scheme.primary, scheme.background, scheme.text, etc.

// Color utilities
QColor lighter = StyleHelper::adjustColorBrightness(color, 20);
QColor blended = StyleHelper::blendColors(color1, color2, 0.5);
QString colorString = StyleHelper::colorToString(color);
```

### Semantic Colors

- **Primary**: Main brand color (#2196F3)
- **Secondary**: Accent color (#FF9800)
- **Success**: Positive actions (#4CAF50)
- **Warning**: Caution states (#FF9800)
- **Error**: Error states (#F44336)
- **Background**: Main background
- **Surface**: Card/panel backgrounds
- **Text**: Primary text color
- **Text Secondary**: Muted text color

## Component Styling

### Buttons

```css
/* Primary Button */
QPushButton {
    background-color: #2196F3;
    color: white;
    border: none;
    border-radius: 8px;
    padding: 12px 24px;
    font-weight: 500;
}

/* Conference Control Button */
QPushButton#MuteAudioButton {
    background-color: #F5F5F5;
    border: 2px solid #E0E0E0;
    border-radius: 24px;
    min-width: 48px;
    min-height: 48px;
}
```

### Input Fields

```css
/* Default Input */
QLineEdit {
    background-color: white;
    border: 2px solid #E0E0E0;
    border-radius: 8px;
    padding: 12px 16px;
    font-size: 11pt;
}

/* Rounded Input */
QLineEdit.rounded {
    border-radius: 20px;
    padding: 12px 20px;
}
```

### Panels and Containers

```css
/* Chat Panel */
QWidget#ChatPanel {
    background-color: white;
    border-left: 1px solid #E0E0E0;
    min-width: 300px;
    max-width: 400px;
}

/* Control Panel */
QWidget#ControlPanel {
    background-color: #FFFFFF;
    border-top: 1px solid #E0E0E0;
    padding: 12px;
}
```

## Usage Examples

### Basic Theme Setup

```cpp
#include "ThemeManager.h"

// Create theme manager
ThemeManager* themeManager = new ThemeManager(this);

// Set initial theme
themeManager->setTheme(ThemeManager::Theme::Light);

// Connect to theme changes
connect(themeManager, &ThemeManager::themeChanged, [](ThemeManager::Theme theme) {
    qDebug() << "Theme changed to:" << ThemeManager::themeToString(theme);
});
```

### Styling Widgets

```cpp
#include "StyleHelper.h"

// Style a join button
QPushButton* joinButton = new QPushButton("Join Meeting");
joinButton->setIcon(QIcon(":/icons/join.svg"));
StyleHelper::styleButton(joinButton, StyleHelper::ButtonStyle::Success);

// Style an input field
QLineEdit* urlInput = new QLineEdit();
urlInput->setPlaceholderText("Enter meeting URL...");
StyleHelper::styleLineEdit(urlInput, StyleHelper::InputStyle::Rounded);

// Style labels
QLabel* titleLabel = new QLabel("Welcome to Jitsi Meet");
StyleHelper::styleLabel(titleLabel, "title");
```

### Adding Animations

```cpp
#include "StyleUtils.h"

// Add hover effect to button
StyleUtils::addHoverScaleEffect(button, 1.05, 200);

// Create fade transition between windows
StyleUtils::animateWidgetTransition(welcomeWindow, conferenceWindow, 
                                  StyleUtils::AnimationType::FadeIn, 300);

// Add loading animation
QPropertyAnimation* loadingAnim = StyleUtils::createPulseAnimation(loadingLabel, 1000);
loadingAnim->start();
```

## Building and Testing

### Build Theme Test

```bash
# Windows
build_theme_test.bat

# Or manually with CMake
mkdir build_theme_test
cd build_theme_test
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\mingw_64 -DBUILD_THEME_TEST=ON ..
mingw32-make
test_theme_resources.exe
```

### Resource Verification

The theme test application verifies:
- ✅ All icons load correctly
- ✅ Stylesheets apply properly
- ✅ Theme switching works
- ✅ Animations function correctly
- ✅ Responsive scaling works

## Best Practices

### Icon Guidelines

1. **Use SVG format** for scalability
2. **Provide dark variants** for dark theme compatibility
3. **Keep consistent sizing** (24x24 for UI icons, 64x64 for app icon)
4. **Use semantic naming** (e.g., `microphone-off.svg`)

### Styling Guidelines

1. **Use semantic classes** and object names
2. **Leverage the color scheme** from StyleHelper
3. **Apply consistent spacing** (8px grid system)
4. **Use border-radius** for modern appearance
5. **Provide hover and focus states**

### Animation Guidelines

1. **Keep animations subtle** (200-300ms duration)
2. **Use easing curves** for natural motion
3. **Provide feedback** for user interactions
4. **Avoid excessive animations** that distract

### Performance Considerations

1. **Cache stylesheets** to avoid repeated file I/O
2. **Use resource system** for efficient bundling
3. **Optimize SVG icons** for smaller file sizes
4. **Limit concurrent animations** to maintain smooth performance

## Troubleshooting

### Common Issues

1. **Icons not loading**
   - Check resource file paths in `resources.qrc`
   - Verify SVG file syntax
   - Ensure resource file is included in CMakeLists.txt

2. **Styles not applying**
   - Check QSS syntax
   - Verify object names and selectors
   - Ensure stylesheet is loaded correctly

3. **Animations not working**
   - Check Qt version compatibility
   - Verify property names for animations
   - Ensure widgets have proper parent relationships

### Debug Commands

```cpp
// Test resource loading
QFile styleFile(":/styles/default.qss");
if (!styleFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to load stylesheet";
}

// Test icon loading
QIcon icon(":/icons/app.svg");
if (icon.isNull()) {
    qWarning() << "Failed to load icon";
}

// Debug theme changes
connect(themeManager, &ThemeManager::themeChanged, [](ThemeManager::Theme theme) {
    qDebug() << "Theme changed to:" << ThemeManager::themeToString(theme);
});
```

## Future Enhancements

### Planned Features

1. **Custom theme creation** - Allow users to create custom themes
2. **High contrast mode** - Accessibility-focused theme
3. **Seasonal themes** - Holiday and seasonal variations
4. **Animation preferences** - User control over animation intensity
5. **Icon packs** - Alternative icon sets
6. **CSS-like animations** - More advanced animation system

### Contributing

When adding new resources:

1. **Follow naming conventions**
2. **Update resources.qrc**
3. **Add to appropriate style files**
4. **Test with all themes**
5. **Update documentation**

This comprehensive resource and theming system provides a solid foundation for a modern, accessible, and visually appealing Jitsi Meet Qt application.