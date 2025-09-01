#include "DefaultTheme.h"
#include <QApplication>
#include <QPixmap>
#include <QDebug>

DefaultTheme::DefaultTheme(QObject *parent)
    : BaseTheme(parent)
    , m_borderRadius(4)
    , m_borderWidth(1)
    , m_spacing(8)
    , m_margin(16)
    , m_padding(8)
    , m_iconSize(16)
    , m_buttonHeight(32)
    , m_toolbarHeight(40)
    , m_loaded(false)
{
    m_name = "default";
    m_displayName = "Default Theme";
    
    setupDefaultColors();
    setupDefaultFonts();
    setupDefaultSizes();
}

DefaultTheme::~DefaultTheme()
{
    unload();
}

QString DefaultTheme::name() const
{
    return m_name;
}

QString DefaultTheme::displayName() const
{
    return m_displayName;
}

QString DefaultTheme::description() const
{
    return "Default theme for Jitsi Meet Qt application";
}

QString DefaultTheme::version() const
{
    return "1.0.0";
}

QString DefaultTheme::author() const
{
    return "Jitsi Meet Qt Team";
}

bool DefaultTheme::load()
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
        qDebug() << "Default theme loaded successfully";
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to load default theme: %1").arg(e.what()));
        return false;
    }
}

bool DefaultTheme::apply()
{
    if (!m_loaded && !load()) {
        return false;
    }

    try {
        // 应用样式表到应用程序
        if (QApplication::instance()) {
            qobject_cast<QApplication*>(QApplication::instance())->setStyleSheet(getStyleSheet());
        }

        emit themeApplied();
        qDebug() << "Default theme applied successfully";
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply default theme: %1").arg(e.what()));
        return false;
    }
}

void DefaultTheme::unload()
{
    if (!m_loaded) {
        return;
    }

    // 清理缓存
    m_cachedStyleSheet.clear();
    m_cachedWidgetStyleSheets.clear();

    m_loaded = false;
    emit themeUnloaded();
    qDebug() << "Default theme unloaded";
}

bool DefaultTheme::isLoaded() const
{
    return m_loaded;
}

// 颜色方案实现
QColor DefaultTheme::primaryColor() const
{
    return m_primaryColor;
}

QColor DefaultTheme::secondaryColor() const
{
    return m_secondaryColor;
}

QColor DefaultTheme::backgroundColor() const
{
    return m_backgroundColor;
}

QColor DefaultTheme::textColor() const
{
    return m_textColor;
}

QColor DefaultTheme::accentColor() const
{
    return m_accentColor;
}

QColor DefaultTheme::borderColor() const
{
    return m_borderColor;
}

QColor DefaultTheme::hoverColor() const
{
    return m_hoverColor;
}

QColor DefaultTheme::pressedColor() const
{
    return m_pressedColor;
}

QColor DefaultTheme::disabledColor() const
{
    return m_disabledColor;
}

QColor DefaultTheme::errorColor() const
{
    return m_errorColor;
}

QColor DefaultTheme::warningColor() const
{
    return m_warningColor;
}

QColor DefaultTheme::successColor() const
{
    return m_successColor;
}

// 字体设置实现
QFont DefaultTheme::defaultFont() const
{
    return m_defaultFont;
}

QFont DefaultTheme::titleFont() const
{
    return m_titleFont;
}

QFont DefaultTheme::headerFont() const
{
    return m_headerFont;
}

QFont DefaultTheme::buttonFont() const
{
    return m_buttonFont;
}

QFont DefaultTheme::menuFont() const
{
    return m_menuFont;
}

QFont DefaultTheme::tooltipFont() const
{
    return m_tooltipFont;
}

// 尺寸设置实现
int DefaultTheme::borderRadius() const
{
    return m_borderRadius;
}

int DefaultTheme::borderWidth() const
{
    return m_borderWidth;
}

int DefaultTheme::spacing() const
{
    return m_spacing;
}

int DefaultTheme::margin() const
{
    return m_margin;
}

int DefaultTheme::padding() const
{
    return m_padding;
}

int DefaultTheme::iconSize() const
{
    return m_iconSize;
}

int DefaultTheme::buttonHeight() const
{
    return m_buttonHeight;
}

int DefaultTheme::toolbarHeight() const
{
    return m_toolbarHeight;
}

// 样式表实现
QString DefaultTheme::styleSheet() const
{
    return getStyleSheet();
}

