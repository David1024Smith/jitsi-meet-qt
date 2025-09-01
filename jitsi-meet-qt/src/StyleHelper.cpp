#include "StyleHelper.h"
#include <QApplication>
#include <QStyleHints>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

StyleHelper* StyleHelper::s_instance = nullptr;

StyleHelper::StyleHelper(QObject *parent)
    : QObject(parent)
    , m_currentTheme(LightTheme)
    , m_initialized(false)
{
    s_instance = this;
}

StyleHelper::~StyleHelper()
{
    s_instance = nullptr;
}

StyleHelper* StyleHelper::instance()
{
    return s_instance;
}

bool StyleHelper::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing StyleHelper...";
    
    loadDefaultColors();
    loadDefaultFonts();
    loadDefaultStyleSheets();
    connectSignals();
    
    // Detect system theme
    StyleTheme systemTheme = detectSystemTheme();
    setTheme(systemTheme);
    
    m_initialized = true;
    qDebug() << "StyleHelper initialized successfully";
    
    return true;
}

void StyleHelper::shutdown()
{
    m_initialized = false;
}

void StyleHelper::setTheme(StyleTheme theme)
{
    if (m_currentTheme == theme) {
        return;
    }
    
    m_currentTheme = theme;
    loadDefaultColors();
    updateStyleSheets();
    
    emit themeChanged(theme);
    qDebug() << "Theme changed to:" << themeName(theme);
}

StyleHelper::StyleTheme StyleHelper::currentTheme() const
{
    return m_currentTheme;
}

QColor StyleHelper::getColor(ColorRole role) const
{
    return m_colors.value(role, QColor(Qt::black));
}

void StyleHelper::setColor(ColorRole role, const QColor& color)
{
    if (m_colors.value(role) != color) {
        m_colors[role] = color;
        updateStyleSheets();
        emit colorChanged(role, color);
    }
}

QFont StyleHelper::getFont(FontRole role) const
{
    return m_fonts.value(role, QApplication::font());
}

void StyleHelper::setFont(FontRole role, const QFont& font)
{
    if (m_fonts.value(role) != font) {
        m_fonts[role] = font;
        updateStyleSheets();
        emit fontChanged(role, font);
    }
}

QString StyleHelper::getIconPath(const QString& iconName) const
{
    return m_iconPaths.value(iconName, QString(":/icons/%1.png").arg(iconName));
}

QString StyleHelper::getStyleSheet(const QString& widgetType) const
{
    return m_styleSheets.value(widgetType, QString());
}

QString StyleHelper::getFullStyleSheet() const
{
    return m_fullStyleSheet;
}

bool StyleHelper::loadStyleSheetFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open stylesheet file:" << filePath;
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    
    // Parse and apply the stylesheet content
    // This is a simplified implementation
    m_fullStyleSheet = content;
    applyStyleToApplication();
    
    return true;
}

bool StyleHelper::saveStyleSheetFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to save stylesheet file:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << m_fullStyleSheet;
    
    return true;
}

void StyleHelper::resetToDefaultStyle()
{
    loadDefaultColors();
    loadDefaultFonts();
    loadDefaultStyleSheets();
    updateStyleSheets();
}

void StyleHelper::applyStyleToApplication()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (app) {
        app->setStyleSheet(m_fullStyleSheet);
        emit styleSheetChanged();
    }
}

QString StyleHelper::colorRoleName(ColorRole role)
{
    switch (role) {
    case Primary: return "Primary";
    case Secondary: return "Secondary";
    case Success: return "Success";
    case Danger: return "Danger";
    case Warning: return "Warning";
    case Info: return "Info";
    case Light: return "Light";
    case Dark: return "Dark";
    case Background: return "Background";
    case Surface: return "Surface";
    case Text: return "Text";
    case TextSecondary: return "TextSecondary";
    case Border: return "Border";
    case Disabled: return "Disabled";
    case Highlight: return "Highlight";
    case Link: return "Link";
    }
    return "Unknown";
}

QString StyleHelper::fontRoleName(FontRole role)
{
    switch (role) {
    case Default: return "Default";
    case Title: return "Title";
    case Subtitle: return "Subtitle";
    case Heading1: return "Heading1";
    case Heading2: return "Heading2";
    case Heading3: return "Heading3";
    case Small: return "Small";
    case Monospace: return "Monospace";
    case Button: return "Button";
    }
    return "Unknown";
}

