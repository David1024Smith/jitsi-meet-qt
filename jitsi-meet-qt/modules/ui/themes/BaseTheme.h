#ifndef BASETHEME_H
#define BASETHEME_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QSize>
#include <QVariantMap>

/**
 * @brief 主题基类
 * 
 * BaseTheme定义了所有主题必须实现的基础接口，
 * 包括颜色方案、字体设置、样式表和资源管理。
 */
class BaseTheme : public QObject
{
    Q_OBJECT

public:
    explicit BaseTheme(QObject *parent = nullptr);
    virtual ~BaseTheme();

    // 主题信息
    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual QString description() const = 0;
    virtual QString version() const;
    virtual QString author() const;

    // 主题生命周期
    virtual bool load() = 0;
    virtual bool apply() = 0;
    virtual void unload() = 0;
    virtual bool isLoaded() const = 0;

    // 颜色方案
    virtual QColor primaryColor() const = 0;
    virtual QColor secondaryColor() const = 0;
    virtual QColor backgroundColor() const = 0;
    virtual QColor textColor() const = 0;
    virtual QColor accentColor() const = 0;
    virtual QColor borderColor() const = 0;
    virtual QColor hoverColor() const = 0;
    virtual QColor pressedColor() const = 0;
    virtual QColor disabledColor() const = 0;
    virtual QColor errorColor() const = 0;
    virtual QColor warningColor() const = 0;
    virtual QColor successColor() const = 0;

    // 字体设置
    virtual QFont defaultFont() const = 0;
    virtual QFont titleFont() const = 0;
    virtual QFont headerFont() const = 0;
    virtual QFont buttonFont() const = 0;
    virtual QFont menuFont() const = 0;
    virtual QFont tooltipFont() const = 0;

    // 尺寸设置
    virtual int borderRadius() const = 0;
    virtual int borderWidth() const = 0;
    virtual int spacing() const = 0;
    virtual int margin() const = 0;
    virtual int padding() const = 0;
    virtual int iconSize() const = 0;
    virtual int buttonHeight() const = 0;
    virtual int toolbarHeight() const = 0;

    // 样式表
    virtual QString styleSheet() const = 0;
    virtual QString getStyleSheet() const = 0;
    virtual QString getWidgetStyleSheet(const QString& widgetType) const = 0;
    virtual QString getButtonStyleSheet() const = 0;
    virtual QString getMenuStyleSheet() const = 0;
    virtual QString getToolBarStyleSheet() const = 0;
    virtual QString getStatusBarStyleSheet() const = 0;
    virtual QString getDialogStyleSheet() const = 0;

    // 图标和资源
    virtual QString getIconPath(const QString& iconName) const = 0;
    virtual QString getImagePath(const QString& imageName) const = 0;
    virtual QPixmap getIcon(const QString& iconName, const QSize& size = QSize()) const = 0;
    virtual QPixmap getImage(const QString& imageName) const = 0;

    // 主题自定义
    virtual void setCustomProperty(const QString& property, const QVariant& value) = 0;
    virtual QVariant getCustomProperty(const QString& property) const = 0;
    virtual bool hasCustomProperty(const QString& property) const = 0;
    virtual void removeCustomProperty(const QString& property) = 0;
    
    // 配置管理
    virtual void applyConfiguration(const QVariantMap& config);
    virtual void setName(const QString& name);
    virtual void setDisplayName(const QString& displayName);

    // 主题序列化
    virtual QVariantMap toVariantMap() const;
    virtual void fromVariantMap(const QVariantMap& map);

signals:
    void themeLoaded();
    void themeApplied();
    void themeUnloaded();
    void propertyChanged(const QString& property, const QVariant& value);
    void errorOccurred(const QString& error);

protected:
    virtual void initializeColors() = 0;
    virtual void initializeFonts() = 0;
    virtual void initializeSizes() = 0;
    virtual void initializeStyleSheets() = 0;
    virtual void initializeResources() = 0;

    QVariantMap m_customProperties;
    QString m_name;
    QString m_displayName;
};

#endif // BASETHEME_H