#ifndef UIMODULE_H
#define UIMODULE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>

class UIManager;
class ThemeFactory;
class UIConfig;

/**
 * @brief UI模块核心类
 * 
 * UIModule是UI模块的核心控制类，负责模块的初始化、
 * 生命周期管理和基础功能协调。
 */
class UIModule : public QObject
{
    Q_OBJECT

public:
    enum ModuleStatus {
        NotInitialized,
        Initializing,
        Ready,
        Error
    };

    explicit UIModule(QObject *parent = nullptr);
    ~UIModule();

    // 模块基础功能
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    ModuleStatus status() const;

    // 模块信息
    QString moduleName() const;
    QString moduleVersion() const;
    QStringList dependencies() const;

    // 配置管理
    bool loadConfiguration(const QVariantMap& config);
    QVariantMap saveConfiguration() const;
    bool validateConfiguration(const QVariantMap& config) const;

    // 单例访问
    static UIModule* instance();

    // 组件访问
    UIManager* uiManager() const;
    ThemeFactory* themeFactory() const;

signals:
    void statusChanged(ModuleStatus status);
    void initialized();
    void shutdownRequested();
    void errorOccurred(const QString& error);
    void configurationChanged();

private slots:
    void onManagerError(const QString& error);
    void onThemeChanged(const QString& themeName);

private:
    void setupConnections();
    void initializeComponents();
    void cleanupComponents();

    ModuleStatus m_status;
    std::unique_ptr<UIManager> m_uiManager;
    std::unique_ptr<ThemeFactory> m_themeFactory;
    std::unique_ptr<UIConfig> m_config;

    static UIModule* s_instance;
};

#endif // UIMODULE_H