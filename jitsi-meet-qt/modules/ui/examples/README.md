# UI Module Examples

This directory contains example code and demonstrations of how to use the UI module components.

## Examples Overview

```
examples/
├── basic/                  # Basic usage examples
│   ├── simple_window.cpp  # Simple window with UI module
│   ├── theme_switching.cpp # Theme switching example
│   └── custom_widgets.cpp # Custom widget usage
├── advanced/              # Advanced usage examples
│   ├── custom_theme.cpp   # Creating custom themes
│   ├── layout_manager.cpp # Advanced layout management
│   └── responsive_ui.cpp  # Responsive UI design
├── integration/           # Integration examples
│   ├── with_audio.cpp     # UI + Audio module integration
│   ├── with_network.cpp   # UI + Network module integration
│   └── full_app.cpp       # Complete application example
├── widgets/               # Widget-specific examples
│   ├── custom_button_demo.cpp
│   ├── status_bar_demo.cpp
│   └── toolbar_demo.cpp
├── layouts/               # Layout examples
│   ├── main_layout_demo.cpp
│   ├── conference_layout_demo.cpp
│   └── settings_layout_demo.cpp
└── CMakeLists.txt         # Build configuration
```

## Building Examples

### Using CMake
```bash
cd examples
mkdir build && cd build
cmake ..
make
```

### Individual Examples
```bash
# Build specific example
cd examples/basic
g++ -o simple_window simple_window.cpp `pkg-config --cflags --libs Qt5Widgets`
```

## Example Descriptions

### Basic Examples

#### simple_window.cpp
Demonstrates basic UI module initialization and window creation:
```cpp
#include "UIModule.h"
#include "UIManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Initialize UI module
    auto uiModule = UIModule::instance();
    uiModule->initialize();
    
    // Create main window
    auto uiManager = uiModule->uiManager();
    // ... window setup code
    
    return app.exec();
}
```

#### theme_switching.cpp
Shows how to implement theme switching:
```cpp
// Switch to dark theme
uiManager->setTheme("dark");

// Apply custom theme
auto customTheme = themeFactory->createTheme("custom");
uiManager->applyTheme(customTheme);
```

#### custom_widgets.cpp
Demonstrates creating and using custom widgets:
```cpp
#include "widgets/CustomButton.h"
#include "widgets/StatusBar.h"

// Create custom button
auto button = new CustomButton("Click Me");
button->setButtonStyle(CustomButton::PrimaryStyle);

// Create status bar
auto statusBar = new StatusBar();
statusBar->showMessage("Ready", StatusBar::InfoStatus);
```

### Advanced Examples

#### custom_theme.cpp
Shows how to create a custom theme:
```cpp
class MyCustomTheme : public BaseTheme
{
public:
    QString name() const override { return "my_theme"; }
    QColor primaryColor() const override { return QColor("#FF6B35"); }
    // ... implement other methods
};

// Register custom theme
themeFactory->registerTheme("my_theme", []() {
    return std::make_shared<MyCustomTheme>();
});
```

#### layout_manager.cpp
Demonstrates advanced layout management:
```cpp
#include "layouts/MainLayout.h"

auto layout = new MainLayout();
layout->setAreaWidget(MainLayout::CentralArea, centralWidget);
layout->setAreaWidget(MainLayout::SideBarArea, sideBar);
layout->setSideBarVisible(true);
layout->apply(mainWindow);
```

#### responsive_ui.cpp
Shows responsive UI design techniques:
```cpp
// Enable responsive mode
layout->setResponsive(true);

// Handle size changes
connect(window, &QWidget::resized, [=](const QSize& size) {
    layout->adaptToSize(size);
});
```

### Integration Examples

#### with_audio.cpp
Demonstrates UI module integration with audio module:
```cpp
#include "UIModule.h"
#include "AudioModule.h"

// Initialize both modules
UIModule::instance()->initialize();
AudioModule::instance()->initialize();

// Create audio control UI
auto audioControls = new AudioControlWidget();
uiManager->registerWidget("audio_controls", audioControls);
```

#### full_app.cpp
Complete application example showing all features:
- Module initialization
- Theme management
- Layout management
- Custom widgets
- Configuration management
- Event handling

## Running Examples

Each example can be run independently:

```bash
# Run basic window example
cd examples/basic
./simple_window

# Run theme switching example
cd examples/basic
./theme_switching

# Run advanced layout example
cd examples/advanced
./layout_manager
```

## Example Features

### Interactive Demos
- Real-time theme switching
- Dynamic layout changes
- Widget customization
- Configuration editing

### Code Documentation
- Inline comments explaining key concepts
- Step-by-step implementation guides
- Best practices demonstrations
- Common pitfalls and solutions

### Learning Path
1. Start with `basic/simple_window.cpp`
2. Progress to `basic/theme_switching.cpp`
3. Explore `widgets/` examples
4. Try `layouts/` examples
5. Study `advanced/` examples
6. Review `integration/` examples

## Contributing Examples

When adding new examples:
1. Follow the existing directory structure
2. Include comprehensive comments
3. Provide a README for complex examples
4. Test on multiple platforms
5. Update this main README

## Troubleshooting

### Common Issues
- **Build errors**: Check Qt installation and CMake configuration
- **Runtime errors**: Verify module initialization order
- **Theme issues**: Check theme file paths and resource loading
- **Layout problems**: Verify widget parent-child relationships

### Getting Help
- Check the main UI module documentation
- Review existing examples for similar use cases
- Consult the API reference documentation
- Ask questions in the project forums