QString DefaultTheme::getStyleSheet() const
{
    if (m_cachedStyleSheet.isEmpty()) {
        QString styleSheet;
        
        // 基础样式
        styleSheet += QString(
            "QWidget {"
            "    background-color: %1;"
            "    color: %2;"
            "    font-family: %3;"
            "    font-size: %4px;"
            "}"
        ).arg(m_backgroundColor.name())
         .arg(m_textColor.name())
         .arg(m_defaultFont.family())
         .arg(m_defaultFont.pointSize());

        // 添加各种组件样式
        styleSheet += getButtonStyleSheet();
        styleSheet += getMenuStyleSheet();
        styleSheet += getToolBarStyleSheet();
        styleSheet += getStatusBarStyleSheet();
        styleSheet += getDialogStyleSheet();

        m_cachedStyleSheet = styleSheet;
    }
    
    return m_cachedStyleSheet;
}

QString DefaultTheme::getWidgetStyleSheet(const QString& widgetType) const
{
    if (m_cachedWidgetStyleSheets.contains(widgetType)) {
        return m_cachedWidgetStyleSheets[widgetType];
    }

    QString styleSheet;
    
    if (widgetType == "QPushButton") {
        styleSheet = getButtonStyleSheet();
    } else if (widgetType == "QMenu") {
        styleSheet = getMenuStyleSheet();
    } else if (widgetType == "QToolBar") {
        styleSheet = getToolBarStyleSheet();
    } else if (widgetType == "QStatusBar") {
        styleSheet = getStatusBarStyleSheet();
    } else if (widgetType == "QDialog") {
        styleSheet = getDialogStyleSheet();
    }

    m_cachedWidgetStyleSheets[widgetType] = styleSheet;
    return styleSheet;
}

QString DefaultTheme::getButtonStyleSheet() const
{
    return generateButtonStyleSheet();
}

QString DefaultTheme::getMenuStyleSheet() const
{
    return generateMenuStyleSheet();
}

QString DefaultTheme::getToolBarStyleSheet() const
{
    return generateToolBarStyleSheet();
}

QString DefaultTheme::getStatusBarStyleSheet() const
{
    return generateStatusBarStyleSheet();
}

QString DefaultTheme::getDialogStyleSheet() const
{
    return generateDialogStyleSheet();
}

// 图标和资源实现
QString DefaultTheme::getIconPath(const QString& iconName) const
{
    return QString("%1/%2.png").arg(m_iconPath, iconName);
}

QString DefaultTheme::getImagePath(const QString& imageName) const
{
    return QString("%1/%2.png").arg(m_imagePath, imageName);
}

