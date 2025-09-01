#ifndef IUIMANAGER_H
#define IUIMANAGER_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QStringList>
#include <QVariantMap>
#include <memory>

class BaseTheme;

/**
 * @brief UI管理器接口
 * 
 * 定义了UI管理器的标准接口，负责管理应用程序的
 * 用户界面、主题切换、布局管理和UI组件协调。
 */
class IUIManager
{
public:
    /**
     * @brief 管理器状态枚举
     */
    enum ManagerStatus {
        Uninitialized,    ///< 未初始化
        Initializing,     ///< 正在初始化
        Active,           ///< 活动状态
        Error,            ///< 错误状态
        ShuttingDown,     ///< 正在关闭
        Inactive          ///< 非活动状态
    };

    /**
     * @brief 析构函数
     */
    virtual ~IUIManager() {}

    /**
     * @brief 初始化UI管理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 关闭UI管理器
     */
    virtual void shutdown() = 0;

    /**
     * @brief 获取当前状态
     * @return 管理器状态
     */
    virtual ManagerStatus status() const = 0;

    // 主题管理
    /**
     * @brief 设置主题
     * @param themeName 主题名称
     * @return 设置是否成功
     */
    virtual bool setTheme(const QString& themeName) = 0;

    /**
     * @brief 获取当前主题
     * @return 当前主题名称
     */
    virtual QString currentTheme() const = 0;

    /**
     * @brief 获取可用主题列表
     * @return 主题名称列表
     */
    virtual QStringList availableThemes() const = 0;

    /**
     * @brief 应用主题
     * @param theme 主题对象
     * @return 应用是否成功
     */
    virtual bool applyTheme(std::shared_ptr<BaseTheme> theme) = 0;

    // 布局管理
    /**
     * @brief 设置布局
     * @param layoutName 布局名称
     * @return 设置是否成功
     */
    virtual bool setLayout(const QString& layoutName) = 0;

    /**
     * @brief 获取当前布局
     * @return 当前布局名称
     */
    virtual QString currentLayout() const = 0;

    /**
     * @brief 获取可用布局列表
     * @return 布局名称列表
     */
    virtual QStringList availableLayouts() const = 0;

    /**
     * @brief 更新布局
     * @return 更新是否成功
     */
    virtual bool updateLayout() = 0;

    // 窗口管理
    /**
     * @brief 设置主窗口
     * @param window 窗口指针
     * @return 设置是否成功
     */
    virtual bool setMainWindow(QWidget* window) = 0;

    /**
     * @brief 获取主窗口
     * @return 窗口指针
     */
    virtual QWidget* mainWindow() const = 0;

    /**
     * @brief 显示窗口
     * @param windowName 窗口名称
     * @return 显示是否成功
     */
    virtual bool showWindow(const QString& windowName) = 0;

    /**
     * @brief 隐藏窗口
     * @param windowName 窗口名称
     * @return 隐藏是否成功
     */
    virtual bool hideWindow(const QString& windowName) = 0;

    // 配置管理
    /**
     * @brief 应用配置
     * @param config 配置对象
     * @return 应用是否成功
     */
    virtual bool applyConfiguration(const class UIConfig& config) = 0;

    /**
     * @brief 获取当前配置
     * @return 配置对象
     */
    virtual class UIConfig currentConfiguration() const = 0;

    /**
     * @brief 保存配置
     * @return 保存是否成功
     */
    virtual bool saveConfiguration() = 0;

    /**
     * @brief 加载配置
     * @return 加载是否成功
     */
    virtual bool loadConfiguration() = 0;

    // 样式管理
    /**
     * @brief 应用样式表
     * @param styleSheet 样式表字符串
     * @return 应用是否成功
     */
    virtual bool applyStyleSheet(const QString& styleSheet) = 0;

    /**
     * @brief 获取当前样式表
     * @return 样式表字符串
     */
    virtual QString currentStyleSheet() const = 0;

    /**
     * @brief 从文件加载样式
     * @param filePath 文件路径
     * @return 加载是否成功
     */
    virtual bool loadStyleFromFile(const QString& filePath) = 0;

    // 组件管理
    /**
     * @brief 注册组件
     * @param name 组件名称
     * @param widget 组件指针
     * @return 注册是否成功
     */
    virtual bool registerWidget(const QString& name, QWidget* widget) = 0;

    /**
     * @brief 获取组件
     * @param name 组件名称
     * @return 组件指针
     */
    virtual QWidget* getWidget(const QString& name) const = 0;

    /**
     * @brief 取消注册组件
     * @param name 组件名称
     * @return 取消注册是否成功
     */
    virtual bool unregisterWidget(const QString& name) = 0;

    /**
     * @brief 获取已注册组件列表
     * @return 组件名称列表
     */
    virtual QStringList registeredWidgets() const = 0;

signals:
    /**
     * @brief 主题改变信号
     * @param themeName 新主题名称
     */
    virtual void themeChanged(const QString& themeName) = 0;

    /**
     * @brief 布局改变信号
     * @param layoutName 新布局名称
     */
    virtual void layoutChanged(const QString& layoutName) = 0;

    /**
     * @brief 窗口显示信号
     * @param windowName 窗口名称
     */
    virtual void windowShown(const QString& windowName) = 0;

    /**
     * @brief 窗口隐藏信号
     * @param windowName 窗口名称
     */
    virtual void windowHidden(const QString& windowName) = 0;

    /**
     * @brief 配置改变信号
     */
    virtual void configurationChanged() = 0;

    /**
     * @brief 样式表改变信号
     */
    virtual void styleSheetChanged() = 0;

    /**
     * @brief 组件注册信号
     * @param name 组件名称
     */
    virtual void widgetRegistered(const QString& name) = 0;

    /**
     * @brief 组件取消注册信号
     * @param name 组件名称
     */
    virtual void widgetUnregistered(const QString& name) = 0;

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    virtual void errorOccurred(const QString& error) = 0;
};

#endif // IUIMANAGER_H