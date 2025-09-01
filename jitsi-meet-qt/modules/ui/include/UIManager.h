#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "interfaces/IUIManager.h"
#include <QObject>
#include <QString>
#include <QWidget>
#include <QVariantMap>
#include <memory>

class IThemeManager;
class ILayoutManager;
class ThemeManager;
class UIConfig;
class BaseTheme;

/**
 * @brief UI管理器实现类
 * 
 * UIManager是IUIManager接口的具体实现，负责管理应用程序的
 * 用户界面、主题切换、布局管理和UI组件协调。
 */
class UIManager : public QObject, public IUIManager
{
    Q_OBJECT

public:
    explicit UIManager(QObject *parent = nullptr);
    ~UIManager() override;

    // IUIManager接口实现
    bool initialize() override;
    void shutdown() override;
    ManagerStatus status() const override;

    // 主题管理
    bool setTheme(const QString& themeName) override;
    QString currentTheme() const override;
    QStringList availableThemes() const override;
    bool applyTheme(std::shared_ptr<BaseTheme> theme) override;

    // 布局管理
    bool setLayout(const QString& layoutName) override;
    QString currentLayout() const override;
    QStringList availableLayouts() const override;
    bool updateLayout() override;

    // 窗口管理
    bool setMainWindow(QWidget* window) override;
    QWidget* mainWindow() const override;
    bool showWindow(const QString& windowName) override;
    bool hideWindow(const QString& windowName) override;

    // 配置管理
    bool applyConfiguration(const UIConfig& config) override;
    UIConfig currentConfiguration() const override;
    bool saveConfiguration() override;
    bool loadConfiguration() override;

    // 样式管理
    bool applyStyleSheet(const QString& styleSheet) override;
    QString currentStyleSheet() const override;
    bool loadStyleFromFile(const QString& filePath) override;

    // 组件管理
    bool registerWidget(const QString& name, QWidget* widget) override;
    QWidget* getWidget(const QString& name) const override;
    bool unregisterWidget(const QString& name) override;
    QStringList registeredWidgets() const override;

    // 单例访问
    static UIManager* instance();

signals:
    void themeChanged(const QString& themeName) override;
    void layoutChanged(const QString& layoutName) override;
    void windowShown(const QString& windowName) override;
    void windowHidden(const QString& windowName) override;
    void configurationChanged() override;
    void styleSheetChanged() override;
    void widgetRegistered(const QString& name) override;
    void widgetUnregistered(const QString& name) override;
    void errorOccurred(const QString& error) override;

private slots:
    void onThemeManagerError(const QString& error);
    void onLayoutManagerError(const QString& error);
    void onThemeChanged(const QString& oldTheme, const QString& newTheme);

private:
    void setupManagers();
    void setupConnections();
    void applyDefaultConfiguration();
    bool validateThemeName(const QString& themeName) const;
    bool validateLayoutName(const QString& layoutName) const;

    ManagerStatus m_status;
    QString m_currentTheme;
    QString m_currentLayout;
    QString m_currentStyleSheet;

    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<ILayoutManager> m_layoutManager;
    std::unique_ptr<UIConfig> m_config;

    QWidget* m_mainWindow;
    QMap<QString, QWidget*> m_registeredWidgets;

    static UIManager* s_instance;
};

#endif // UIMANAGER_H