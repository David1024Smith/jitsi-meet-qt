#include "LightTheme.h"
#include <QApplication>
#include <QPixmap>
#include <QDir>
#include <QStandardPaths>

LightTheme::LightTheme(QObject *parent)
    : BaseTheme(parent)
    , m_borderRadius(6)
    , m_borderWidth(1)
    , m_spacing(8)
    , m_margin(12)
    , m_padding(8)
    , m_iconSize(24)
    , m_buttonHeight(32)
    , m_toolbarHeight(48)
    , m_loaded(false)
{
    m_name = "light";
    m_displayName = "Light Theme";
    
    // 设置资源路径
    m_resourcePath = ":/ui/themes/light";
    m_iconPath = m_resourcePath + "/icons";
    m_imagePath = m_resourcePath + "/images";
}

LightTheme::~LightTheme()
{
    if (m_loaded) {
        unload();
    }
}

QString LightTheme::name() const
{
    return "light";
}

QString LightTheme::displayName() const
{
    return tr("Light Theme");
}

QString LightTheme::description() const
{
    return tr("A light theme optimized for bright environments with light backgrounds and dark text.");
}

QString LightTheme::version() const
{
    return "1.0.0";
}

QString LightTheme::author() const
{
    return "Jitsi Meet Qt Team";
}

bool LightTheme::load()
{
    if (m_loaded) {
        return true;
    }

    try {
        initializeColors();
        initializeFonts();
        initializeSizes();
        initializeStyleSheets();
        initializeResources();
        
        m_loaded = true;
        emit themeLoaded();
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to load light theme: %1").arg(e.what()));
        return false;
    }
}

bool LightTheme::apply()
{
    if (!m_loaded && !load()) {
        return false;
    }

    try {
        // 应用样式表到应用程序
        if (QApplication::instance()) {
            QApplication::instance()->setStyleSheet(getStyleSheet());
        }
        
        emit themeApplied();
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply light theme: %1").arg(e.what()));
        return false;
    }
}

void LightTheme::unload()
{
    if (!m_loaded) {
        return;
    }

    // 清除缓存的样式表
    m_cachedStyleSheet.clear();
    m_cachedWidgetStyleSheets.clear();
    
    m_loaded = false;
    emit themeUnloaded();
}

bool LightTheme::isLoaded() const
{
    return m_loaded;
}

// 颜色方案实现
QColor LightTheme::primaryColor() const
{
    return m_primaryColor;
}

QColor LightTheme::secondaryColor() const
{
    return m_secondaryColor;
}

QColor LightTheme::backgroundColor() const
{
    return m_backgroundColor;
}

QColor LightTheme::textColor() const
{
    return m_textColor;
}

QColor LightTheme::accentColor() const
{
    return m_accentColor;
}

QColor LightTheme::borderColor() const
{
    return m_borderColor;
}

QColor LightTheme::hoverColor() const
{
    return m_hoverColor;
}

QColor LightTheme::pressedColor() const
{
    return m_pressedColor;
}

QColor LightTheme::disabledColor() const
{
    return m_disabledColor;
}

QColor LightTheme::errorColor() const
{
    return m_errorColor;
}

QColor LightTheme::warningColor() const
{
    return m_warningColor;
}

QColor LightTheme::successColor() const
{
    return m_successColor;
}

// 字体设置实现
QFont LightTheme::defaultFont() const
{
    return m_defaultFont;
}

QFont LightTheme::titleFont() const
{
    return m_titleFont;
}

QFont LightTheme::headerFont() const
{
    return m_headerFont;
}

QFont LightTheme::buttonFont() const
{
    return m_buttonFont;
}

QFont LightTheme::menuFont() const
{
    return m_menuFont;
}

QFont LightTheme::tooltipFont() const
{
    return m_tooltipFont;
}

// 尺寸设置实现
int LightTheme::borderRadius() const
{
    return m_borderRadius;
}

int LightTheme::borderWidth() const
{
    return m_borderWidth;
}

int LightTheme::spacing() const
{
    return m_spacing;
}

int LightTheme::margin() const
{
    return m_margin;
}

