#ifndef DEFAULTTHEME_H
#define DEFAULTTHEME_H

#include "BaseTheme.h"
#include <QColor>
#include <QFont>
#include <QString>

/**
 * @brief 默认主题类
 * 
 * DefaultTheme提供应用程序的默认主题样式，包括标准的
 * 颜色方案、字体设置和UI元素样式。
 */
class DefaultTheme : public BaseTheme
{
    Q_OBJECT

public:
    explicit DefaultTheme(QObject *parent = nullptr);
    ~DefaultTheme() override;

    // BaseTheme接口实现
    QString name() const override;
    QString displayName() const override;
    QString description() const override;
    QString version() const override;
    QString author() const override;

    // 主题加载和应用
    bool load() override;
    bool apply() override;
    void unload() override;
    bool isLoaded() const override;

    // 颜色方案
    QColor primaryColor() const override;
    QColor secondaryColor() const override;
    QColor backgroundColor() const override;
    QColor textColor() const override;
    QColor accentColor() const override;
    QColor borderColor() const override;
    QColor hoverColor() const override;
    QColor pressedColor() const override;
    QColor disabledColor() const override;
    QColor errorColor() const override;
    QColor warningColor() const override;
    QColor successColor() const override;

    // 字体设置
    QFont defaultFont() const override;
    QFont titleFont() const override;
    QFont headerFont() const override;
    QFont buttonFont() const override;
    QFont menuFont() const override;
    QFont tooltipFont() const override;

    // 尺寸设置
    int borderRadius() const override;
    int borderWidth() const override;
    int spacing() const override;
    int margin() const override;
    int padding() const override;
    int iconSize() const override;
    int buttonHeight() const override;
    int toolbarHeight() const override;

    // 样式表
    QString styleSheet() const override;
    QString getStyleSheet() const override;
    QString getWidgetStyleSheet(const QString& widgetType) const override;
    QString getButtonStyleSheet() const override;
    QString getMenuStyleSheet() const override;
    QString getToolBarStyleSheet() const override;
    QString getStatusBarStyleSheet() const override;
    QString getDialogStyleSheet() const override;

    // 图标和资源
    QString getIconPath(const QString& iconName) const override;
    QString getImagePath(const QString& imageName) const override;
    QPixmap getIcon(const QString& iconName, const QSize& size = QSize()) const override;
    QPixmap getImage(const QString& imageName) const override;

    // 主题自定义
    void setCustomProperty(const QString& property, const QVariant& value) override;
    QVariant getCustomProperty(const QString& property) const override;
    bool hasCustomProperty(const QString& property) const override;
    void removeCustomProperty(const QString& property) override;
    
    // 配置管理
    void setName(const QString& name) override;
    void setDisplayName(const QString& displayName) override;

protected:
    void initializeColors();
    void initializeFonts();
    void initializeSizes();
    void initializeStyleSheets();
    void initializeResources();

private:
    void setupDefaultColors();
    void setupDefaultFonts();
    void setupDefaultSizes();
    QString generateButtonStyleSheet() const;
    QString generateMenuStyleSheet() const;
    QString generateToolBarStyleSheet() const;
    QString generateStatusBarStyleSheet() const;
    QString generateDialogStyleSheet() const;
    
    // 主题基本信息
    QString m_name;
    QString m_displayName;

    // 颜色定义
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_backgroundColor;
    QColor m_textColor;
    QColor m_accentColor;
    QColor m_borderColor;
    QColor m_hoverColor;
    QColor m_pressedColor;
    QColor m_disabledColor;
    QColor m_errorColor;
    QColor m_warningColor;
    QColor m_successColor;

    // 字体定义
    QFont m_defaultFont;
    QFont m_titleFont;
    QFont m_headerFont;
    QFont m_buttonFont;
    QFont m_menuFont;
    QFont m_tooltipFont;

    // 尺寸定义
    int m_borderRadius;
    int m_borderWidth;
    int m_spacing;
    int m_margin;
    int m_padding;
    int m_iconSize;
    int m_buttonHeight;
    int m_toolbarHeight;

    // 样式表缓存
    mutable QString m_cachedStyleSheet;
    mutable QMap<QString, QString> m_cachedWidgetStyleSheets;

    // 资源路径
    QString m_resourcePath;
    QString m_iconPath;
    QString m_imagePath;

    bool m_loaded;
};

#endif // DEFAULTTHEME_H