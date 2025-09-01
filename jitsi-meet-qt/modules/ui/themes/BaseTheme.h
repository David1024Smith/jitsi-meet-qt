#ifndef BASETHEME_H
#define BASETHEME_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QVariantMap>
#include <QMap>

/**
 * @brief 基础主题类
 * 
 * 定义了主题的基本接口和通用功能，所有具体主题类都应继承自此类
 */
class BaseTheme : public QObject
{
    Q_OBJECT

protected:
    QMap<QString, QVariant> m_customProperties;

public:
    /**
     * @brief 设置自定义属性
     * @param name 属性名
     * @param value 属性值
     * @return 设置是否成功
     */
    virtual bool setProperty(const QString& name, const QVariant& value) {
        m_customProperties[name] = value;
        return true;
    }
    
    /**
     * @brief 获取自定义属性
     * @param name 属性名
     * @return 属性值
     */
    virtual QVariant property(const QString& name) const {
        return m_customProperties.value(name);
    }

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BaseTheme(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 析构函数
     */
    virtual ~BaseTheme() {}

    /**
     * @brief 获取主题名称
     * @return 主题名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取主题显示名称
     * @return 显示名称
     */
    virtual QString displayName() const = 0;

    /**
     * @brief 获取主题描述
     * @return 描述
     */
    virtual QString description() const = 0;

    /**
     * @brief 获取主题版本
     * @return 版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 获取主题作者
     * @return 作者
     */
    virtual QString author() const = 0;

    /**
     * @brief 加载主题
     * @return 加载是否成功
     */
    virtual bool load() = 0;

    /**
     * @brief 应用主题
     * @return 应用是否成功
     */
    virtual bool apply() = 0;

    /**
     * @brief 卸载主题
     */
    virtual void unload() = 0;

    /**
     * @brief 检查主题是否已加载
     * @return 是否已加载
     */
    virtual bool isLoaded() const = 0;

    // 颜色方案
    /**
     * @brief 获取主色调
     * @return 颜色
     */
    virtual QColor primaryColor() const = 0;

    /**
     * @brief 获取次要色调
     * @return 颜色
     */
    virtual QColor secondaryColor() const = 0;

    /**
     * @brief 获取背景色
     * @return 颜色
     */
    virtual QColor backgroundColor() const = 0;

    /**
     * @brief 获取文本颜色
     * @return 颜色
     */
    virtual QColor textColor() const = 0;

    /**
     * @brief 获取强调色
     * @return 颜色
     */
    virtual QColor accentColor() const = 0;

    /**
     * @brief 获取边框颜色
     * @return 颜色
     */
    virtual QColor borderColor() const = 0;

    /**
     * @brief 获取悬停颜色
     * @return 颜色
     */
    virtual QColor hoverColor() const = 0;

    /**
     * @brief 获取按下颜色
     * @return 颜色
     */
    virtual QColor pressedColor() const = 0;

    /**
     * @brief 获取禁用颜色
     * @return 颜色
     */
    virtual QColor disabledColor() const = 0;

    /**
     * @brief 获取错误颜色
     * @return 颜色
     */
    virtual QColor errorColor() const = 0;

    /**
     * @brief 获取警告颜色
     * @return 颜色
     */
    virtual QColor warningColor() const = 0;

    /**
     * @brief 获取成功颜色
     * @return 颜色
     */
    virtual QColor successColor() const = 0;

    // 字体设置
    /**
     * @brief 获取默认字体
     * @return 字体
     */
    virtual QFont defaultFont() const = 0;

    /**
     * @brief 获取标题字体
     * @return 字体
     */
    virtual QFont titleFont() const = 0;

    /**
     * @brief 获取标头字体
     * @return 字体
     */
    virtual QFont headerFont() const = 0;

    /**
     * @brief 获取按钮字体
     * @return 字体
     */
    virtual QFont buttonFont() const = 0;

    /**
     * @brief 获取菜单字体
     * @return 字体
     */
    virtual QFont menuFont() const = 0;

    /**
     * @brief 获取工具提示字体
     * @return 字体
     */
    virtual QFont tooltipFont() const = 0;

    // 尺寸设置
    /**
     * @brief 获取边框圆角
     * @return 圆角大小
     */
    virtual int borderRadius() const = 0;

    /**
     * @brief 获取边框宽度
     * @return 宽度
     */
    virtual int borderWidth() const = 0;

    /**
     * @brief 获取间距
     * @return 间距
     */
    virtual int spacing() const = 0;

    /**
     * @brief 获取外边距
     * @return 外边距
     */
    virtual int margin() const = 0;

    /**
     * @brief 获取内边距
     * @return 内边距
     */
    virtual int padding() const = 0;

    /**
     * @brief 获取图标大小
     * @return 图标大小
     */
    virtual int iconSize() const = 0;

    /**
     * @brief 获取按钮高度
     * @return 按钮高度
     */
    virtual int buttonHeight() const = 0;

    /**
     * @brief 获取工具栏高度
     * @return 工具栏高度
     */
    virtual int toolbarHeight() const = 0;

    // 样式表
    /**
     * @brief 获取样式表
     * @return 样式表字符串
     */
    virtual QString styleSheet() const = 0;

    /**
     * @brief 获取组件样式表
     * @param widgetType 组件类型
     * @return 样式表字符串
     */
    virtual QString getWidgetStyleSheet(const QString& widgetType) const = 0;

    // 图标和资源
    /**
     * @brief 获取图标路径
     * @param iconName 图标名称
     * @return 图标路径
     */
    virtual QString getIconPath(const QString& iconName) const = 0;

    /**
     * @brief 获取图片路径
     * @param imageName 图片名称
     * @return 图片路径
     */
    virtual QString getImagePath(const QString& imageName) const = 0;

    /**
     * @brief 获取图标
     * @param iconName 图标名称
     * @param size 大小
     * @return 图标
     */
    virtual QPixmap getIcon(const QString& iconName, const QSize& size = QSize()) const = 0;

    /**
     * @brief 获取图片
     * @param imageName 图片名称
     * @return 图片
     */
    virtual QPixmap getImage(const QString& imageName) const = 0;

    // 主题自定义
    /**
     * @brief 设置自定义属性
     * @param property 属性名称
     * @param value 属性值
     */
    virtual void setCustomProperty(const QString& property, const QVariant& value) = 0;

    /**
     * @brief 获取自定义属性
     * @param property 属性名称
     * @return 属性值
     */
    virtual QVariant getCustomProperty(const QString& property) const = 0;

    /**
     * @brief 检查是否有自定义属性
     * @param property 属性名称
     * @return 是否有属性
     */
    virtual bool hasCustomProperty(const QString& property) const = 0;

    /**
     * @brief 移除自定义属性
     * @param property 属性名称
     */
    virtual void removeCustomProperty(const QString& property) = 0;

    /**
     * @brief 应用配置
     * @param config 配置映射
     * @return 应用是否成功
     */
    virtual bool applyConfiguration(const QVariantMap& config) { Q_UNUSED(config); return false; }
    
    /**
     * @brief 转换为变量映射
     * @return 变量映射
     */
    virtual QVariantMap toVariantMap() const;
    
    /**
     * @brief 从变量映射加载
     * @param map 变量映射
     */
    virtual void fromVariantMap(const QVariantMap& map);
    
    /**
     * @brief 设置主题名称
     * @param name 主题名称
     */
    virtual void setName(const QString& name);
    
    /**
     * @brief 设置主题显示名称
     * @param displayName 显示名称
     */
    virtual void setDisplayName(const QString& displayName);
    
    /**
     * @brief 获取样式表
     * @return 样式表字符串
     */
    virtual QString getStyleSheet() const { return styleSheet(); }
    
    /**
     * @brief 获取按钮样式表
     * @return 样式表字符串
     */
    virtual QString getButtonStyleSheet() const { return getWidgetStyleSheet("QPushButton"); }
    
    /**
     * @brief 获取菜单样式表
     * @return 样式表字符串
     */
    virtual QString getMenuStyleSheet() const { return getWidgetStyleSheet("QMenu"); }
    
    /**
     * @brief 获取工具栏样式表
     * @return 样式表字符串
     */
    virtual QString getToolBarStyleSheet() const { return getWidgetStyleSheet("QToolBar"); }
    
    /**
     * @brief 获取状态栏样式表
     * @return 样式表字符串
     */
    virtual QString getStatusBarStyleSheet() const { return getWidgetStyleSheet("QStatusBar"); }
    
    /**
     * @brief 获取对话框样式表
     * @return 样式表字符串
     */
    virtual QString getDialogStyleSheet() const { return getWidgetStyleSheet("QDialog"); }
    
    /**
     * @brief 初始化颜色
     */
    virtual void initializeColors() {}
    
    /**
     * @brief 初始化字体
     */
    virtual void initializeFonts() {}
    
    /**
     * @brief 初始化尺寸
     */
    virtual void initializeSizes() {}
    
    /**
     * @brief 初始化样式表
     */
    virtual void initializeStyleSheets() {}
    
    /**
     * @brief 初始化资源
     */
    virtual void initializeResources() {}

signals:
    /**
     * @brief 主题加载信号
     */
    void themeLoaded();

    /**
     * @brief 主题卸载信号
     */
    void themeUnloaded();

    /**
     * @brief 主题应用信号
     */
    void themeApplied();

    /**
     * @brief 属性改变信号
     * @param property 属性名称
     * @param value 属性值
     */
    void propertyChanged(const QString& property, const QVariant& value);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

#endif // BASETHEME_H