QString StyleHelper::themeName(StyleTheme theme)
{
    switch (theme) {
    case LightTheme: return "Light";
    case DarkTheme: return "Dark";
    case SystemTheme: return "System";
    case CustomTheme: return "Custom";
    }
    return "Unknown";
}

StyleHelper::StyleTheme StyleHelper::detectSystemTheme()
{
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (app) {
        QStyleHints* hints = app->styleHints();
        if (hints && hints->colorScheme() == Qt::ColorScheme::Dark) {
            return DarkTheme;
        }
    }
    return LightTheme;
}

QVariantMap StyleHelper::getStyleConfig() const
{
    QVariantMap config;
    config["theme"] = static_cast<int>(m_currentTheme);
    
    // Save colors
    QVariantMap colors;
    for (auto it = m_colors.constBegin(); it != m_colors.constEnd(); ++it) {
        colors[colorRoleName(it.key())] = it.value().name();
    }
    config["colors"] = colors;
    
    // Save fonts
    QVariantMap fonts;
    for (auto it = m_fonts.constBegin(); it != m_fonts.constEnd(); ++it) {
        QVariantMap fontData;
        fontData["family"] = it.value().family();
        fontData["size"] = it.value().pointSize();
        fontData["bold"] = it.value().bold();
        fontData["italic"] = it.value().italic();
        fonts[fontRoleName(it.key())] = fontData;
    }
    config["fonts"] = fonts;
    
    return config;
}

void StyleHelper::setStyleConfig(const QVariantMap& config)
{
    // Load theme
    if (config.contains("theme")) {
        setTheme(static_cast<StyleTheme>(config["theme"].toInt()));
    }
    
    // Load colors
    if (config.contains("colors")) {
        QVariantMap colors = config["colors"].toMap();
        for (auto it = colors.constBegin(); it != colors.constEnd(); ++it) {
            // Find color role by name and set color
            for (int i = Primary; i <= Link; ++i) {
                ColorRole role = static_cast<ColorRole>(i);
                if (colorRoleName(role) == it.key()) {
                    setColor(role, QColor(it.value().toString()));
                    break;
                }
            }
        }
    }
    
    // Load fonts
    if (config.contains("fonts")) {
        QVariantMap fonts = config["fonts"].toMap();
        for (auto it = fonts.constBegin(); it != fonts.constEnd(); ++it) {
            QVariantMap fontData = it.value().toMap();
            QFont font;
            font.setFamily(fontData["family"].toString());
            font.setPointSize(fontData["size"].toInt());
            font.setBold(fontData["bold"].toBool());
            font.setItalic(fontData["italic"].toBool());
            
            // Find font role by name and set font
            for (int i = Default; i <= Button; ++i) {
                FontRole role = static_cast<FontRole>(i);
                if (fontRoleName(role) == it.key()) {
                    setFont(role, font);
                    break;
                }
            }
        }
    }
}

void StyleHelper::loadDefaultColors()
{
    if (m_currentTheme == DarkTheme) {
        // Dark theme colors
        m_colors[Primary] = QColor(0, 122, 255);
        m_colors[Secondary] = QColor(108, 117, 125);
        m_colors[Success] = QColor(40, 167, 69);
        m_colors[Danger] = QColor(220, 53, 69);
        m_colors[Warning] = QColor(255, 193, 7);
        m_colors[Info] = QColor(23, 162, 184);
        m_colors[Light] = QColor(248, 249, 250);
        m_colors[Dark] = QColor(52, 58, 64);
        m_colors[Background] = QColor(33, 37, 41);
        m_colors[Surface] = QColor(52, 58, 64);
        m_colors[Text] = QColor(255, 255, 255);
        m_colors[TextSecondary] = QColor(173, 181, 189);
        m_colors[Border] = QColor(73, 80, 87);
        m_colors[Disabled] = QColor(108, 117, 125);
        m_colors[Highlight] = QColor(0, 122, 255);
        m_colors[Link] = QColor(0, 122, 255);
    } else {
        // Light theme colors
        m_colors[Primary] = QColor(0, 122, 255);
        m_colors[Secondary] = QColor(108, 117, 125);
        m_colors[Success] = QColor(40, 167, 69);
        m_colors[Danger] = QColor(220, 53, 69);
        m_colors[Warning] = QColor(255, 193, 7);
        m_colors[Info] = QColor(23, 162, 184);
        m_colors[Light] = QColor(248, 249, 250);
        m_colors[Dark] = QColor(52, 58, 64);
        m_colors[Background] = QColor(255, 255, 255);
        m_colors[Surface] = QColor(248, 249, 250);
        m_colors[Text] = QColor(33, 37, 41);
        m_colors[TextSecondary] = QColor(108, 117, 125);
        m_colors[Border] = QColor(222, 226, 230);
        m_colors[Disabled] = QColor(173, 181, 189);
        m_colors[Highlight] = QColor(0, 122, 255);
        m_colors[Link] = QColor(0, 122, 255);
    }
}