int LightTheme::padding() const
{
    return m_padding;
}

int LightTheme::iconSize() const
{
    return m_iconSize;
}

int LightTheme::buttonHeight() const
{
    return m_buttonHeight;
}

int LightTheme::toolbarHeight() const
{
    return m_toolbarHeight;
}

// 样式表实现
QString LightTheme::styleSheet() const
{
    return getStyleSheet();
}

QString LightTheme::getStyleSheet() const
{
    if (!m_cachedStyleSheet.isEmpty()) {
        return m_cachedStyleSheet;
    }

    QString css = QString(R"(
        /* 全局样式 */
        QWidget {
            background-color: %1;
            color: %2;
            font-family: %3;
            font-size: %4px;
        }
        
        /* 主窗口 */
        QMainWindow {
            background-color: %1;
            border: none;
        }
        
        /* 按钮样式 */
        %5
        
        /* 菜单样式 */
        %6
        
        /* 工具栏样式 */
        %7
        
        /* 状态栏样式 */
        %8
        
        /* 对话框样式 */
        %9
        
        /* 滚动条样式 */
        QScrollBar:vertical {
            background-color: %10;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background-color: %11;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: %12;
        }
        
        /* 分割器样式 */
        QSplitter::handle {
            background-color: %13;
        }
        
        QSplitter::handle:horizontal {
            width: 2px;
        }
        
        QSplitter::handle:vertical {
            height: 2px;
        }
    )")
    .arg(m_backgroundColor.name())           // %1
    .arg(m_textColor.name())                // %2
    .arg(m_defaultFont.family())            // %3
    .arg(m_defaultFont.pointSize())         // %4
    .arg(getButtonStyleSheet())             // %5
    .arg(getMenuStyleSheet())               // %6
    .arg(getToolBarStyleSheet())            // %7
    .arg(getStatusBarStyleSheet())          // %8
    .arg(getDialogStyleSheet())             // %9
    .arg(m_backgroundColor.darker(110).name()) // %10
    .arg(m_borderColor.name())              // %11
    .arg(m_hoverColor.name())               // %12
    .arg(m_borderColor.name());             // %13

    m_cachedStyleSheet = css;
    return css;
}

QString LightTheme::getWidgetStyleSheet(const QString& widgetType) const
{
    if (m_cachedWidgetStyleSheets.contains(widgetType)) {
        return m_cachedWidgetStyleSheets[widgetType];
    }

    QString css;
    if (widgetType == "QPushButton") {
        css = getButtonStyleSheet();
    } else if (widgetType == "QMenu") {
        css = getMenuStyleSheet();
    } else if (widgetType == "QToolBar") {
        css = getToolBarStyleSheet();
    } else if (widgetType == "QStatusBar") {
        css = getStatusBarStyleSheet();
    } else if (widgetType == "QDialog") {
        css = getDialogStyleSheet();
    }

    m_cachedWidgetStyleSheets[widgetType] = css;
    return css;
}

QString LightTheme::getButtonStyleSheet() const
{
    return generateLightButtonStyleSheet();
}

QString LightTheme::getMenuStyleSheet() const
{
    return generateLightMenuStyleSheet();
}

QString LightTheme::getToolBarStyleSheet() const
{
    return generateLightToolBarStyleSheet();
}

QString LightTheme::getStatusBarStyleSheet() const
{
    return generateLightStatusBarStyleSheet();
}

QString LightTheme::getDialogStyleSheet() const
{
    return generateLightDialogStyleSheet();
}

// 图标和资源实现
QString LightTheme::getIconPath(const QString& iconName) const
{
    return QString("%1/%2.svg").arg(m_iconPath, iconName);
}

QString LightTheme::getImagePath(const QString& imageName) const
{
    return QString("%1/%2").arg(m_imagePath, imageName);
}

