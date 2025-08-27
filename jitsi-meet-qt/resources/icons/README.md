# Icon Files

This directory contains the application icons. The SVG files are the source files that should be converted to the required formats:

## Required Conversions:

1. **app.svg** → **app.ico** (64x64, 32x32, 16x16 sizes)
2. **app.svg** → **app.png** (64x64)
3. **settings.svg** → **settings.png** (24x24)
4. **about.svg** → **about.png** (24x24)
5. **back.svg** → **back.png** (24x24)
6. **join.svg** → **join.png** (24x24)
7. **recent.svg** → **recent.png** (24x24)

## Conversion Commands:

Using Inkscape (if available):
```bash
# Convert SVG to PNG
inkscape --export-png=app.png --export-width=64 --export-height=64 app.svg
inkscape --export-png=settings.png --export-width=24 --export-height=24 settings.svg

# Convert PNG to ICO (using ImageMagick)
convert app.png -resize 64x64 -resize 32x32 -resize 16x16 app.ico
```

Or use online converters or Qt's built-in SVG support by updating the .qrc file to reference .svg files directly.