void StyleHelper::loadDefaultFonts()
{
    QFont defaultFont = QApplication::font();
    
    m_fonts[Default] = defaultFont;
    
    QFont titleFont = defaultFont;
    titleFont.setPointSize(defaultFont.pointSize() + 8);
    titleFont.setBold(true);
    m_fonts[Title] = titleFont;
    
    QFont subtitleFont = defaultFont;
    subtitleFont.setPointSize(defaultFont.pointSize() + 4);
    m_fonts[Subtitle] = subtitleFont;
    
    QFont h1Font = defaultFont;
    h1Font.setPointSize(defaultFont.pointSize() + 6);
    h1Font.setBold(true);
    m_fonts[Heading1] = h1Font;
    
    QFont h2Font = defaultFont;
    h2Font.setPointSize(defaultFont.pointSize() + 4);
    h2Font.setBold(true);
    m_fonts[Heading2] = h2Font;
    
    QFont h3Font = defaultFont;
    h3Font.setPointSize(defaultFont.pointSize() + 2);
    h3Font.setBold(true);
    m_fonts[Heading3] = h3Font;
    
    QFont smallFont = defaultFont;
    smallFont.setPointSize(defaultFont.pointSize() - 2);
    m_fonts[Small] = smallFont;
    
    QFont monoFont("Consolas", defaultFont.pointSize());
    if (!monoFont.exactMatch()) {
        monoFont.setFamily("Courier New");
    }
    m_fonts[Monospace] = monoFont;
    
    m_fonts[Button] = defaultFont;
}

void StyleHelper::loadDefaultStyleSheets()
{
    // Load default stylesheets for different widget types
    m_styleSheets["QPushButton"] = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %4;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %5;"
        "}"
    ).arg(getColor(Primary).name())
     .arg(getColor(Text).name())
     .arg(getColor(Border).name())
     .arg(getColor(Primary).lighter(110).name())
     .arg(getColor(Primary).darker(110).name());
    
    m_styleSheets["QLineEdit"] = QString(
        "QLineEdit {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %4;"
        "}"
    ).arg(getColor(Background).name())
     .arg(getColor(Text).name())
     .arg(getColor(Border).name())
     .arg(getColor(Primary).name());
}

void StyleHelper::updateStyleSheets()
{
    loadDefaultStyleSheets();
    generateFullStyleSheet();
}

void StyleHelper::generateFullStyleSheet()
{
    QStringList sheets;
    for (auto it = m_styleSheets.constBegin(); it != m_styleSheets.constEnd(); ++it) {
        sheets << it.value();
    }
    m_fullStyleSheet = sheets.join("\n\n");
}

QString StyleHelper::colorToStyleSheet(const QColor& color) const
{
    return color.name();
}

QString StyleHelper::fontToStyleSheet(const QFont& font) const
{
    return QString("font-family: %1; font-size: %2pt;")
        .arg(font.family())
        .arg(font.pointSize());
}

void StyleHelper::connectSignals()
{
    // Connect to system theme changes if available
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (app) {
        QStyleHints* hints = app->styleHints();
        if (hints) {
            connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme scheme) {
                if (m_currentTheme == SystemTheme) {
                    setTheme(scheme == Qt::ColorScheme::Dark ? DarkTheme : LightTheme);
                }
            });
        }
    }
}