QPixmap LightTheme::getIcon(const QString& iconName, const QSize& size) const
{
    QString iconPath = getIconPath(iconName);
    QPixmap pixmap(iconPath);
    
    if (!pixmap.isNull() && !size.isEmpty()) {
        pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return pixmap;
}

QPixmap LightTheme::getImage(const QString& imageName) const
{
    QString imagePath = getImagePath(imageName);
    return QPixmap(imagePath);
}

// 主题自定义实现
void LightTheme::setCustomProperty(const QString& property, const QVariant& value)
{
    m_customProperties[property] = value;
    emit propertyChanged(property, value);
}

QVariant LightTheme::getCustomProperty(const QString& property) const
{
    return m_customProperties.value(property);
}

bool LightTheme::hasCustomProperty(const QString& property) const
{
    return m_customProperties.contains(property);
}

void LightTheme::removeCustomProperty(const QString& property)
{
    if (m_customProperties.remove(property) > 0) {
        emit propertyChanged(property, QVariant());
    }
}

// 保护方法实现
void LightTheme::initializeColors()
{
    setupLightColors();
}

void LightTheme::initializeFonts()
{
    setupLightFonts();
}

void LightTheme::initializeSizes()
{
    setupLightSizes();
}

void LightTheme::initializeStyleSheets()
{
    // 清除缓存，强制重新生成
    m_cachedStyleSheet.clear();
    m_cachedWidgetStyleSheets.clear();
}

void LightTheme::initializeResources()
{
    // 初始化资源路径
    // 这里可以添加资源验证逻辑
}

// 私有方法实现
void LightTheme::setupLightColors()
{
    // 亮色主题颜色方案
    m_primaryColor = QColor(0x21, 0x96, 0xF3);      // 蓝色主色
    m_secondaryColor = QColor(0xF5, 0xF5, 0xF5);    // 浅灰色
    m_backgroundColor = QColor(0xFF, 0xFF, 0xFF);   // 白色背景
    m_textColor = QColor(0x21, 0x21, 0x21);         // 深色文字
    m_accentColor = QColor(0xFF, 0x57, 0x22);       // 橙色强调色
    m_borderColor = QColor(0xE0, 0xE0, 0xE0);       // 边框颜色
    m_hoverColor = QColor(0xF0, 0xF0, 0xF0);        // 悬停颜色
    m_pressedColor = QColor(0xE8, 0xE8, 0xE8);      // 按下颜色
    m_disabledColor = QColor(0xBD, 0xBD, 0xBD);     // 禁用颜色
    m_errorColor = QColor(0xF4, 0x43, 0x36);        // 错误颜色
    m_warningColor = QColor(0xFF, 0x98, 0x00);      // 警告颜色
    m_successColor = QColor(0x4C, 0xAF, 0x50);      // 成功颜色
}

void LightTheme::setupLightFonts()
{
    // 设置字体
    m_defaultFont = QFont("Segoe UI", 9);
    m_titleFont = QFont("Segoe UI", 14, QFont::Bold);
    m_headerFont = QFont("Segoe UI", 12, QFont::DemiBold);
    m_buttonFont = QFont("Segoe UI", 9);
    m_menuFont = QFont("Segoe UI", 9);
    m_tooltipFont = QFont("Segoe UI", 8);
}

void LightTheme::setupLightSizes()
{
    // 尺寸已在构造函数中设置
}

QString LightTheme::generateLightButtonStyleSheet() const
{
    return QString(R"(
        QPushButton {
            background-color: %1;
            border: %2px solid %3;
            border-radius: %4px;
            padding: %5px %6px;
            font-family: %7;
            font-size: %8px;
            color: %9;
            min-height: %10px;
        }
        
        QPushButton:hover {
            background-color: %11;
            border-color: %12;
        }
        
        QPushButton:pressed {
            background-color: %13;
        }
        
        QPushButton:disabled {
            background-color: %14;
            color: %15;
            border-color: %16;
        }
    )")
    .arg(m_secondaryColor.name())           // %1
    .arg(m_borderWidth)                     // %2
    .arg(m_borderColor.name())              // %3
    .arg(m_borderRadius)                    // %4
    .arg(m_padding)                         // %5
    .arg(m_padding * 2)                     // %6
    .arg(m_buttonFont.family())             // %7
    .arg(m_buttonFont.pointSize())          // %8
    .arg(m_textColor.name())                // %9
    .arg(m_buttonHeight)                    // %10
    .arg(m_hoverColor.name())               // %11
    .arg(m_accentColor.name())              // %12
    .arg(m_pressedColor.name())             // %13
    .arg(m_disabledColor.name())            // %14
    .arg(m_disabledColor.darker(150).name()) // %15
    .arg(m_disabledColor.name());           // %16
}

QString LightTheme::generateLightMenuStyleSheet() const
{
    return QString(R"(
        QMenu {
            background-color: %1;
            border: %2px solid %3;
            border-radius: %4px;
            padding: %5px;
        }
        
        QMenu::item {
            background-color: transparent;
            padding: %6px %7px;
            border-radius: %8px;
        }
        
        QMenu::item:selected {
            background-color: %9;
        }
        
        QMenu::item:disabled {
            color: %10;
        }
        
        QMenu::separator {
            height: 1px;
            background-color: %11;
            margin: %12px;
        }
    )")
    .arg(m_backgroundColor.name())          // %1
    .arg(m_borderWidth)                     // %2
    .arg(m_borderColor.name())              // %3
    .arg(m_borderRadius)                    // %4
    .arg(m_padding / 2)                     // %5
    .arg(m_padding)                         // %6
    .arg(m_padding * 2)                     // %7
    .arg(m_borderRadius / 2)                // %8
    .arg(m_hoverColor.name())               // %9
    .arg(m_disabledColor.name())            // %10
    .arg(m_borderColor.name())              // %11
    .arg(m_padding);                        // %12
}

QString LightTheme::generateLightToolBarStyleSheet() const
{
    return QString(R"(
        QToolBar {
            background-color: %1;
            border: none;
            border-bottom: %2px solid %3;
            spacing: %4px;
            padding: %5px;
        }
        
        QToolBar::handle {
            background-color: %6;
            width: 2px;
            margin: %7px;
        }
        
        QToolBar QToolButton {
            background-color: transparent;
            border: none;
            border-radius: %8px;
            padding: %9px;
            margin: %10px;
        }
        
        QToolBar QToolButton:hover {
            background-color: %11;
        }
        
        QToolBar QToolButton:pressed {
            background-color: %12;
        }
    )")
    .arg(m_backgroundColor.name())          // %1
    .arg(m_borderWidth)                     // %2
    .arg(m_borderColor.name())              // %3
    .arg(m_spacing)                         // %4
    .arg(m_padding)                         // %5
    .arg(m_borderColor.name())              // %6
    .arg(m_padding)                         // %7
    .arg(m_borderRadius)                    // %8
    .arg(m_padding)                         // %9
    .arg(2)                                 // %10
    .arg(m_hoverColor.name())               // %11
    .arg(m_pressedColor.name());            // %12
}

QString LightTheme::generateLightStatusBarStyleSheet() const
{
    return QString(R"(
        QStatusBar {
            background-color: %1;
            border: none;
            border-top: %2px solid %3;
            padding: %4px;
        }
        
        QStatusBar::item {
            border: none;
        }
        
        QStatusBar QLabel {
            color: %5;
            padding: %6px;
        }
    )")
    .arg(m_backgroundColor.name())          // %1
    .arg(m_borderWidth)                     // %2
    .arg(m_borderColor.name())              // %3
    .arg(m_padding / 2)                     // %4
    .arg(m_textColor.name())                // %5
    .arg(m_padding / 2);                    // %6
}

QString LightTheme::generateLightDialogStyleSheet() const
{
    return QString(R"(
        QDialog {
            background-color: %1;
            border: %2px solid %3;
            border-radius: %4px;
        }
        
        QDialog QLabel {
            color: %5;
        }
        
        QDialog QPushButton {
            min-width: 80px;
        }
    )")
    .arg(m_backgroundColor.name())          // %1
    .arg(m_borderWidth)                     // %2
    .arg(m_borderColor.name())              // %3
    .arg(m_borderRadius)                    // %4
    .arg(m_textColor.name());               // %5
}