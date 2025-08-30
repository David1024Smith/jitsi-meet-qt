#ifndef IUIMANAGER_H
#define IUIMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <memory>

class UIConfig;
class BaseTheme;

/**
 * @brief UI管理器接口
 * 
 * IUIManager定义了UI管理器的标准接口，包括主题管理、
 * 布局管理、窗口管理和配置管理等核心功能。
 */
class IUIManager
{
public:
    enum ManagerStatus {
        Uninitialized,
        Initializing,
        Ready,
        Busy,
        Error
    };

    virtual ~IUIManager() = default;

    // 生命周期管理
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual ManagerStatus status() const = 0;

    // 主题管理
    virtual bool setTheme(const QString& themeName) = 0;
    virtual QString currentTheme() const = 0;
    virtual QStringList availableThemes() const = 0;
    virtual bool applyTheme(std::shared_ptr<BaseTheme> theme) = 0;

    // 布局管理
    virtual bool setLayout(const QString& layoutName) = 0;
    virtual QString currentLayout() const = 0;
    virtual QStringList availableLayouts() const = 0;
    virtual bool updateLayout() = 0;

    // 窗口管理
    virtual bool setMainWindow(QWidget* window) = 0;
    virtual QWidget* mainWindow() const = 0;
    virtual bool showWindow(const QString& windowName) = 0;
    virtual bool hideWindow(const QString& windowName) = 0;

    // 配置管理
    virtual bool applyConfiguration(const UIConfig& config) = 0;
    virtual UIConfig currentConfiguration() const = 0;
    virtual bool saveConfiguration() = 0;
    virtual bool loadConfiguration() = 0;

    // 样式管理
    virtual bool applyStyleSheet(const QString& styleSheet) = 0;
    virtual QString currentStyleSheet() const = 0;
    virtual bool loadStyleFromFile(const QString& filePath) = 0;

    // 组件管理
    virtual bool registerWidget(const QString& name, QWidget* widget) = 0;
    virtual QWidget* getWidget(const QString& name) const = 0;
    virtual bool unregisterWidget(const QString& name) = 0;
    virtual QStringList registeredWidgets() const = 0;

    // 信号接口 (需要在实现类中定义为signals)
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

Q_DECLARE_INTERFACE(IUIManager, "org.jitsi.UIManager/1.0")

#endif // IUIMANAGER_H