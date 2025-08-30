#ifndef SCREENSHAREMODULE_H
#define SCREENSHAREMODULE_H

#include <QObject>
#include <QVariantMap>
#include "../interfaces/IScreenShareManager.h"

/**
 * @brief 屏幕共享模块核心类
 * 
 * 屏幕共享模块的主要入口点，负责模块的初始化、配置和生命周期管理
 */
class ScreenShareModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    /**
     * @brief 模块状态枚举
     */
    enum ModuleStatus {
        NotLoaded,      ///< 未加载
        Loading,        ///< 加载中
        Loaded,         ///< 已加载
        Initializing,   ///< 初始化中
        Ready,          ///< 就绪状态
        Error           ///< 错误状态
    };
    Q_ENUM(ModuleStatus)

    explicit ScreenShareModule(QObject *parent = nullptr);
    virtual ~ScreenShareModule();

    // 单例访问
    static ScreenShareModule* instance();

    // 模块基础接口
    bool initialize(const QVariantMap& config = QVariantMap());
    void shutdown();
    bool isInitialized() const;
    ModuleStatus status() const;
    QString version() const;

    // 模块控制接口
    bool isEnabled() const;
    void setEnabled(bool enabled);
    void reload();
    void reset();

    // 配置管理接口
    void setConfiguration(const QVariantMap& config);
    QVariantMap configuration() const;
    bool loadConfiguration(const QString& filePath = QString());
    bool saveConfiguration(const QString& filePath = QString()) const;

    // 管理器访问接口
    IScreenShareManager* screenShareManager() const;

    // 模块信息接口
    QString moduleName() const;
    QString moduleDescription() const;
    QStringList dependencies() const;
    QVariantMap moduleInfo() const;

    // 诊断接口
    bool selfTest();
    QStringList getLastErrors() const;
    void clearErrors();

public slots:
    /**
     * @brief 启动模块
     */
    void start();

    /**
     * @brief 停止模块
     */
    void stop();

    /**
     * @brief 重启模块
     */
    void restart();

signals:
    /**
     * @brief 模块初始化状态改变信号
     * @param initialized 是否已初始化
     */
    void initializedChanged(bool initialized);

    /**
     * @brief 模块启用状态改变信号
     * @param enabled 是否已启用
     */
    void enabledChanged(bool enabled);

    /**
     * @brief 模块状态改变信号
     * @param status 新的模块状态
     */
    void statusChanged(ModuleStatus status);

    /**
     * @brief 模块错误信号
     * @param error 错误信息
     */
    void moduleError(const QString& error);

    /**
     * @brief 配置改变信号
     * @param config 新的配置
     */
    void configurationChanged(const QVariantMap& config);

    /**
     * @brief 模块就绪信号
     */
    void moduleReady();

    /**
     * @brief 模块关闭信号
     */
    void moduleShutdown();

private slots:
    void onManagerStatusChanged(IScreenShareManager::ManagerStatus status);
    void onManagerError(const QString& error);

private:
    void initializeComponents();
    void cleanupComponents();
    bool validateConfiguration(const QVariantMap& config) const;
    void updateStatus(ModuleStatus newStatus);

    class Private;
    Private* d;
};

#endif // SCREENSHAREMODULE_H