QPixmap DefaultTheme::getIcon(const QString& iconName, const QSize& size) const
{
    QString path = getIconPath(iconName);
    QPixmap pixmap(path);
    
    if (!size.isEmpty() && !pixmap.isNull()) {
        pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return pixmap;
}

QPixmap DefaultTheme::getImage(const QString& imageName) const
{
    return QPixmap(getImagePath(imageName));
}

// 主题自定义实现
void DefaultTheme::setCustomProperty(const QString& property, const QVariant& value)
{
    m_customProperties[property] = value;
    
    // 清理缓存以强制重新生成样式表
    m_cachedStyleSheet.clear();
    m_cachedWidgetStyleSheets.clear();
    
    emit propertyChanged(property, value);
}

QVariant DefaultTheme::getCustomProperty(const QString& property) const
{
    return m_customProperties.value(property);
}

bool DefaultTheme::hasCustomProperty(const QString& property) const
{
    return m_customProperties.contains(property);
}

void DefaultTheme::removeCustomProperty(const QString& property)
{
    if (m_customProperties.remove(property) > 0) {
        // 清理缓存
        m_cachedStyleSheet.clear();
        m_cachedWidgetStyleSheets.clear();
        
        emit propertyChanged(property, QVariant());
    }
}

void DefaultTheme::setName(const QString& name)
{
    m_name = name;
}

void DefaultTheme::setDisplayName(const QString& displayName)
{
    m_displayName = displayName;
}

// 保护方法实现
void DefaultTheme::initializeColors()
{
    setupDefaultColors();
}

void DefaultTheme::initializeFonts()
{
    setupDefaultFonts();
}

void DefaultTheme::initializeSizes()
{
    setupDefaultSizes();
}

void DefaultTheme::initializeStyleSheets()
{
    // 清理缓存以强制重新生成
    m_cachedStyleSheet.clear();
    m_cachedWidgetStyleSheets.clear();
}

void DefaultTheme::initializeResources()
{
    m_resourcePath = ":/themes/default";
    m_iconPath = m_resourcePath + "/icons";
    m_imagePath = m_resourcePath + "/images";
}

// 私有方法实现
void DefaultTheme::setupDefaultColors()
{
    m_primaryColor = QColor("#2196F3");      // 蓝色
    m_secondaryColor = QColor("#FFC107");    // 琥珀色
    m_backgroundColor = QColor("#FFFFFF");   // 白色
    m_textColor = QColor("#212121");         // 深灰色
    m_accentColor = QColor("#FF5722");       // 深橙色
    m_borderColor = QColor("#E0E0E0");       // 浅灰色
    m_hoverColor = QColor("#1976D2");        // 深蓝色
    m_pressedColor = QColor("#1565C0");      // 更深蓝色
    m_disabledColor = QColor("#BDBDBD");     // 灰色
    m_errorColor = QColor("#F44336");        // 红色
    m_warningColor = QColor("#FF9800");      // 橙色
    m_successColor = QColor("#4CAF50");      // 绿色
}

void DefaultTheme::setupDefaultFonts()
{
    m_defaultFont = QFont("Arial", 10);
    m_titleFont = QFont("Arial", 16, QFont::Bold);
    m_headerFont = QFont("Arial", 14, QFont::Bold);
    m_buttonFont = QFont("Arial", 10);
    m_menuFont = QFont("Arial", 9);
    m_tooltipFont = QFont("Arial", 8);
}

void DefaultTheme::setupDefaultSizes()
{
    // 尺寸已在构造函数中设置
}

QString DefaultTheme::generateButtonStyleSheet() const
{
    return QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: %3px solid %4;"
        "    border-radius: %5px;"
        "    padding: %6px %7px;"
        "    font-family: %8;"
        "    font-size: %9px;"
        "    min-height: %10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %11;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %12;"
        "}"
        "QPushButton:disabled {"
        "    background-color: %13;"
        "    color: %14;"
        "}"
    ).arg(m_primaryColor.name())
     .arg(m_backgroundColor.name())
     .arg(m_borderWidth)
     .arg(m_borderColor.name())
     .arg(m_borderRadius)
     .arg(m_padding)
     .arg(m_padding * 2)
     .arg(m_buttonFont.family())
     .arg(m_buttonFont.pointSize())
     .arg(m_buttonHeight)
     .arg(m_hoverColor.name())
     .arg(m_pressedColor.name())
     .arg(m_disabledColor.name())
     .arg(m_textColor.lighter().name());
}

QString DefaultTheme::generateMenuStyleSheet() const
{
    return QString(
        "QMenu {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: %3px solid %4;"
        "    border-radius: %5px;"
        "}"
        "QMenu::item {"
        "    padding: %6px %7px;"
        "    font-family: %8;"
        "    font-size: %9px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: %10;"
        "}"
    ).arg(m_backgroundColor.name())
     .arg(m_textColor.name())
     .arg(m_borderWidth)
     .arg(m_borderColor.name())
     .arg(m_borderRadius)
     .arg(m_padding / 2)
     .arg(m_padding)
     .arg(m_menuFont.family())
     .arg(m_menuFont.pointSize())
     .arg(m_hoverColor.lighter().name());
}

QString DefaultTheme::generateToolBarStyleSheet() const
{
    return QString(
        "QToolBar {"
        "    background-color: %1;"
        "    border: none;"
        "    spacing: %2px;"
        "    min-height: %3px;"
        "}"
        "QToolBar::separator {"
        "    background-color: %4;"
        "    width: 1px;"
        "    margin: %5px;"
        "}"
    ).arg(m_backgroundColor.darker(105).name())
     .arg(m_spacing)
     .arg(m_toolbarHeight)
     .arg(m_borderColor.name())
     .arg(m_margin / 2);
}

QString DefaultTheme::generateStatusBarStyleSheet() const
{
    return QString(
        "QStatusBar {"
        "    background-color: %1;"
        "    color: %2;"
        "    border-top: %3px solid %4;"
        "    font-family: %5;"
        "    font-size: %6px;"
        "}"
    ).arg(m_backgroundColor.darker(105).name())
     .arg(m_textColor.name())
     .arg(m_borderWidth)
     .arg(m_borderColor.name())
     .arg(m_defaultFont.family())
     .arg(m_defaultFont.pointSize() - 1);
}

QString DefaultTheme::generateDialogStyleSheet() const
{
    return QString(
        "QDialog {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
    ).arg(m_backgroundColor.name())
     .arg(m_textColor.name());
}