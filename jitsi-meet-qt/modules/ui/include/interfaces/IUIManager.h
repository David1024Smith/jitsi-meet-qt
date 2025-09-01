#ifndef IUIMANAGER_H
#define IUIMANAGER_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QVariantMap>
#include <memory>

class BaseTheme;
class UIConfig;

/**
 * @brief UI管理器接口
 * 
 * IUIManager定义了UI管理器的接口，负责管理应用程序的
 * 用户界面、主题切换、布局管理和UI组件协调。
 */
class IUIManager
{
public:
    /**
     * @brief 管理器状态枚举
     */
    enum ManagerStatus {
        NotInitialized,  ///< 未初始化
        Initializing,    ///< 初始化中
        Running,         ///< 运行中
        ShuttingDown,    ///< 关闭中
        Error            ///< 错误状态
    };

    virtual ~IUIManager() = default;

    // 基本管理
    /**
     * @brief 初始化UI管理器
     * @return 是否成功初始化
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
     * @return 是否成功设置
     */
    virtual bool setTheme(const QString& themeName) = 0;

    /**
     * @brief 获取当前主题
     * @return 主题名称
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
     * @return 是否成功应用
     */
    virtual bool applyTheme(std::shared_ptr<BaseTheme> theme) = 0;

    // 布局管理
    /**
     * @brief 设置布局
     * @param layoutName 布局名称
     * @return 是否成功设置
     */
    virtual bool setLayout(const QString& layoutName) = 0;

    /**
     * @brief 获取当前布局
     * @return 布局名称
     */
    virtual QString currentLayout() const = 0;

    /**
     * @brief 获取可用布局列表
     * @return 布局名称列表
     */
    virtual QStringList availableLayouts() const = 0;

    /**
     * @brief 更新布局
     * @return 是否成功更新
     */
    virtual bool updateLayout() = 0;

    // 窗口管理
    /**
     * @brief 设置主窗口
     * @param window 窗口对象
     * @return 是否成功设置
     */
    virtual bool setMainWindow(QWidget* window) = 0;

    /**
     * @brief 获取主窗口
     * @return 窗口对象
     */
    virtual QWidget* mainWindow() const = 0;

    /**
     * @brief 显示窗口
     * @param windowName 窗口名称
     * @return 是否成功显示
     */
    virtual bool showWindow(const QString& windowName) = 0;

    /**
     * @brief 隐藏窗口
     * @param windowName 窗口名称
     * @return 是否成功隐藏
     */
    virtual bool hideWindow(const QString& windowName) = 0;

    // 配置管理
    /**
     * @brief 应用配置
     * @param config 配置对象
     * @return 是否成功应用
     */
    virtual bool applyConfiguration(const UIConfig& config) = 0;

    /**
     * @brief 获取当前配置
     * @return 配置对象
     */
    virtual UIConfig currentConfiguration() const = 0;

    /**
     * @brief 保存配置
     * @return 是否成功保存
     */
    virtual bool saveConfiguration() = 0;

    /**
     * @brief 加载配置
     * @return 是否成功加载
     */
    virtual bool loadConfiguration() = 0;

    // 样式管理
    /**
     * @brief 应用样式表
     * @param styleSheet 样式表字符串
     * @return 是否成功应用
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
     * @return 是否成功加载
     */
    virtual bool loadStyleFromFile(const QString& filePath) = 0;

    // 组件管理
    /**
     * @brief 注册组件
     * @param name 组件名称
     * @param widget 组件对象
     * @return 是否成功注册
     */
    virtual bool registerWidget(const QString& name, QWidget* widget) = 0;

    /**
     * @brief 获取组件
     * @param name 组件名称
     * @return 组件对象
     */
    virtual QWidget* getWidget(const QString& name) const = 0;

    /**
     * @brief 注销组件
     * @param name 组件名称
     * @return 是否成功注销
     */
    virtual bool unregisterWidget(const QString& name) = 0;

    /**
     * @brief 获取已注册组件列表
     * @return 组件名称列表
     */
    virtual QStringList registeredWidgets() const = 0;

    // 信号定义
    virtual void themeChanged(const QString& themeName) = 0;
    virtual void layoutChanged(const QString& layoutName) = 0;
    virtual void windowShown(const QString& windowName) = 0;
    virtual void windowHidden(const QString& windowName) = 0;
    virtual void configurationChanged() = 0;
    virtual void styleSheetChanged() = 0;
    virtual void widgetRegistered(const QString& name) = 0;
    virtual void widgetUnregistered(const QString& name) = 0;
    virtual void errorOccurred(const QString& error) = 0;
};

#endif // IUIMANAGER_H
