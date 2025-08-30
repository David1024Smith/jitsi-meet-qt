# UI Module Resources

This directory contains all the resources used by the UI module, including themes, icons, images, and style sheets.

## Directory Structure

```
resources/
├── themes/          # Theme-specific resources
│   ├── default/     # Default theme resources
│   ├── dark/        # Dark theme resources
│   └── light/       # Light theme resources
├── icons/           # Icon resources
│   ├── actions/     # Action icons (buttons, menu items)
│   ├── status/      # Status indicators
│   └── ui/          # UI element icons
├── styles/          # Style sheet files
│   ├── base.qss     # Base styles
│   ├── widgets.qss  # Widget-specific styles
│   └── themes/      # Theme-specific style sheets
└── ui_resources.qrc # Qt resource file
```

## Usage

Resources are accessed through the Qt resource system using the `:/` prefix:

```cpp
// Loading an icon
QIcon icon(":/icons/actions/play.png");

// Loading a style sheet
QFile file(":/styles/base.qss");
file.open(QFile::ReadOnly);
QString styleSheet = file.readAll();
```

## Adding New Resources

1. Add the resource file to the appropriate subdirectory
2. Update the `ui_resources.qrc` file to include the new resource
3. Rebuild the project to compile the resources

## Theme Resources

Each theme should have its own subdirectory under `themes/` containing:
- `colors.json` - Color definitions
- `fonts.json` - Font specifications  
- `sizes.json` - Size and spacing values
- `icons/` - Theme-specific icons
- `images/` - Theme-specific images

## Icon Guidelines

- Use SVG format when possible for scalability
- Provide multiple sizes: 16x16, 24x24, 32x32, 48x48
- Follow the naming convention: `category_action_state.ext`
- Example: `media_play_normal.svg`, `media_play_hover.svg`

## Style Sheet Guidelines

- Use CSS-like syntax compatible with Qt Style Sheets
- Organize styles by widget type
- Use variables for colors and sizes when possible
- Comment complex selectors and rules